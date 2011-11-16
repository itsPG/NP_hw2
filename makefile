main: np_hw2.cpp np_hw2_type2.cpp
	g++ -o 2_hw2 np_hw2_type2.cpp

clean:
	 rm -rf hw2 server client *.o
