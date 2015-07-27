
CFLAGS += -lpthread -I../../inc
SDK_LIB := ../../out/libsdk.so


all: logtest

logtest : log.o $(SDK_LIB)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f logtest
	rm -f *.o
	rm -f *.log
