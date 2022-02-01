PREFIX		= /usr/local
CFLAGS		= -O2 -std=c89
LDFLAGS     = -lc -lgcc -lm

all: BMPToCDI ExtractADPCM PadFile

# Rules to make executable
BMPToCDI: BMPToCDI.o
	$(CC) $(CFLAGS) -o BMPToCDI $^ $(LDFLAGS)

ExtractADPCM: ExtractADPCM.o
	$(CC) $(CFLAGS) -o ExtractADPCM $^ $(LDFLAGS)

PadFile: PadFile.o
	$(CC) $(CFLAGS) -o PadFile $^ $(LDFLAGS)
	
 : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
install:
	cp BMPToCDI $(PREFIX)/bin/
	cp PadFile $(PREFIX)/bin/
	cp ExtractADPCM $(PREFIX)/bin/
	
clean:
	rm -f BMPToCDI ExtractADPCM PadFile *.o
