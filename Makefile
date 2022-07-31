PROJECT = server

CPP = $(wildcard *.cpp)

LIBS = event

CXX = g++

FLAGS = -Wall

OBJ = $(CPP:.cpp=.o)

%.o: %.cpp
	$(CXX) $(FLAGS) -c $^ -o $@

all: $(OBJ)
	$(CXX) $(FLAGS) $(OBJ) -o $(PROJECT) $(addprefix -l, $(LIBS))
