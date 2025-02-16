LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ET-T3_demo.elf

LOCAL_CPPFLAGS := -std=c++17

# 隐藏符号 #
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CPPFLAGS += -fvisibility=hidden

LOCAL_SRC_FILES += main.cpp

LOCAL_CPPFLAGS += -fexceptions -frtti

include $(BUILD_EXECUTABLE)