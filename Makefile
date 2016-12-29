#CC := gcc
CC := LIBRARY_PATH=~/forge/openmp4/llvm/install/lib ~/forge/openmp4/llvm/build/bin/clang-3.8
#testing flags
CFLAGS := -fopenmp -omptargets=nvptx64sm_52-nvidia-linux -O3 -std=gnu99 -Wall -Werror -c

#debug flags
#CFLAGS := -fopenmp -omptargets=nvptx64sm_52-nvidia-linux -O0 -g3 -std=gnu99 -Wall -Werror -c
LFLAGS := -fopenmp -omptargets=nvptx64sm_52-nvidia-linux -o

TARGET := gpu
SOURCE := $(TARGET).c
OBJECTS := $(TARGET).o
OUTPUT_DIR := test_files/output


all:$(TARGET)

$(TARGET):$(OBJECTS)
	mkdir -p test_files/output/
	$(CC) $(LFLAGS) $@ $^

exec:$(TARGET)
	./$(TARGET) test_files/text_test.txt

gdb:$(TARGET)
	gdbtui -x gdb.txt --args  ./aes_gpugg ../test_files/text_test.txt


$(OBJECTS):$(SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(OUTPUT_DIR)/*

astyle:
	astyle --style=linux --indent=tab -p -U -K -H --suffix=none ./aes_gpugg.c
