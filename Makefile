# @file Makefile
# BES mypopen
# Projekt 2
#
# authors:
#
# Binder Patrik         	<ic19b030@technikum-wien.at>
# Ferchenbauer Nikolaus		<ic19b013@technikum-wien.at>
# Pittner Stefan        	<ic19b003@technikum-wien.at>
# @date 2020/27/04
#
# @version 1.7.0
#
# --------------------------------------------------------------------------

# ------------------------------------------------------------- variables --
# tools and options:

CC     =  gcc
CFLAGS = -Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic \
         -fno-common -ftrapv -O3 -g -c -std=gnu11 
TFLAGS = -lpopentest -ldl
RM     =  rm -f

# filenames:

EXEC   = popentest
OBJ    = mypopen.o
SRC    = mypopen.c
HEADER = mypopen.h
DOC    = doc
DOXY   = doxygen
DFILE  = Doxyfile

# --------------------------------------------------------------- targets --
# .PHONY

.PHONY: all clean doc distclean cleanall

all: $(EXEC)


clean:
	$(RM) $(EXEC) $(OBJ)

doc:
	$(DOXY) $(DFILE)

distclean:
	$(RM) -r $(DOC)

cleanall:
	$(RM) -r $(EXEC) $(OBJ) $(DOC)

# rules:

$(EXEC): $(OBJ)
	$(CC) -o $(EXEC) $(OBJ) $(TFLAGS)

$(OBJ): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) $<

# --------------------------------------------------------------------------


