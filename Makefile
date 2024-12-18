CC = gcc
CFLAGS = -I/usr/include/SDL2
LDFLAGS = -lSDL2
TARGET = fluid

all: $(TARGET)

$(TARGET): fluid.c
	rm -f $(TARGET)
	$(CC) -o $(TARGET) fluid.c $(CFLAGS) $(LDFLAGS)
	./$(TARGET)
