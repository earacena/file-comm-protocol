OBJS = main.cpp Protocol.cpp Logger.cpp
FLAGS = -g -Wall -pedantic
EXEC = netnode
#LIBS = 
#INCLUDE = 

all:
	g++ $(OBJS) -o $(EXEC) $(FLAGS) 
