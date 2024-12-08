CC = gcc

# compiler flags:
#  -g    debug info
#  -Wall most warnings enabled
#  -Wno-format-security to avoid printf warnings for runtime strings
CFLAGS  = -Wno-format-security -lm -lSDL2 -lreadline

# build target executable:
TARGET = c8emu

all: $(TARGET)

$(TARGET): main.o graphics.o keyboard.o cpu.o
	$(CC) -o $(TARGET) main.c graphics.o keyboard.o cpu.o $(CFLAGS)
	./c8emu

clean:
	$(RM) $(TARGET)

