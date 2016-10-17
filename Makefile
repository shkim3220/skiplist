CC = gcc
CFLAGS= -D WLOCK -g
LDLIBS= -lpthread

all : skip_wlock skip_plock

skip_wlock :
	$(CC) $(CFLAGS) -o skip_wlock skip.c $(LDLIBS)

skip_plock :
	$(CC) -g -o skip_plock skip.c $(LDLIBS)

clean :
	rm -rf skip_*
