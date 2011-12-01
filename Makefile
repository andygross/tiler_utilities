CC = arm-none-linux-gnueabi-gcc
rootdir = /home/x0hebbar/ion_tiler_user/
CFLAGS = -I$(rootdir)/
#obj = ion.o ion_test.o
ion_tiler_test: ion.o ion_tiler_test.o testlib.o
	$(CC) $(CFLAGS) -o ion_tiler_test ion.o ion_tiler_test.o testlib.o
#ion.o : -I$(rootdir)/linux/
#ion_test.o : -I$(rootdir)/linux/
clean:
	rm -f ion_tiler_test ion.o ion_tiler_test.o testlib.o

