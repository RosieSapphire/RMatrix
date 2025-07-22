CC        := gcc
CFLAGS    := -Wall -Wextra -Werror -ansi -pedantic -O3
RMATRIX   := rmatrix
BUILD_DIR := build
SRC_DIRS  := src
RP_INC    := -Ilib/rosepetal
RP_LINK   := -Llib/rosepetal -lrosepetal
C_FILES   := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
O_FILES   := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

all: $(RMATRIX)

$(RMATRIX): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(RP_LINK)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $< $(RP_INC)

.PHONY: clean

clean:
	rm -rf $(RMATRIX) $(BUILD_DIR)
