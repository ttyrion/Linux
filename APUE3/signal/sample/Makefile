CC = g++
MAKE = make -w

CURRENT_DIR = $(shell pwd)
#The tmp directory to hold object files
OBJs_DIR = $(CURRENT_DIR)/obj
#The target executable file
BIN_DIR = $(CURRENT_DIR)/bin
BIN = $(BIN_DIR)/sample

SUB_DIRs = $(shell ls -l | grep ^d | awk '{if($$9 != "bin" && $$9 != "obj") print $$9}')
#All the source files in current directory
CURRENT_DIR_SOURCEs = ${wildcard *.cpp}
CURRENT_DIR_OBJs = ${patsubst %.cpp, %.o, $(CURRENT_DIR_SOURCEs)}

#export these variables which would be used when executing make in subdirs
export CC MAKE OBJs_DIR

.PHONY : all
all : buildenv $(SUB_DIRs) $(CURRENT_DIR_OBJs)
	$(CC) -o $(BIN) $(OBJs_DIR)/*.o
	@echo "make all succeed."

#create build direcotry if necessary
buildenv:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJs_DIR)
	@echo "create " $(BIN_DIR)
	@echo "create " $(OBJs_DIR)

#The target here is a set of many files.
$(SUB_DIRs) :
	@echo "build in subdir " $@ " now:"
	$(MAKE) -C $@
.PHONY : $(SUB_DIRs)

$(CURRENT_DIR_OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $(OBJs_DIR)/$@

#A phony target : it's not really the name of a file
#1. A phony target's recipe would always be executed when explicitly specified as the param of "make"
# (except for the first target of a Makefile).
#2. A phony target's prerequisites(if it has any) would always be deduced
# ("Make" deduces prerequisites's rules and then make them if necessary).
.PHONY : clean
clean :
	@rm -r $(OBJs_DIR)
	@rm -r $(BIN_DIR)























