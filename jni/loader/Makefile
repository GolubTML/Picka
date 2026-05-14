NDK := /opt/android-ndk
BIN := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin
CC  := $(BIN)/aarch64-linux-android26-clang

SRCS := main.c
BUILD_DIR := ../../build/libloader
TARGET := ../../build/libloader/libloader.so

CFLAGS := -shared -fPIC -O2 -I.
LDFLAGS := -shared -llog

OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	@echo "Linking $@"
	@$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling loader: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)