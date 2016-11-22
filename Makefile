CC := gcc
CFLAGS := -O3 -std=gnu99 -Wall -Werror -c
CFLAGS_debug := -O0 -g3 -std=gnu99 -Wall -Werror -c
LFLAGS := -fopenmp -o

TARGET := aes
SOURCE := $(TARGET).c
OBJECTS := $(TARGET).o


all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) $(LFLAGS) $@ $^


$(OBJECTS):$(SOURCE)
	$(CC) $(CFLAGS_debug) $^ -o $@
#$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
