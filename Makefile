# From makefiletutorial

# Using SDL2
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

TARGET_EXE := dango_editor_linux
BUILD_DIR := ./build
SRC_DIRS := ./src

SRC_FILES := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJ_FILES := $(SRC_FILES:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJ_FILES:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP $(SDL_CFLAGS) -g
LDFLAGS := $(SDL_LDFLAGS) -lGL

$(BUILD_DIR)/$(TARGET_EXE): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
