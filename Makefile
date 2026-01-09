CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags libpng sdl2)
LDFLAGS = $(shell pkg-config --libs libpng sdl2)

SRCS = main.c png_utils.c steg_text.c steg_image.c display_sdl.c
OBJS = $(SRCS:.c=.o)

TARGET = steg_png

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

