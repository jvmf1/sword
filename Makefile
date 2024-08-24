DESTDIR=/usr/local/bin
NAME=sword
CFLAGS=-Wall -Wextra
LIBS=-lslib

all: $(NAME)

$(NAME).o: $(NAME).c
	cc $(CFLAGS) $(LIBS) $(NAME).c -c

$(NAME): $(NAME).o
	cc $(LIBS) $(NAME).o -o $@ 

install: $(NAME)
	mkdir -p $(DESTDIR)
	cp -f $(NAME) $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/$(NAME)

clean:
	rm -f $(NAME)
	rm -f *.o
