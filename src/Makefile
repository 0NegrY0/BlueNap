# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -std=c++11 -pthread

# Source files
SRC = Subservices/Discovery_Subservice.cpp Subservices/Monitoring_Subservice.cpp main.cpp utils.cpp Subservices/Discovery_Subservice.hpp Subservices/Monitoring_Subservice.hpp utils.hpp

# Output binary
BIN = bluenap.exe

# Make all
all: $(BIN)

# Link
$(BIN): $(SRC)
    $(CC) $(CFLAGS) -o $(BIN) $(SRC)

# Clean
clean:
    rm -f $(BIN)