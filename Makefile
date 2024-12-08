#the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#  -Wno-format-security to avoid printf warnings for runtime strings
CFLAGS  = -Wno-format-security -lm -lSDL2 -lreadline

# the build target executable:
TARGET = c8emu

all: $(TARGET)

$(TARGET): main.o graphics.o keyboard.o cpu.o
	$(CC) -o $(TARGET) main.c graphics.o keyboard.o cpu.o $(CFLAGS)
	./c8emu

clean:
	$(RM) $(TARGET)

