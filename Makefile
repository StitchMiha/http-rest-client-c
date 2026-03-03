CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRC = client.c helpers.c buffer.c requests.c utils.c commands.c parson.c
OBJS = $(SRC:.c=.o)
TARGET = client

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
