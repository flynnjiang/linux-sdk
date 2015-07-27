
SDK_INSTALL_DIR := $(HN_APP_OUT)/lib

CFLAGS += -I$(HN_SDK_ROOT)/inc

SDK_LIB := $(HN_SDK_ROOT)/libsdk.so
SDK_SRC := $(wildcard */*.c)
SDK_OBJ := $(SDK_SRC:%.c=%.o)

all: prep $(SDK_LIB)

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(SDK_LIB) : $(SDK_OBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $^

prep:

install:
	mkdir -p $(SDK_INSTALL_DIR)
	cp -f $(SDK_LIB) $(SDK_INSTALL_DIR)

clean:
	rm -f $(SDK_OBJ)
	rm -f $(SDK_LIB)
