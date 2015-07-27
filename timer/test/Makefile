
CFLAGS += -lpthread -I../../inc
SDK_LIB := ../../out/libsdk.so


all: timer

timer : timer.o $(SDK_LIB)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f timer
	rm -f *.o
	rm -f *.log
