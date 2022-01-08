all: cache

cache: cache.c
	gcc -o cache cache.c

clean: cache
	rm cache