build: main.o
	cc main.o -o zbatt `pkg-config --cflags --libs gtk+-2.0`

main.o: main.c
	cc -c main.c `pkg-config --cflags gtk+-2.0`

clean:
	rm *.o
	rm zbatt

uninstall:
	rm $(PREFIX)/bin/zbatt
