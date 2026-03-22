CC = gcc
CFLAGS = -O3 -march=native -mtune=native -funroll-loops
LDFLAGS = -lm
BIN = iqproc
SRC = src/main.c src/process.c

.PHONY: all clean
all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LDFLAGS)

clean:
	rm -f $(BIN) *.o
