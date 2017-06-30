
CC= gcc
CCOPTS= -Wall -g -DLOGGER_ALWAYS_FLUSH

all: compareResults

#C_OBJS=compareResults.o

compareResults_OBJS= compareResults.o


compareResults: $(compareResults_OBJS)
	$(CC) $(CCOPTS) $(INCLUDES) -o compareResults $(compareResults_OBJS)

#
# Generic rules
#

%.o: %.c %.h
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

clean:
	rm -f *.o *.a *.il *.ti *\~ core core.* compareResults
