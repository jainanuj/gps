
#AZ_PLATFORM=LINUX
#AZ_IMPL=SERIAL
##AZ_IMPL=MPI
#AZTEC=./Aztec2.1

#LIBS= -L$(AZTEC)/lib -lg2c -laztec -lm
#INCLUDES= -I$(AZTEC)/lib

LIBS= -lm
INCLUDES= 

# Specify the location of your Apache Portable Runtime
# This is only if you want to dump some stuff for METIS.
# Hopefully, this will go away.
#APRLIBS=$(shell /usr/local/apr/bin/apr-config --libs --link-ld)
#APRINC=-I$(shell /usr/local/apr/bin/apr-config --includedir)

#MPICH=/usr/local/mpich-1.2.5.2
#MPICH=/usr/local/mpich-1.2.7p1

USE_MPI=0

CC= gcc

#
# -----------------------------------------------------------------------------
# Production options
#

CCOPTS= -Wall -g -DLOGGER_ALWAYS_FLUSH

#CCOPTS+= -DNDEBUG -O3

#CCOPTS+= -DNDEBUG -DUSE_AZTEC -O3
# -O3
# -g -pg
# 

# options suitable for an AMD Opteron
#CCOPTS+= $(COM_OPTS) -mfpmath=sse -ffast-math -mmmx -msse -m3dnow -march=athlon-xp -mcpu=athlon-xp

#CCOPTS+= $(COM_OPTS) -mfpmath=sse -ffast-math -mmmx -msse -m3dnow -march=athlon-xp -mcpu=athlon-xp


# options suitable for an Intel Pentium 4
#CCOPTS+= $(COM_OPTS) -mfpmath=sse2 -ffast-math -mmmx -msse -m3dnow -msse2 -march=pentium4 -mcpu=pentium4

# general options suitable for all architectures
#CCOPTS+= $(COM_OPTS)

#
# -----------------------------------------------------------------------------
#

all: gps

#aztec:  $(AZTEC)/lib/libaztec.a
#$(AZTEC)/lib/libaztec.a:
#	./build_aztec $(AZ_PLATFORM) $(AZ_IMPL)

C_OBJS=bfuncs.o intheap.o med_hash.o sanity.o flags.o opts.o part_stuff.o logger.o odcd.o small-matvec.o stok.o

GPS_OBJS=gps.o $(C_OBJS)

ifeq ($(USE_MPI),1)
CCOPTS += -DUSE_MPI
GPS_OBJS += par_mpi.o
LIBS += -L$(MPICH)/lib -lmpich
INCLUDES += -I$(MPICH)/include
endif

#gps: $(GPS_OBJS) $(AZTEC)/lib/libaztec.a
#	$(CC) $(CCOPTS) -o gps $(GPS_OBJS) $(LIBS)

gps: $(GPS_OBJS)
	$(CC) $(CCOPTS) $(INCLUDES) -o gps $(GPS_OBJS) $(LIBS)

#
# Generic rules
#

%.o: %.c %.h
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CCOPTS) $(INCLUDES) -c $<

clean:
	rm -f *.o *.a *.il *.ti *\~ core core.* gps geomview
