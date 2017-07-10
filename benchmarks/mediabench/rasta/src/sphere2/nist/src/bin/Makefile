# **********************************************************
# Project:		ARG General libraries
# SubTree:		/export/ramoth4/bishop/simplescalar/mediabench/rasta/src/sphere2/nist/src/progs/ h_add h_delete h_edit h_read h_strip h_test tsphere w_decode w_diff w_edit w_encode 
# Filename:		Makefile
# Programmer:		J. Fiscus
# Organization:		NIST/NCSL/DIV 670/Auto Rec. Group
# Host System:		SUN 4 OS/4.1.1 (UNIX)
# Date Created:		05/28/92
#
# **********************************************************
# Makefile for SRC Binary Directory
# **********************************************************

PROJECT_ROOT	= /export/ramoth4/bishop/simplescalar/mediabench/rasta/src/sphere2/nist
SHELL = /bin/sh

# **************************************************
# INDIVIDUAL PROGRAM MODIFICATIONS SHOULD BEGIN HERE
# **************************************************

# Program to make (binary name)
EXECUTABLE	= h_add h_delete h_edit h_read h_strip h_test tsphere w_decode w_diff w_edit w_encode 
# Supporting functions to make if any (They will be included in every compile)
OBJ		= 
# List all .c source code files
SRC		= h_add.c h_delete.c h_edit.c h_read.c h_strip.c h_test.c tsphere.c w_decode.c w_diff.c w_edit.c w_encode.c 
# Enable converting the distribution to K&R C
# KRC             = $(SRC:%.c=%.kr.c)

# LIBS needed to compile the EXECUTABLE
# (Full paths eg: /usr/local/image/lib/libimage.a)
# use $(LIBDIR) defined above for path extensions if appropriate
LIBS	= $(PROJECT_ROOT)/lib
# How the libraries look when invoked on the compile line (eg: -limage)
LLIBS	= -lsp -lutil  -lm
# Local additions for CFLAG options (eg: -g)
LOCAL_CFLAGS	= -g -static -DNARCH_SimpleScalar

# *******************************************************
# THE REST OF THE MAKEFILE SHOULD NOT NEED TO BE MODIFIED
#       (EXCEPT UPON APPROVAL OF PROJECT MANAGER)
# *******************************************************

BIN	= $(PROJECT_ROOT)/bin
INCLUDE = $(PROJECT_ROOT)/include
LIBDIR	= $(LIBS)

CFLAGS	= -I$(INCLUDE) -L$(LIBDIR) $(LOCAL_CFLAGS)
CC	= /export/ramoth4/bishop/simplescalar/bin/ssbig-na-sstrix-gcc 
INSTALL	= install -s -m 755

MAKEFILE	= Makefile
.PRECIOUS: $(MAKEFILE)

#.c: $(BIN)/$@ stub.o
#	$(CC) $@.c $(FUNCT_O) $(LLIBS) -o $@
#
#.c.o:
#	cc -c $@.c

it: $(EXECUTABLE)

install: $(MAKEFILE) $(EXECUTABLE)
	$(INSTALL) $(EXECUTABLE) $(BIN)

#build_kr: $(KRC)
#$(KRC): $@
#	cp `echo $@ | sed 's/.kr//'` $@
#	unprotoize -c "-I$(INCLUDE)" $@
#	rm -f $@.save

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $@.c $(OBJ) $(LLIBS) -o $@

# if there are other separate programs to compile, add the name to
#	SRC, OBJ, ... and the redo the last 3 lines EXPLICITLY
#	for each program.
#	Install will also have to be changed.

clean :
	rm -f *.o $(EXECUTABLE) core a.out *.BAK

bare: clean
	rm -f $(EXECUTABLE)

$(MAKEFILE): $(SRC)
	$(CC) $(CFLAGS) -M $(SRC) > dependlist
	@sed -e '1,/^# DO NOT DELETE/!d' $(MAKEFILE) > $(MAKEFILE).tmp.$$$$; \
	cat dependlist >> $(MAKEFILE).tmp.$$$$; \
	cp $(MAKEFILE) $(MAKEFILE).BAK; \
	mv $(MAKEFILE).tmp.$$$$ $(MAKEFILE); \
	rm -f dependlist;

depend: $(SRC)
	$(CC) $(CFLAGS) -M $(SRC) > dependlist
	@sed -e '1,/^# DO NOT DELETE/!d' $(MAKEFILE) > $(MAKEFILE).tmp.$$$$; \
	cat dependlist >> $(MAKEFILE).tmp.$$$$; \
	cp $(MAKEFILE) $(MAKEFILE).BAK; \
	mv $(MAKEFILE).tmp.$$$$ $(MAKEFILE); \
	rm -f dependlist; \
	echo make depend complete

# DO NOT DELETE THIS LINE - make depend uses it
