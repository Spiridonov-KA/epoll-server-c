CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE

BIN = epoll-server
SRC = src/epoll-server.c src/string_utilities.c src/utilities.c src/list.c src/statistic_utilities.c
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

deb:
	dpkg-buildpackage -us -uc -b

clean:
	rm -f $(BIN) $(OBJ) *.deb *.changes *.buildinfo

.PHONY: all deb clean