CC = g++ -g -std=c++11
MAKE = make -w

CURRENT_DIR = $(shell pwd)
#The tmp directory to hold object files
OBJs_DIR = $(CURRENT_DIR)/obj
#The target executable file
BIN_DIR = $(CURRENT_DIR)

#All the source files in current directory
CURRENT_DIR_SOURCEs = ${wildcard *.cpp}
CURRENT_DIR_OBJs = ${patsubst %.cpp, %.o, $(CURRENT_DIR_SOURCEs)}
BINs = ${patsubst %.cpp, %, $(CURRENT_DIR_SOURCEs)}

.PHONY : all
all : buildenv $(BINs)
	@echo "make all succeed."

$(BINs) : %:%.o
	$(CC) -o $@ $^

%.o : %.cpp
	$(CC) -c $^ -o $@

#create build direcotry if necessary
buildenv:
	mkdir -p $(OBJs_DIR)
	@echo "create " $(OBJs_DIR)

#A phony target : it's not really the name of a file
#1. A phony target's recipe would always be executed when explicitly specified as the param of "make"
# (except for the first target of a Makefile).
#2. A phony target's prerequisites(if it has any) would always be deduced
# ("Make" deduces prerequisites's rules and then make them if necessary).
.PHONY : clean
clean :
	@rm -r $(OBJs_DIR)























