CXX = g++
LN = g++
ODIR = build

SRC_DIRS = $(shell find src -type d)

CPP_FLAGS += --std=c++17
CPP_FLAGS += -MMD -MP
CPP_FLAGS += $(patsubst %,-I%,$(SRC_DIRS))
CPP_FLAGS += $(shell sdl2-config --cflags)

CPP_FILES := $(shell find src -type f -name '*.cpp')

# FLAGS += -m32 # uncomment for 32-bit build
FLAGS += -Og -g -D_DEBUG # debug build
# FLAGS += -O3 # release build

LN_FLAGS += $(shell sdl2-config --libs)
LN_FLAGS += -lGL
LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

libresprite: $(OBJ)
	$(LN) $(FLAGS) $(LN_FLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
