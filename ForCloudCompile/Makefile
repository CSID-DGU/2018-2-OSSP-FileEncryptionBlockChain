ME = main
OBJS = Node

CXX = g++
CXXFLAGS = -std=c++17 -lstdc++fs -lssl -lcrypto -lcrypto++ -lpthread -Wall -lboost_system 

main.o : Node.cpp
	$(CXX) Node.cpp -o $(OBJS) $(CXXFLAGS) 

clean:
	-rm -f *.0 $(EXENAME)