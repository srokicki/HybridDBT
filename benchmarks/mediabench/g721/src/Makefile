# Makefile for CCITT subroutines and sample programs
#

# Modify the CC definition to invoke your ANSI-C compiler
#

#############################
# Modified by C. Lee
CC =  /home/simon/Documents/Recherche/RISCV-toolchain/riscv/bin/riscv64-unknown-elf-gcc # gcc -mv8 -static
CFLAGS = -O3 -msoft-float -march=RV64IM

# Rules to build sample programs do not depend on a UNIX library program
#
ALL = encode decode
all: $(ALL)
	cp encode decode ../bin/.

OBJECTS = g711.o g72x.o g721.o g723_24.o g723_40.o

encode: $(OBJECTS) encode.c
	$(CC) -o $@ encode.c $(OBJECTS)

decode: $(OBJECTS) decode.c
	$(CC) -o $@ decode.c $(OBJECTS)


# Library rules for UNIX systems
#
LIB = ccitt.a
lib: $(LIB)

$(LIB): $(OBJECTS)
	@rm -f $(LIB)
	ar cr $(LIB) $(OBJECTS)
	@ranlib $(LIB)

clean:
	/bin/rm -f $(LIB) $(ALL) *.o ; /bin/rm -f ../bin/*
