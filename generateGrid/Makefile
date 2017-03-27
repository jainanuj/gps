
CC= gcc
CCOPTS= -Wall -g -DLOGGER_ALWAYS_FLUSH

all: createGrid

#C_OBJS=createGrid.o

createGrid_OBJS=createGrid.o


createGrid: $(createGrid_OBJS)
	$(CC) $(CCOPTS) $(INCLUDES) -o createGrid $(createGrid_OBJS)

#
# Generic rules
#

%.o: %.c %.h
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

clean:
	rm -f *.o *.a *.il *.ti *\~ core core.* createGrid
