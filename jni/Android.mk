LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := ru_tts

LOCAL_SRC_FILES := ../text2speech.c ../synth.c \
	 ../utterance.c ../speechrate_control.c ../time_planner.c \
	 ../numerics.c ../intonator.c ../transcription.c \
	 ../male.c ../female.c ../sink.c ../soundproducer.c

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := ruvoicesynth

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_SRC_FILES := ru_tts_jni.cpp

LOCAL_WHOLE_STATIC_LIBRARIES := ru_tts

include $(BUILD_SHARED_LIBRARY)
