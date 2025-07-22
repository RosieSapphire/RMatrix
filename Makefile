CC        := gcc
CFLAGS    := -Wall -Wextra -Werror -ansi -pedantic -O3
RMATRIX   := rmatrix
BUILD_DIR := build
SRC_DIRS  := src
RP_DIR    := lib/rosepetal
RP_INC    := -I$(RP_DIR)/include
RP_LINK   := -L$(RP_DIR) -lrosepetal
RP_SLIB   := librosepetal.a
C_FILES   := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
O_FILES   := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

all: $(RP_SLIB) $(RMATRIX)

$(RP_SLIB):
	make lib -C lib/rosepetal -j

$(RMATRIX): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(RP_LINK)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $< $(RP_INC)

.PHONY: clean

clean:
	rm -rf $(RMATRIX) $(BUILD_DIR)
	make clean -C lib/rosepetal
