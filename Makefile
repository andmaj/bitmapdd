all:
	$(CC) $(CFLAGS) -Wall -o bin/bitmapdd src/bitmapdd.c

clean:
	rm bin/bitmapdd
