OBJ:=$(patsubst %.c,%.o,$(wildcard *.c))

mini.exe: $(OBJ)
	gcc -o $@ $^

clean:
	rm -f *.o *.exe