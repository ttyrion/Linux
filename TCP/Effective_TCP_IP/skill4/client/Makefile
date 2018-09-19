CURRENT_DIR = $(shell pwd | awk 'BEGIN{FS="/"} {print $$NF}')
SOURCEs = ${wildcard *.cpp}
OBJs = ${patsubst %.cpp, $(TEMP_DIR)/%.o, $(SOURCEs)}

.PHONY : all
all : $(OBJs)
	@echo "make in " $(CURRENT_DIR) "succeed."

$(OBJs): $(TEMP_DIR)/%.o:%.cpp
	$(CC) -c $^ -o $@

.PHONY : clean
clean:
	-@rm $(OBJs)