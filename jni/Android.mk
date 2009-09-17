LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_WAVE_SRC := muslib/wave/i_BD909.c muslib/wave/i_HANDCLAP.c muslib/wave/i_HO909.c \
	muslib/wave/i_SDGATE.c muslib/wave/i_TOML1.c muslib/wave/i_CYMBD.c \
	muslib/wave/i_HC909.c muslib/wave/i_SD909.c muslib/wave/i_TOMH1.c muslib/wave/i_TOMM1.c
MY_MUSLIB_SRC := muslib/exp12.c muslib/instdef.c muslib/mus.c muslib/seq.c muslib/wavetable.c

LOCAL_MODULE    := muslib
LOCAL_SRC_FILES := jni.c ifmuslib.c $(MY_WAVE_SRC) $(MY_MUSLIB_SRC)
LOCAL_CFLAGS    += -I$(LOCAL_PATH)/muslib -I.. 

include $(BUILD_SHARED_LIBRARY)
