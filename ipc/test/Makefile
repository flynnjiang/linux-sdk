
CFLAGS += -lpthread -I../../inc
SDK_LIB := ../../out/libsdk.so


all: ma mb

ma : module_a.o $(SDK_LIB)
	$(CC) -o $@ $^ $(CFLAGS)

mb : module_b.o $(SDK_LIB)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f ma mb
	rm -f *.o
	rm -f *.log
