main: np_hw2_shell.cpp np_hw2_type1.cpp np_hw2_type2.cpp
	g++ -o 2_hw2 np_hw2_type2.cpp
	g++ -o 1_hw2 np_hw2_type1.cpp

clean:
	 rm -rf 1_hw2 2_hw2 *.o
