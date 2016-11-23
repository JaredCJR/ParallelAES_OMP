CC := gcc
#CFLAGS := -O3 -std=gnu99 -Wall -Werror -c
CFLAGS := -O0 -g3 -std=gnu99 -Wall -Werror -c
LFLAGS := -fopenmp -o

TARGET := aes
SOURCE := $(TARGET).c
OBJECTS := $(TARGET).o
OUTPUT_DIR := test_files/output

exec:$(TARGET)
	./$(TARGET) test_files/text_test.txt

all:$(TARGET)

$(TARGET):$(OBJECTS)
	mkdir -p test_files/output/
	$(CC) $(LFLAGS) $@ $^

gdb:$(TARGET)
	gdbtui -x gdb.txt --args  ./aes test_files/text_test.txt


$(OBJECTS):$(SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(OUTPUT_DIR)/*

astyle:
	astyle --style=linux --indent=tab -p -U -K -H --suffix=none ./aes.c
