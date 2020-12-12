CC = `g++ -std=c++11`
OPENCV =  `pkg-config --cflags --libs opencv`
PTHREAD = -pthread

CLIENT = client.cpp
SERVER = server.cpp
CLI = client
SER = server
testc = testc.cpp
tests = tests.cpp

all: server client openCV
  
server: $(SERVER)
	$(CC) $(SERVER) -o $(SER)  $(OPENCV) $(PTHREAD) 
client: $(CLIENT)
	$(CC) $(CLIENT) -o $(CLI)  $(OPENCV) $(PTHREAD)
opencv: openCV.cpp
	$(CC) openCV.cpp -o openCV $(OPENCV)
testc: $(testc)
	$(CC) $(testc) -o testc  $(OPENCV) $(PTHREAD) 
tests: $(tests)
	$(CC) $(tests) -o tests  $(OPENCV) $(PTHREAD) 

.PHONY: clean

clean:
	rm $(CLI) $(SER) openCV
