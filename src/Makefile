OBJ_FILE = $(wildcard $(OBJ_DIR)/*.o)

.PHONY: all subdir static_lib dynamic_lib

all: subdir static_lib dynamic_lib 

static_lib:
	$(AR) rcs $(STATIC_NAME) $(OBJ_FILE)

dynamic_lib:
	$(CC) -shared -o $(DYNAMIC_NAME) $(OBJ_FILE)

subdir:
	$(MAKE) -C $(RTMP_DIR)  CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR)
	$(MAKE) -C $(MEDIA_DIR) CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR)

