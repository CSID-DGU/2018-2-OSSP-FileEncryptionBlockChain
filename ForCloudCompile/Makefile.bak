ME = main
OBJS = Node

CXX = g++
CXXFLAGS = -std=c++14 -lssl -lcrypto -lcrypto++ -lpthread -Wall -lboost_system

main.o : Node.cpp
	$(CXX) $(CXXFLAGS) Node.cpp -o $(OBJS)

clean:
	-rm -f *.0 $(EXENAME)