all:
	gcc protocolo.c application.c  files.c writenoncanonical.c -lrt -lm -Wall -g -o writer
	gcc protocolo.c application.c  files.c noncanonical.c  -lrt -lm -Wall -g -o reader

clean:
	rm writer
	rm reader
