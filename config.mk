CURRENT_DIR = $(shell pwd)
OUT_DIR     = $(CURRENT_DIR)/out
OBJ_DIR     = $(OUT_DIR)/obj
RTMP_DIR    = $(CURRENT_DIR)/src/rtmp
MEDIA_DIR   = $(CURRENT_DIR)/src/media
BIN_DIR     = $(OUT_DIR)/bin
INCLUDE_DIR = $(OUT_DIR)/include
LIB_DIR     = $(OUT_DIR)/lib
SRC_DIR     = $(CURRENT_DIR)/src

STATIC_NAME  = $(LIB_DIR)/librtmp.a
DYNAMIC_NAME = $(LIB_DIR)/librtmp.so