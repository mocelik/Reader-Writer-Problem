
# FOLDERS
MAIN_EXE := rwp
SRC_DIR := .
OBJ_DIR := ./obj
INC := -I$(SRC_DIR)

dummy_make_dirs := $(shell mkdir -p $(OBJ_DIR))

# Main source files
SRC_FILES :=  $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
DEPENDS :=  $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.d,$(SRC_FILES))

LDFLAGS := -lpthread
CFLAGS += $(INC) --std=c11
GCC := gcc

all: $(MAIN_EXE)

$(MAIN_EXE): $(OBJ_FILES)
	$(GCC) $(CFLAGS) -o $(MAIN_EXE) $^ $(LDFLAGS)

-include $(DEPENDS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf rwp $(OBJ_DIR)
