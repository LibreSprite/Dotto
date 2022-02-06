ifeq ($(OS),Windows_NT)
    # CCFLAGS += -D WIN32
    # ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        # CPP_FLAGS += -DV8_COMPRESS_POINTERS
    # else
    #     ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    #     endif
    #     ifeq ($(PROCESSOR_ARCHITECTURE),x86)
    #     endif
    # endif
else
    CC = gcc
    CXX = g++
    OBJC = g++
    LN = g++
    UNAME_S := $(shell uname -s)

    # uncomment if you get a linker error due to GCC 8
    # LN_FLAGS += -lstdc++fs


    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
        BITS := $(shell getconf LONG_BIT)
        ifeq ($(BITS),64)
            CPP_FLAGS += -DV8_COMPRESS_POINTERS
	    # CPP_FLAGS += -DSCRIPT_ENGINE_V8
	    # LIB_DIRS := $(shell find linux-x64 -type d)
	    # SO_FILES := $(shell find linux-x64 -type f -name '*.so*')
        endif

        ifneq ("$(wildcard /usr/lib/arm-linux-gnueabihf/libv8.so)","")
	    CPP_FLAGS += -I/usr/include/nodejs/deps/v8/include
	    LN_FLAGS += -L/usr/lib/arm-linux-gnueabihf
	    LN_FLAGS += -lv8 -lv8_libplatform
	    CPP_FLAGS += -DSCRIPT_ENGINE_V8
        endif

	LN_FLAGS += -lGL
	LN_FLAGS += -llcms2
    endif

    ifeq ($(UNAME_S),Darwin)
	CPP_FILES += $(shell find src -type f -name '*.mm')
        CPP_FLAGS += -DV8_COMPRESS_POINTERS # just assume OSX is 64-bit
	LN_FLAGS += -framework OpenGL
	LN_FLAGS += -framework Foundation
    endif

    # ifneq ($(filter %86,$(UNAME_P)),)
    # endif

    LIB_DIRS += $(shell find libs -type d)
    CPP_FLAGS += $(patsubst %,-I%,$(LIB_DIRS))

    SRC_DIRS += $(shell find src -type d)
    SRC_DIRS += $(LIB_DIRS)
    CPP_FLAGS += -Isrc

    CPP_FLAGS += -MMD -MP
    CPP_FLAGS += $(shell sdl2-config --cflags)

#BEGIN FREETYPE2
    CPP_FLAGS += $(shell pkg-config --cflags freetype2)
    LN_FLAGS += $(shell pkg-config --libs freetype2)
#END

    CPP_FILES += $(shell find src -type f -name '*.cpp')
    CPP_FILES += $(shell find libs -type f -name '*.cpp')

    C_FLAGS := $(CPP_FLAGS)
    C_FILES += $(shell find src -type f -name '*.c')
    C_FILES += $(shell find libs -type f -name '*.c')

    LN_FLAGS += $(shell sdl2-config --libs)
    LN_FLAGS += -lSDL2_image
    LN_FLAGS += $(SO_FILES)

#BEGIN V8 SUPPORT
    # CPP_FLAGS += -DSCRIPT_ENGINE_V8
    # CPP_FLAGS += $(shell pkg-config --cflags v8)
    # CPP_FLAGS += $(shell pkg-config --cflags v8_libplatform)
    # LN_FLAGS += $(shell pkg-config --libs v8)
    # LN_FLAGS += $(shell pkg-config --libs v8_libplatform)
#END

#BEGIN LUA SUPPORT
    CPP_FLAGS += -DSCRIPT_ENGINE_LUA
    LUAPKG := $(shell for p in lua5.4 lua-5.4 lua54 lua5.3 lua-5.3 lua53 lua5.2 lua-5.2 lua52 lua5.1 lua-5.1 lua51 lua ; do pkg-config --exists $$p && echo $$p && break ; done)
    CPP_FLAGS += $(shell pkg-config --cflags $(LUAPKG))
    LN_FLAGS += $(shell pkg-config --libs $(LUAPKG))
#END
endif

ODIR = build

CPP_FLAGS += --std=c++17

# FLAGS += -m32 # uncomment for 32-bit build
FLAGS += -Og -g -D_DEBUG # debug build
# FLAGS += -O3 # release build

LN_FLAGS += -lpng
LN_FLAGS += -lm
# LN_FLAGS += -fsanitize=leak

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
OBJ += $(patsubst %,$(ODIR)/%.o,$(C_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

$(ODIR)/%.c.o: %.c
	@mkdir -p "$$(dirname "$@")"
	$(CC) $(FLAGS) $(C_FLAGS) -c $< -o $@

$(ODIR)/%.mm.o: %.mm
	@mkdir -p "$$(dirname "$@")"
	$(OBJC) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

dotto: $(OBJ)
	$(LN) $(FLAGS) $^ -o $@ $(LN_FLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
