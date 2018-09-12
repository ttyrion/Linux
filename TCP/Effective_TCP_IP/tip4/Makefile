
CC := g++ -g -std=c++11
MAKE := make -w

CURRENT_DIR = $(shell pwd)
BIN_DIR = $(CURRENT_DIR)/build/bin
TEMP_DIR = $(CURRENT_DIR)/build/obj
BIN := $(BIN_DIR)/server

export CC MAKE TEMP_DIR BIN

SOURCEs = ${wildcard *.cpp}
CURRENT_OBJs = ${patsubst %.cpp, $(TEMP_DIR)/%.o, $(SOURCEs)}
#SUB_DIRs = $(shell ls -l | grep ^d | awk '{if($$9 != "bin" && $$9 != "obj") print $$9}')
#exclude Build directory
SUB_DIRs = $(shell ls -l | grep ^d | awk '{if($$9 != "build") print $$9}')
BUILD = $(CURRENT_DIR)/build

# A phony target would always be reciped
.PHONY: All
All: buildenv $(BIN)
	@echo "All makes succeed."

#create build direcotry if necessary
buildenv:
	mkdir -p $(TEMP_DIR)
	@echo "create " $(TEMP_DIR)
	mkdir -p $(BIN_DIR)
	@echo "create " $(BIN_DIR)

# BUILD should be made last, it generates the final executed file.
$(BIN): $(SUB_DIRs) $(CURRENT_OBJs) $(BUILD)
	@echo $(BIN) "made."

$(SUB_DIRs) :
	$(MAKE) -C $@
.PHONY : $(SUB_DIRs)

$(CURRENT_OBJs): $(TEMP_DIR)/%.o:%.cpp
	$(CC) -c $^ -o $@

.PHONY: $(BUILD)
$(BUILD):
	$(MAKE) -C $@

.PHONY: clean
clean:
	-@rm -r $(BIN_DIR)
	-@rm -r $(TEMP_DIR)




