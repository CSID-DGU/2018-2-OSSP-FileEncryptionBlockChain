ME = main
OBJS = SendSign

CXX = g++
CXXFLAGS = -std=c++14 -lssl -lcrypto -lcrypto++ -lpthread -Wall -lboost_system

main.o : main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(OBJS)

clean:
	-rm -f *.0 $(EXENAME)