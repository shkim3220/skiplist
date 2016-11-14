CC = gcc
LDLIBS= -lpthread

all : skip_wlock skip_plock

skip_wlock :
	$(CC) -g -o skip_ori ori_skip.c $(LDLIBS)

skip_plock :
	$(CC) -g -o skip_new skip.c $(LDLIBS)

clean :
	rm -rf skip_*
