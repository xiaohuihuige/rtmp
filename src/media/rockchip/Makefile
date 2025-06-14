
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o,$(wildcard *.c))

.PHONY: all clean

all: $(OBJ)

$(OBJ_DIR)/%.o: %.c
	@echo "CFLAGS from parent: $(CFLAGS)"
	$(CC) $(CFLAGS) -c $< -o $@ 