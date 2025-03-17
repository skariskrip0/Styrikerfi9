CC=     gcc
LD=     ld
CFLAGS= -g -W -Wall -Werror -std=c17 -fopenmp
TARGET= malloc
SRC=    malloc.c testlib.c main.c test-malloc.c
OBJ=    $(SRC:.c=.o)

malloc: $(OBJ)
	$(CC) -pthread -fopenmp -o $@ $(OBJ)

.PHONY: $(TARGET)-solution.o
$(TARGET)-sol: $(subst $(TARGET),$(TARGET)-solution,$(OBJ))
	$(CC) -pthread -fopenmp -o $@ $^

$(TARGET)-solution.o:
	@test -f $(TARGET)-solution.c \
	|| { echo "Please download solution." >&2; false; }
	$(CC) $(CFLAGS) -c $(TARGET)-solution.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

sol:	malloc-sol

clean:
	rm -rf $(TARGET) $(TARGET)-sol *.o

showsrc:
	@echo malloc.c test-malloc.c improvements.txt

zip:
	cd ..; ./makezip.py
