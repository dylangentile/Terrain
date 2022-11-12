CC := clang
CFLAGS := --std=c11 -O3 -march=native -Wall -Wextra -pedantic -fno-fast-math
LDFLAGS := -lm -pthread

TARGET := terra
OBJ :=  main.o erosion.o perlin.o world.o smp.o stb_image_write.o

.PHONY: all clean depend

all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

depend: 
	$(CC) $(CFLAGS) -E -MM $(OBJ:.o=.c) > .depend

clean:
	-rm *.o $(TARGET)

include .depend