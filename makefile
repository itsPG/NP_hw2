main:  np_hw2_type2.o np_hw2_type1.o np_hw2_shell.cpp
	g++ -o 2_hw2 np_hw2_type2.o
	g++ -o 1_hw2 np_hw2_type1.o

clean:
	 rm -rf hw2 server client *.o
