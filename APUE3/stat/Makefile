CC := g++
MAKE := make -w

# all the source file of current directory
SOURCEs = ${wildcard *.cpp}
# all the obj file would be made by source file
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}
BIN := ${patsubst %.cpp, %, $(SOURCEs)}

.PHONY: All
All: $(BIN)
	@echo "All makes succeed."

#Wrong: "$<" here represents the first prereq file, so both target would be make from the same .o object file
#$(BIN): $(OBJs)
#	@echo "making" $(BIN)
#	@echo "making" $@ "with" $<
#	$(CC) -o $@ $<

#Right: "$<" represents the prereq set and the prereq file would be get one by one("$<" is auto variable).
#Why? Because the prereq set here is defined by pattern("%")
$(BIN): %:%.o
	@echo "making" $(BIN)
	@echo "making" $@ "with" $<
	$(CC) -o $@ $<

#static mode, we get prereq-set(.cpp) from target-set(.o) according to the two pattern
$(OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $@
	@echo "making" $@

.PHONY: clean
clean:
	-@rm $(BIN)
	-@rm $(OBJs)



