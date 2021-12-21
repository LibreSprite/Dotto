ifeq ($(OS),Windows_NT)
    # CCFLAGS += -D WIN32
    # ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    #     CCFLAGS += -D AMD64
    # else
    #     ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    #         CCFLAGS += -D AMD64
    #     endif
    #     ifeq ($(PROCESSOR_ARCHITECTURE),x86)
    #         CCFLAGS += -D IA32
    #     endif
    # endif
else
    CXX = g++
    OBJC = g++
    LN = g++
    UNAME_S := $(shell uname -s)

    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
	CPP_FILES += $(shell find src -type f -name '*.mm')
    endif

    # UNAME_P := $(shell uname -p)
    # ifeq ($(UNAME_P),x86_64)
    #     CCFLAGS += -D AMD64
    # endif
    # ifneq ($(filter %86,$(UNAME_P)),)
    #     CCFLAGS += -D IA32
    # endif
    # ifneq ($(filter arm%,$(UNAME_P)),)
    #     CCFLAGS += -D ARM
    # endif

    SRC_DIRS += $(shell find src -type d)
    SRC_DIRS += $(shell find libs -type d)

    CPP_FLAGS += $(patsubst %,-I%,$(SRC_DIRS))
    CPP_FLAGS += $(shell sdl2-config --cflags)
    CPP_FLAGS += $(shell pkg-config --cflags freetype2)

    CPP_FILES += $(shell find src -type f -name '*.cpp')
    CPP_FILES += $(shell find libs -type f -name '*.cpp')

    LN_FLAGS += $(shell sdl2-config --libs)
    LN_FLAGS += $(shell pkg-config --libs freetype2)
endif

ODIR = build

CPP_FLAGS += --std=c++17
CPP_FLAGS += -MMD -MP

# FLAGS += -m32 # uncomment for 32-bit build
FLAGS += -Og -g -D_DEBUG # debug build
# FLAGS += -O3 # release build


LN_FLAGS += -lGL
LN_FLAGS += -lpng
LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

$(ODIR)/%.mm.o: %.mm
	@mkdir -p "$$(dirname "$@")"
	$(OBJC) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

libresprite: $(OBJ)
	$(LN) $(FLAGS) $^ -o $@ $(LN_FLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
