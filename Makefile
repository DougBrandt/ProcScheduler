# Makefile

all: schedule two

schedule: schedule.c schedule.h
	gcc -Wall -Werror -g -o $@ schedule.c schedule.h

two: two.c
	gcc -Wall -Werror -g -o $@ two.c

clean:
	rm -f schedule two

