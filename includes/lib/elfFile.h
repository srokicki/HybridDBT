
#ifdef __LINUX_API

#ifndef __ELFFILE
#define __ELFFILE

#include <lib/elf.h>
#include <string>
#include <vector>

class ElfSection;
class ElfRelocation;
class ElfSymbol;

class ElfFile {
public:
  Elf32_Ehdr fileHeader32;
  Elf64_Ehdr fileHeader64;

  int is32Bits;

  std::vector<ElfSection*>* sectionTable;
  std::vector<std::string>* nameTable;
  std::vector<ElfSymbol*>* symbols;

  int indexOfSymbolNameSection;

  ElfFile(char* pathToElfFile);
  ElfFile* copy(char* newDest);

  FILE* elfFile;
  char* pathToElfFile;
};

class ElfSection {
public:
  ElfFile* containingElfFile;
  int id;

  unsigned int size;
  unsigned int offset;
  unsigned int nameIndex;
  unsigned int address;
  unsigned int type;
  unsigned int info;

  // General functions
  std::string getName();

  // Test for special section types
  bool isRelSection();
  bool isRelaSection();

  // Functions to access content
  unsigned char* getSectionCode();
  std::vector<ElfRelocation*>* getRelocations();
  void writeSectionCode(unsigned char* newContent);
  void writeSectionCode(FILE* file, unsigned char* newContent);

  // Class constructor
  ElfSection(ElfFile* elfFile, int id, Elf32_Shdr header);
  ElfSection(ElfFile* elfFile, int id, Elf64_Shdr header);
};

class ElfSymbol {
public:
  unsigned int name;
  unsigned int type;
  unsigned int offset;
  unsigned int size;
  unsigned int section;
  unsigned int value;

  // Class constructors
  ElfSymbol(Elf32_Sym);
  ElfSymbol(Elf64_Sym);
};

class ElfRelocation {
public:
  unsigned int offset;
  unsigned int symbol;
  unsigned int type;
  unsigned int info;

  // Class constructors
  ElfRelocation(Elf32_Rel);
  ElfRelocation(Elf32_Rela);
  ElfRelocation(Elf64_Rel);
  ElfRelocation(Elf64_Rela);
};

#endif
#endif
