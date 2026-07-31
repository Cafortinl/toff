# Compiler Related Variables
CXX        := clang
CXX_FLAGS  := -Wall -Wextra -pedantic #-g `pkg-config --cflags gtk4`

# Project Specific Variables
BIN		   := bin
SRC		   := src
INCLUDE	   := include
LIB		   := lib
LIBRARIES  := -lsqlite3 #`pkg-config --libs gtk4`
EXECUTABLE := toff


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	@echo "Executing...\n"
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	@echo "Building...\n"
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	@echo "Clearing...\n"
	-rm $(BIN)/*
