NDK := /opt/android-ndk
BIN := $(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin
CXX := $(BIN)/aarch64-linux-android26-clang++
CC  := $(BIN)/aarch64-linux-android26-clang

LUA_DIR := libs/Lua54
FFI_DIR := libs/libffi
JSON_DIR := libs/nlohmann
STB_IMAGE_DIR := libs/stb_image
PAYLOAD_DIR := .
BUILD_DIR := ../../build/payload

CPP_SRCS := $(shell find $(PAYLOAD_DIR) -name build -prune -o -name "*.cpp" -print)
LUA_SRCS := $(wildcard $(LUA_DIR)/*.c)

CPP_OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRCS))
LUA_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(LUA_SRCS))

CFLAGS   := -fPIC -O2 -I. -I$(LUA_DIR) -I$(FFI_DIR)/include -I$(PAYLOAD_DIR) -I$(JSON_DIR) -I$(STB_IMAGE_DIR) -MMD -MP
CXXFLAGS := $(CFLAGS) -std=c++17
LDFLAGS  := -shared -llog -landroid -ldl -static-libstdc++ -Wl,--export-dynamic

.PHONY: all clean

all: $(BUILD_DIR) libpayload.so

$(CPP_OBJS): | $(BUILD_DIR)
$(LUA_OBJS): | $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $(sort $(dir $(CPP_OBJS) $(LUA_OBJS)))

libpayload.so: $(LUA_OBJS) $(CPP_OBJS)
	@echo "Linking $@"
	@$(CXX) $(LDFLAGS) -o $@ $^ $(FFI_DIR)/libffi.a
	@cp $@ ../../build/payload/libpayload.so
	@rm libpayload.so

$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) libpayload.so