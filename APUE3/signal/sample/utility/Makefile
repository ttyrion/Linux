SUB_DIRs = $(shell ls -l | grep ^d | awk '{print $$9}')
SOURCEs = ${wildcard *.cpp}
OBJs = ${patsubst %.cpp, %.o, $(SOURCEs)}

all : $(SUB_DIRs) $(OBJs)

#equals : cd $@; $(MAKE)
$(SUB_DIRs) :
	$(MAKE) -C $@

$(OBJs) : %.o:%.cpp
	@echo $(OBJs_DIR)
	$(CC) -c $^ -o $(OBJs_DIR)/$@

