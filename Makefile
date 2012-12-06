CC = $(CROSS_COMPILE)gcc
CFLAGS = --static -Werror -Wall
#obj = ion.o ion_test.o
ion_tiler_test: ion.o ion_tiler_test.o testlib.o
	$(CC) $(CFLAGS) -o ion_tiler_test ion.o ion_tiler_test.o testlib.o
#ion.o : -I$(rootdir)/linux/
#ion_test.o : -I$(rootdir)/linux/
clean:
	rm -f ion_tiler_test ion.o ion_tiler_test.o testlib.o

