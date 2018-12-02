ME = main
OBJS = Node

CXX = g++
CXXFLAGS = -std=c++17 -lssl -lcrypto -lcrypto++ -lpthread -Wall -lboost_system -lstdc++fs

main.o : Node.cpp
	$(CXX) $(CXXFLAGS) Node.cpp -o $(OBJS)

clean:
	-rm -f *.0 $(EXENAME)