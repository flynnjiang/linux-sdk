
CFLAGS += -lpthread -I../../inc
SDK_LIB := ../../out/libsdk.so


all: nvtest

nvtest : nv.o $(SDK_LIB)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f nvtest
	rm -f *.o
	rm -f *.log
