SRC = ./src
BIN = ./bin
OBJ = ./obj
INC = ./include

all: $(BIN)/main

$(OBJ)/main.o: $(INC)/decision_tree.h $(SRC)/main.cpp 
	g++ -std=c++11 -c -I$(INC) $(SRC)/main.cpp -o $(OBJ)/main.o
	
$(OBJ)/decision_tree.o: $(INC)/decision_tree.h $(SRC)/decision_tree.cpp 
	g++ -std=c++11 -c -I$(INC) $(SRC)/decision_tree.cpp -o $(OBJ)/decision_tree.o
	
$(BIN)/main: $(OBJ)/main.o $(OBJ)/decision_tree.o
	g++ -std=c++11 -O2 $(OBJ)/main.o $(OBJ)/decision_tree.o  -o $(BIN)/main	
	
clean:
	rm -f $(OBJ)/*.o $(BIN)/*