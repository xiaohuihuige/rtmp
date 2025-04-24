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

OBJ_FILE = $(wildcard $(OBJ_DIR)/*.o)

EXECUTABLES = $(patsubst example/%.c, $(BIN_DIR)/%, $(wildcard example/*.c))

CFLAGS = -I$(RTMP_DIR)  -I$(MEDIA_DIR) -lschedule  -lpthread -lrt -Wall -fPIC -g 

.PHONY: all clean subdir TEMP_PATH

all: TEMP_PATH subdir $(EXECUTABLES)

TEMP_PATH:
ifeq ("$(wildcard $(OUT_DIR))","")
	mkdir $(OUT_DIR)
endif
ifeq ("$(wildcard $(OBJ_DIR))","")
	mkdir $(OBJ_DIR)
endif
ifeq ("$(wildcard $(BIN_DIR))","")
	mkdir $(BIN_DIR)
endif
ifeq ("$(wildcard $(INCLUDE_DIR))","")
	mkdir $(INCLUDE_DIR)
endif
ifeq ("$(wildcard $(LIB_DIR))","")
	mkdir $(LIB_DIR)
endif

$(BIN_DIR)/%: example/%.c | TEMP_PATH
	$(CC) -o $@ $^ $(OBJ_FILE) $(CFLAGS)

subdir:
	$(MAKE) -C $(SRC_DIR) CFLAGS="$(CFLAGS)" \
		OBJ_DIR=$(OBJ_DIR) RTMP_DIR=$(RTMP_DIR)  MEDIA_DIR=$(MEDIA_DIR)\
		STATIC_NAME=$(STATIC_NAME) DYNAMIC_NAME=$(DYNAMIC_NAME) 
	find $(SRC_DIR) -name "*.h" -exec cp {} $(INCLUDE_DIR) \;

clean:
	-$(RM) -rf $(OUT_DIR) 
	
install:
	-cp $(DYNAMIC_NAME)   /usr/local/lib/librtmp.so
	-cp $(BIN_DIR)/*      /usr/local/bin/
	-cp $(INCLUDE_DIR)/*  /usr/local/include
	-ldconfig



