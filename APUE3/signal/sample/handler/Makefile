SOURCEs = ${wildcard *.cpp}
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}

.PHONY : all
all : $(OBJs)
	@echo "make all in handler succeed."

$(OBJs) : %.o:%.cpp
	$(CC) -c $^ -o $(OBJs_DIR)/$@
	@echo "make " $(OBJs_DIR)/$@ "succeed."
