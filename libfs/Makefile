# Target library
lib := libfs.a
objs := fs.o disk.o

all: $(lib)

libfs.a: $(objs)
	ar rcs $(lib) $(objs)

fs.o: fs.c
	cc -c -g -Wall -Werror fs.c

disk.o: disk.c
	cc -c -g -Wall -Werror disk.c

clean:
	rm -f $(lib) $(objs)
