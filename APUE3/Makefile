BIN := stat
CC := g++
MAKE := make -w

# all the source file of current directory
SOURCEs = ${wildcard *.cpp}
# all the obj file would be made by source file
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}

.PHONY: All
All: $(BIN)
	@echo "All makes succeed."

$(BIN): $(OBJs)
	@echo "making" $(BIN)
	$(CC) -o $(BIN) $(OBJs)

#static mode, we get prereq-set(.cpp) from target-set(.o) according to the two pattern
$(OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $@
	@echo "making" $@

.PHONY: clean
clean:
	-@rm $(BIN)
	-@rm $(OBJs)



