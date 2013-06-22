all: autofd.h epollfd.h epollfd.cpp test.cpp
	g++ -Wall -Wextra -Werror -std=c++11 -g -o test test.cpp epollfd.cpp
	g++ -Wall -Wextra -Werror -std=c++11 -g -o async test_ao.cpp epollfd.cpp

