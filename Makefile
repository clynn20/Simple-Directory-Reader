CC = gcc
CFLAGS = -std=gnu99

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = reader

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -rf $(TARGET) $(OBJECTS)