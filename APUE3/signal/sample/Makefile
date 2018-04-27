CC = g++
MAKE = make -w

CURRENT_DIR = $(shell pwd)
OBJs_DIR = $(CURRENT_DIR)/obj
BIN_DIR = $(CURRENT_DIR)/bin
SUB_DIRs = $(shell ls -l | grep ^d | awk '{if($$9 != "bin" && $$9 != "obj") print $$9}')
SOURCEs = ${wildcard *.cpp}
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}
BIN = $(BIN_DIR)/sample

export CC MAKE OBJs_DIR

all : buildenv $(SUB_DIRs) $(OBJs)
	$(CC) -o $(BIN) $(OBJs_DIR)/*.o
	@echo "make end."
.PHONY : all

buildenv:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJs_DIR)

$(SUB_DIRs) :
	@echo "build source in subdirs now:"
	@echo $@
	$(MAKE) -C $@
.PHONY : $(SUB_DIRs)

$(OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $(OBJs_DIR)/$@

clean :
	@rm -r $(OBJs_DIR)
	@rm -r $(BIN_DIR)
.PHONY : clean



