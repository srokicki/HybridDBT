
#ifdef __LINUX_API

#include <cstdio>
#include <lib/elfFile.h>
#include <stdlib.h>
#include <string>
#include <vector>
//#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG 0

#define SWAP_2(x) ((((x)&0xff) << 8) | ((unsigned short)(x) >> 8))
#define SWAP_4(x) ((x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24))
#define FIX_SHORT(x) (x) = needToFixEndianness ? SWAP_2(x) : x
#define FIX_INT(x) (x) = needToFixEndianness ? SWAP_4(x) : x

using namespace std;

char needToFixEndianness = 0;

/*************************************************************************************************************
 ******************************************  Code for class ElfFile  *****************************************
 *************************************************************************************************************/

ElfFile::ElfFile(char* pathToElfFile)
{

  this->pathToElfFile = pathToElfFile;
  this->elfFile       = fopen(pathToElfFile, "r+");
  if (this->elfFile == NULL) {
    printf("Failing to open %s\n exiting...\n", pathToElfFile);
    exit(-1);
  }

  //*************************************************************************************
  // First step is to read the 16 first bits to determine the type of the elf file to read.
  unsigned int result;
  char eident[16];
  result = fread(eident, sizeof(char), 16, elfFile);
  result = fseek(elfFile, 0, SEEK_SET);

  if (eident[EI_CLASS] == ELFCLASS32)
    this->is32Bits = 1;
  else if (eident[EI_CLASS] == ELFCLASS64)
    this->is32Bits = 0;
  else {
    fprintf(stderr, "Error while reading ELF file header, cannot handle this type of ELF file...\n");
    exit(-1);
  }

  //*************************************************************************************
  // Reading the header of the elf file
  // With different code if it is 32 or 64 bits

  if (this->is32Bits) {
    result = fread(&this->fileHeader32, sizeof(this->fileHeader32), 1, elfFile);

    if (this->fileHeader32.e_ident[0] == 0x7f)
      needToFixEndianness = 0;
    else
      needToFixEndianness = 1;
  } else {
    result = fread(&this->fileHeader64, sizeof(this->fileHeader64), 1, elfFile);

    if (this->fileHeader64.e_ident[0] == 0x7f)
      needToFixEndianness = 0;
    else
      needToFixEndianness = 1;
  }

  if (DEBUG && this->is32Bits)
    printf("Program table is at %hu and contains %u entries of %u bytes\n", FIX_INT(this->fileHeader32.e_phoff),
           FIX_SHORT(this->fileHeader32.e_phnum), FIX_SHORT(this->fileHeader32.e_phentsize));

  //*************************************************************************************
  // Parsing section table

  unsigned long start;
  unsigned long tableSize;
  unsigned long entrySize;

  if (this->is32Bits) {
    start     = FIX_INT(fileHeader32.e_shoff);
    tableSize = FIX_SHORT(fileHeader32.e_shnum);
    entrySize = FIX_SHORT(fileHeader32.e_shentsize);
  } else {
    start     = FIX_INT(fileHeader64.e_shoff);
    tableSize = FIX_SHORT(fileHeader64.e_shnum);
    entrySize = FIX_SHORT(fileHeader64.e_shentsize);
  }

  if (DEBUG)
    printf("Section table is at %hu and contains %u entries of %u bytes\n", start, tableSize, entrySize);

  unsigned int res = fseek(elfFile, start, SEEK_SET);
  if (res != 0)
    printf("Error while moving to the beginning of section table\n");

  //*************************************************************************************
  // We create a simple array and read the section table

  if (this->is32Bits) {
    Elf32_Shdr* localSectionTable = (Elf32_Shdr*)malloc(tableSize * entrySize);

    res = fread(localSectionTable, entrySize, tableSize, elfFile);
    if (res != tableSize)
      printf("Error while reading the section table ! (section size is %u while we only read %u entries)\n", tableSize,
             res);

    // We then copy the section table into it
    this->sectionTable = new std::vector<ElfSection*>();
    for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++)
      this->sectionTable->push_back(new ElfSection(this, sectionNumber, localSectionTable[sectionNumber]));

  } else {
    Elf64_Shdr* localSectionTable = (Elf64_Shdr*)malloc(tableSize * entrySize);

    res = fread(localSectionTable, entrySize, tableSize, elfFile);
    if (res != tableSize)
      printf("Error while reading the section table ! (section size is %u while we only read %u entries)\n", tableSize,
             res);

    // We then copy the section table into it
    this->sectionTable = new std::vector<ElfSection*>();
    for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++)
      this->sectionTable->push_back(new ElfSection(this, sectionNumber, localSectionTable[sectionNumber]));
  }

  if (DEBUG)
    for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++) {
      ElfSection* sect = this->sectionTable->at(sectionNumber);
      printf("Section is at %x, of size %x\n", sect->offset, sect->size);
    }

  //*************************************************************************************
  // Location of the String table containing every name

  unsigned long nameTableIndex;
  if (this->is32Bits)
    nameTableIndex = FIX_SHORT(fileHeader32.e_shstrndx);
  else
    nameTableIndex = FIX_SHORT(fileHeader64.e_shstrndx);

  ElfSection* nameTableSection = this->sectionTable->at(nameTableIndex);

  unsigned char* localNameTable = nameTableSection->getSectionCode();
  this->nameTable               = new std::vector<string>(this->sectionTable->size());

  for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++) {
    ElfSection* section = this->sectionTable->at(sectionNumber);

    unsigned int nameIndex = section->nameIndex;
    std::string name("");
    while (localNameTable[nameIndex] != '\0') {
      name += localNameTable[nameIndex];
      nameIndex++;
    }
    section->nameIndex = this->nameTable->size();
    this->nameTable->push_back(name);
  }

  //*************************************************************************************
  // Reading the symbol table

  this->symbols = new vector<ElfSymbol*>();
  for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++) {
    ElfSection* section = this->sectionTable->at(sectionNumber);
    if (section->type == SHT_SYMTAB) {

      if (this->is32Bits) {
        Elf32_Sym* symbols = (Elf32_Sym*)section->getSectionCode();
        for (unsigned int oneSymbolIndex = 0; oneSymbolIndex < section->size / sizeof(Elf32_Sym); oneSymbolIndex++)
          this->symbols->push_back(new ElfSymbol(symbols[oneSymbolIndex]));
      } else {
        Elf64_Sym* symbols = (Elf64_Sym*)section->getSectionCode();
        for (unsigned int oneSymbolIndex = 0; oneSymbolIndex < section->size / sizeof(Elf64_Sym); oneSymbolIndex++)
          this->symbols->push_back(new ElfSymbol(symbols[oneSymbolIndex]));
      }
    }
  }

  for (unsigned int sectionNumber = 0; sectionNumber < tableSize; sectionNumber++) {
    ElfSection* section = this->sectionTable->at(sectionNumber);
    if (section->getName().compare(std::string(".strtab")) == 0) {
      this->indexOfSymbolNameSection = sectionNumber;
      break;
    }
  }
}

ElfFile* ElfFile::copy(char* newDest)
{

  //    int input, output;
  //    if ((input = open(this->pathToElfFile, O_RDONLY)) == -1){
  //    	printf("Error while opening %s", this->pathToElfFile);
  //        exit(-1);
  //    }
  //    if ((output = open(newDest, O_WRONLY|O_TRUNC|O_CREAT,S_IWRITE | S_IREAD)) == -1){
  //        close(input);
  //    	printf("Error while opening %s", newDest);
  //        exit(-1);
  //    }

  fprintf(stderr, "Send file function in elf file is no longer supported !\n Exiting...\n");
  exit(-1);

  //    //sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+
  //    off_t bytesCopied = 0;
  //    struct stat fileinfo = {0};
  //    fstat(input, &fileinfo);
  ////    sendfile(output, input, &bytesCopied, fileinfo.st_size);
  //
  //    close(input);
  //    close(output);
  //
  //    return new ElfFile(newDest);
}

/*************************************************************************************************************
 *****************************************  Code for class ElfSection  ***************************************
 *************************************************************************************************************/

ElfSection::ElfSection(ElfFile* elfFile, int id, Elf32_Shdr header)
{
  this->containingElfFile = elfFile;
  this->id                = id;
  this->offset            = FIX_INT(header.sh_offset);
  this->size              = FIX_INT(header.sh_size);
  this->nameIndex         = FIX_INT(header.sh_name);
  this->address           = FIX_INT(header.sh_addr);
  this->type              = FIX_INT(header.sh_type);
  this->info              = FIX_INT(header.sh_info);
}

ElfSection::ElfSection(ElfFile* elfFile, int id, Elf64_Shdr header)
{
  this->containingElfFile = elfFile;
  this->id                = id;
  this->offset            = FIX_INT(header.sh_offset);
  this->size              = FIX_INT(header.sh_size);
  this->nameIndex         = FIX_INT(header.sh_name);
  this->address           = FIX_INT(header.sh_addr);
  this->type              = FIX_INT(header.sh_type);
  this->info              = FIX_INT(header.sh_info);
}

string ElfSection::getName()
{
  return this->containingElfFile->nameTable->at(this->nameIndex);
}

bool ElfSection::isRelSection()
{
  return type == SHT_REL;
}

bool ElfSection::isRelaSection()
{
  return type == SHT_RELA;
}

unsigned char* ElfSection::getSectionCode()
{
  unsigned char* sectionContent = (unsigned char*)malloc(this->size);
  unsigned int result;
  result = fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
  result = fread(sectionContent, 1, this->size, this->containingElfFile->elfFile);

  return sectionContent;
}

std::vector<ElfRelocation*>* ElfSection::getRelocations()
{
  vector<ElfRelocation*>* result = new vector<ElfRelocation*>();

  unsigned int readResult;

  // On non REL or RELA section, we return an empty vector
  if (!(this->isRelSection() || this->isRelaSection())) {
    return result;
  }

  if (this->isRelSection()) {
    Elf32_Rel* sectionContent = (Elf32_Rel*)malloc(this->size);
    readResult                = fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
    readResult =
        fread(sectionContent, sizeof(Elf32_Rel), this->size / sizeof(Elf32_Rel), this->containingElfFile->elfFile);
    for (unsigned int relCounter = 0; relCounter < this->size / sizeof(Elf32_Rel); relCounter++)
      result->push_back(new ElfRelocation(sectionContent[relCounter]));
  } else {
    Elf32_Rela* sectionContent = (Elf32_Rela*)malloc(this->size);
    readResult                 = fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
    readResult =
        fread(sectionContent, sizeof(Elf32_Rela), this->size / sizeof(Elf32_Rela), this->containingElfFile->elfFile);
    for (unsigned int relCounter = 0; relCounter < this->size / sizeof(Elf32_Rela); relCounter++)
      result->push_back(new ElfRelocation(sectionContent[relCounter]));
  }

  return result;
}

void ElfSection::writeSectionCode(unsigned char* newContent)
{
  unsigned int readResult;
  readResult = fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
  readResult = fwrite(newContent, 1, this->size, this->containingElfFile->elfFile);
}

void ElfSection::writeSectionCode(FILE* file, unsigned char* newContent)
{
  fseek(this->containingElfFile->elfFile, this->offset, SEEK_SET);
  fwrite(newContent, 1, this->size, this->containingElfFile->elfFile);
}

/*************************************************************************************************************
 ****************************************  Code for class ElfSymbol  *************************************
 *************************************************************************************************************/

ElfSymbol::ElfSymbol(Elf32_Sym sym)
{
  this->offset  = FIX_INT(sym.st_value);
  this->type    = (ELF32_ST_TYPE(sym.st_info));
  this->section = FIX_SHORT(sym.st_shndx);
  this->size    = FIX_INT(sym.st_size);
  this->name    = FIX_INT(sym.st_name);
}

ElfSymbol::ElfSymbol(Elf64_Sym sym)
{
  this->offset  = FIX_INT(sym.st_value);
  this->type    = (ELF64_ST_TYPE(sym.st_info));
  this->section = FIX_SHORT(sym.st_shndx);
  this->size    = FIX_INT(sym.st_size);
  this->name    = FIX_INT(sym.st_name);
}

/*************************************************************************************************************
 ****************************************  Code for class ElfRelocation  *************************************
 *************************************************************************************************************/

ElfRelocation::ElfRelocation(Elf32_Rel header)
{
  this->offset = FIX_INT(header.r_offset);

  unsigned int tempInfo = FIX_INT(header.r_info);
  this->symbol          = ELF32_R_SYM(tempInfo);
  this->type            = ELF32_R_TYPE(tempInfo);
  this->info            = 0;
}

ElfRelocation::ElfRelocation(Elf64_Rel header)
{
  this->offset = FIX_INT(header.r_offset);

  unsigned int tempInfo = FIX_INT(header.r_info);
  this->symbol          = ELF64_R_SYM(tempInfo);
  this->type            = ELF64_R_TYPE(tempInfo);
  this->info            = 0;
}

ElfRelocation::ElfRelocation(Elf32_Rela header)
{
  this->offset = FIX_INT(header.r_offset);

  unsigned int tempInfo = FIX_INT(header.r_info);
  this->symbol          = ELF32_R_SYM(tempInfo);
  this->type            = ELF32_R_TYPE(tempInfo);
  this->info            = 0;
}

ElfRelocation::ElfRelocation(Elf64_Rela header)
{
  this->offset = FIX_INT(header.r_offset);

  unsigned int tempInfo = FIX_INT(header.r_info);
  this->symbol          = ELF64_R_SYM(tempInfo);
  this->type            = ELF64_R_TYPE(tempInfo);
  this->info            = 0;
}
#endif
