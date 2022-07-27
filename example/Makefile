LOCAL_LDLIBS += -lagora-aws-shadow
LOCAL_LDLIBS += -lagora-iot-solution
LOCAL_LDLIBS += -lagora-mbedtls
LOCAL_LDLIBS += -lagora-opus
LOCAL_LDLIBS += -lrtsa
LOCAL_LDLIBS += -lahpl
LOCAL_LDLIBS += -lagora-cjson
LOCAL_LDLIBS += -lmedia-engine
LOCAL_LDLIBS += -liot-utility
LOCAL_LDLIBS += -lagora-webclient
LOCAL_LDLIBS += -lagora-callkit
LOCAL_LDLIBS += -llicense-active
LOCAL_LDLIBS += -liot-license
LOCAL_LDLIBS += -lrt -lm -lpthread

all:
	gcc test_av_src.c hello_agora_call.c -o hello_agora_call -I../include -L../lib -Wl,--start-group $(LOCAL_LDLIBS) -Wl,--end-group

clean:
	rm -rf hello_agora_call