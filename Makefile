    CC     = gcc -g
    CFLAGS = 
    LFLAGS =

      PROG = Uno-Online
      OBJS = uno.o \
             $(PROG).o

.PHONY: clean purge all

%.o: %.c %.h 
	$(CC) -c $(CFLAGS) $<

$(PROG):  $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	@rm -f *~ *.bak

purge:   clean
	@rm -f *.o core
	@rm -f $(PROG)