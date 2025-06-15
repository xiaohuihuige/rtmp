OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o,$(wildcard *.c))

.PHONY: all clean

all: $(OBJ) subdir

$(OBJ_DIR)/%.o: %.c
	@echo "CFLAGS from parent: $(CFLAGS)"
	$(CC) $(CFLAGS) -c $< -o $@

subdir:
	$(MAKE) -C ./utils      CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR) CC=$(CC) AR=$(AR)
	$(MAKE) -C ./localfile  CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR) CC=$(CC) AR=$(AR)
#	$(MAKE) -C ./rockchip   CFLAGS="$(CFLAGS)" OBJ_DIR=$(OBJ_DIR) CC=$(CC) AR=$(AR)
