LOCAL_LDLIBS += -lagora-iot-sdk
LOCAL_LDLIBS += -lrt -lm -lpthread

all:
	gcc src/hello_doorbell.c src/hello_doorbell_comm.c -o hello_link -I../include -I./inc -L../lib -Wl,--start-group $(LOCAL_LDLIBS) -Wl,--end-group

clean:
	rm -rf hello_link
