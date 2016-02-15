all:   vdl120
vdl120:  src/vdl120.c
	gcc -o $@ src/vdl120.c -lusb -lrt -Wall

install: vdl120
	cp -v vdl120 /usr/bin/

clean:
	rm vdl120
