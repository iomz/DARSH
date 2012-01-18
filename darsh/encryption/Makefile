CC=gcc
CFLAGS= -g -I/use/include
LIBS= -lcrypto -lssl

all: rsautl
rsautl: rsautl.o
	$(CC) $(CFLAGS) -o rsautl rsautl.o $(LIBS)
clean:
	rm -f rsautl rsautl.o *pem