CC = g++
MAKE = make -w

CURRENT_DIR = $(shell pwd)
BIN_DIR = $(CURRENT_DIR)/bin
OBJs_DIR = $(CURRENT_DIR)/obj

# ^d means that the first char of line must be 'd', here means a directory
# $9 reference the 9th field(of 'ls -l' is file name) of each line
SUB_DIRs = $(shell ls -l | grep ^d | awk '{if($$9 != "bin" && $$9 != "obj") print $$9}')

# all the source file of current directory
SOURCEs = ${wildcard *.cpp}
# all the obj file would be made by source file
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}
# the bin file we want to get finaly
BIN = $(BIN_DIR)/ls

#export variables into Shell, 
#which means export them into the makefiles in subdirs
export CC MAKE OBJs_DIR

all : buildenv $(SUB_DIRs) $(OBJs)
	$(CC) -o $(BIN) $(OBJs_DIR)/*.o
	@echo "make end."
.PHONY : all

buildenv :
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJs_DIR)
.PHONY : buildenv

#equals : cd %@; $(MAKE)
#That is, we will execute 'make -w' on each subdir
$(SUB_DIRs) :
	$(MAKE) -C $@
.PHONY : $(SUB_DIRs)

#static mode, we get prereq-set(.cpp) from target-set(.o) according to the two pattern
$(OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $(OBJs_DIR)/$@

clean :
	@rm -r $(OBJs_DIR)
	@rm -r $(BIN_DIR)
.PHONY : clean




