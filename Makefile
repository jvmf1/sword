DESTDIR=/usr/local/bin
NAME=sword
CFLAGS=-Wall -Wextra
LIBS=-lslib
CC=cc

all: $(NAME)

$(NAME).o: $(NAME).c
	$(CC) $(CFLAGS) $(LIBS) $(NAME).c -c

$(NAME): $(NAME).o
	$(CC) $(LIBS) $(NAME).o -o $@

install: $(NAME)
	mkdir -p $(DESTDIR)
	cp -f $(NAME) $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/$(NAME)

clean:
	rm -f $(NAME)
	rm -f *.o
