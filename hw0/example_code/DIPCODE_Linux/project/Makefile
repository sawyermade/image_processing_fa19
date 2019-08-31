#all:
	#cd ../iptools; make
#	g++ -o bin/test test.cpp  -L ../lib -l iptools 
#	g++ -o bin/test2 test2.cpp  -L ../lib -l iptools -l mpeg 

	# source files.

BIN_DIR=bin/

EXEC = iptool


all: $(EXEC)

iptools:
	(cd ../iptools; make)

clean:
	$(RM) $(BIN_DIR)$(EXEC); make all

DO_EXEC = g++ -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools

#------ now compile
$(EXEC):   		iptools
				$(DO_EXEC)
