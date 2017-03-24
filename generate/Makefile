
CC= gcc
CCOPTS= -Wall -DNDEBUG -O3

# options suitable for an AMD Opteron
#CCOPTS+= -mfpmath=sse -ffast-math -mmmx -msse -m3dnow -march=athlon-xp -mcpu=athlon-xp

# options suitable for an Intel Pentium 4
#CCOPTS+= -mfpmath=sse2 -ffast-math -mmmx -msse -m3dnow -msse2 -march=pentium4 -mcpu=pentium4

LIBS+= -lm

ALL=g_mcar g_sap g_dap g_tap g_hotplate
C_OBJS=flags.o opts.o logger.o

all: $(ALL)

MCAR_OBJS=mcar_prob.o mcar_cont_world.o mcar_disc.o
SAP_OBJS=sap_prob.o sap_cont_world.o sap_disc.o
DAP_OBJS=dap_prob.o dap_cont_world.o dap_disc.o
TAP_OBJS=tap_prob.o tap_cont_world.o tap_disc.o
HOTPLATE_OBJS=hotplate_prob.o hotplate_cont_world.o hotplate_disc.o

g_mcar: $(C_OBJS) $(MCAR_OBJS)
	$(CC) $(CCOPTS) -DMCAR_PROB -o g_mcar gen.c $(C_OBJS) $(MCAR_OBJS) $(LIBS)

g_sap: $(C_OBJS) $(SAP_OBJS)
	$(CC) $(CCOPTS) -DSAP_PROB -o g_sap gen.c $(C_OBJS) $(SAP_OBJS) $(LIBS)

g_dap: $(C_OBJS) $(DAP_OBJS)
	$(CC) $(CCOPTS) -DDAP_PROB -o g_dap gen.c $(C_OBJS) $(DAP_OBJS) $(LIBS)

g_tap: $(C_OBJS) $(TAP_OBJS)
	$(CC) $(CCOPTS) -DTAP_PROB -o g_tap gen.c $(C_OBJS) $(TAP_OBJS) $(LIBS)

g_hotplate: $(C_OBJS) $(HOTPLATE_OBJS)
	$(CC) $(CCOPTS) -DHOTPLATE_PROB -o g_hotplate gen.c $(C_OBJS) $(HOTPLATE_OBJS) $(LIBS)

#
# Generic rules
#

%.o: %.c %.h
	$(CC) $(CCOPTS) -c $<

%.o: %.c
	$(CC) $(CCOPTS) -c $<

mcar_%.o: %.c %.h
	$(CC) $(CCOPTS) -DMCAR_PROB -c $< -o $@

sap_%.o: %.c %.h
	$(CC) $(CCOPTS) -DSAP_PROB -c $< -o $@

dap_%.o: %.c %.h
	$(CC) $(CCOPTS) -DDAP_PROB -c $< -o $@

tap_%.o: %.c %.h
	$(CC) $(CCOPTS) -DTAP_PROB -c $< -o $@

hotplate_%.o: %.c %.h
	$(CC) $(CCOPTS) -DHOTPLATE_PROB -c $< -o $@

clean:
	rm -f *.o *.a *.il *.ti *\~ core core.* $(ALL)
