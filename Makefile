
CC=gcc
CFLAGS=-Wall $(COPT)
CLIBS=-lnfc

all: ulread ulwrite

common.o: common.h common.c
	$(CC) $(CFLAGS) -c common.c

ulread: common.o ulread.o ultralight.o
	$(CC) $(CFLAGS) common.o ulread.o ultralight.o -o ulread $(CLIBS)

ulread.o: common.h ultralight.h ulread.c
	$(CC) $(CFLAGS) -c ulread.c

ulwrite: common.o ulwrite.o ultralight.o
	$(CC) $(CFLAGS) common.o ulwrite.o ultralight.o -o ulwrite $(CLIBS)

ulwrite.o: common.h ultralight.h ulwrite.c
	$(CC) $(CFLAGS) -c ulwrite.c

ultralight.o: structs.h ultralight.h ultralight.c
	$(CC) $(CFLAGS) -c ultralight.c

clean:
	$(RM) *.o
	$(RM) *.stackdump *.bak
	$(RM) ulread ulread.exe
	$(RM) ulwrite ulwrite.exe

