NDK := /opt/android-ndk
BIN := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin
CXX := $(BIN)/aarch64-linux-android26-clang++
CC  := $(BIN)/aarch64-linux-android26-clang

LUA_DIR := libs/Lua54
FFI_DIR := libs/libffi
PAYLOAD_DIR := .
BUILD_DIR := ../../build/payload

CPP_SRCS := $(shell find $(PAYLOAD_DIR) -name build -prune -o -name "*.cpp" -print)
LUA_SRCS := $(wildcard $(LUA_DIR)/*.c)

CPP_OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
LUA_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(LUA_SRCS))

CFLAGS   := -fPIC -O2 -I. -I$(LUA_DIR) -I$(FFI_DIR)/include -I$(PAYLOAD_DIR)
CXXFLAGS := $(CFLAGS) -std=c++17
LDFLAGS  := -shared -llog -landroid -ldl -static-libstdc++ -Wl,--export-dynamic

.PHONY: all clean

all: $(BUILD_DIR) payload.so

$(BUILD_DIR):
	@mkdir -p $(sort $(dir $(CPP_OBJS) $(LUA_OBJS)))

payload.so: $(LUA_OBJS) $(CPP_OBJS)
	@echo "Linking $@"
	@$(CXX) $(LDFLAGS) -o $@ $^ $(FFI_DIR)/libffi.a
	@cp $@ ../../build/payload/payload.so
	@rm payload.so

$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) payload.so