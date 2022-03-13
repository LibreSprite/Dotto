ifeq ($(OS),Windows_NT)
    CC = /mingw32/bin/i686-w64-mingw32-gcc
    CXX = /mingw32/bin/i686-w64-mingw32-g++
    OBJC = /mingw32/bin/i686-w64-mingw32-g++
    LN = /mingw32/bin/i686-w64-mingw32-g++
    STRIP = strip
    FIND = find src -type f -name
    PKGCONFIG = pkg-config
    DOTTO = dotto.exe

    LN_FLAGS += -lv8 -lv8_libplatform
    CPP_FLAGS += -DSCRIPT_ENGINE_V8
    CPP_FLAGS += -D__WINDOWS__

    LN_FLAGS += -lopengl32
    LN_FLAGS += -lglew32
    LN_FLAGS += -llcms2
    LN_FLAGS += -lole32
    LN_FLAGS += -mconsole

else
    CC = gcc
    CXX = g++
    OBJC = g++
    LN = g++
    STRIP = strip
    FIND = find src -type f -name
    UNAME_S := $(shell uname -s)
    DOTTO = dotto

    ifeq ($(OLDGCC),true)
        LN_FLAGS += -lstdc++fs
    endif

    ifeq ($(UNAME_S),Linux)
	PKGCONFIG = pkg-config
        CCFLAGS += -D LINUX
        BITS := $(shell getconf LONG_BIT)

        ifneq ("$(wildcard /usr/lib/x86_64-linux-gnu/libv8.so)","")
	    CPP_FLAGS += -I/usr/include/nodejs/deps/v8/include
	    LN_FLAGS += -L/usr/lib/x86_64-linux-gnu
	    LN_FLAGS += -lv8 -lv8_libplatform
	    CPP_FLAGS += -DSCRIPT_ENGINE_V8
            ifneq ($(DEBIANELDERLY),true)
	        CPP_FLAGS += -DV8_COMPRESS_POINTERS
            endif
        endif

        ifneq ("$(wildcard /usr/lib/arm-linux-gnueabihf/libv8.so)","")
	    CPP_FLAGS += -I/usr/include/nodejs/deps/v8/include
	    LN_FLAGS += -L/usr/lib/arm-linux-gnueabihf
	    LN_FLAGS += -lv8 -lv8_libplatform
	    CPP_FLAGS += -DSCRIPT_ENGINE_V8
        endif

	LN_FLAGS += -lGL
	LN_FLAGS += -llcms2
	LN_FLAGS += -lX11 -lXi
    endif

    ifeq ($(UNAME_S),Darwin)
	PKGCONFIG = /usr/local/bin/pkg-config
	CPP_FILES += $(shell $(FIND) '*.mm')
        CPP_FLAGS += -DV8_COMPRESS_POINTERS # just assume OSX is 64-bit
	CPP_FLAGS += -DGL_SILENCE_DEPRECATION
	LN_FLAGS += -framework OpenGL
	LN_FLAGS += -framework Foundation
	LN_FLAGS += -llcms2
    endif

    # ifneq ($(filter %86,$(UNAME_P)),)
    # endif

endif

LIB_DIRS += $(shell find libs -type d)
CPP_FLAGS += $(patsubst %,-I%,$(LIB_DIRS))

SRC_DIRS += $(shell find src -type d)
SRC_DIRS += $(LIB_DIRS)
CPP_FLAGS += -Isrc

CPP_FLAGS += -MMD -MP
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags sdl2)

#BEGIN FREETYPE2
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags freetype2)
LN_FLAGS += $(shell $(PKGCONFIG) --libs freetype2)
#END

CPP_FILES += $(shell find src -type f -name '*.cpp')
CPP_FILES += $(shell find libs -type f -name '*.cpp')

C_FLAGS := $(CPP_FLAGS)
C_FILES += $(shell find src -type f -name '*.c')
C_FILES += $(shell find libs -type f -name '*.c')

LN_FLAGS += $(shell $(PKGCONFIG) --libs sdl2)
LN_FLAGS += -lSDL2_image
LN_FLAGS += $(SO_FILES)

#BEGIN V8 SUPPORT
# CPP_FLAGS += -DSCRIPT_ENGINE_V8
# CPP_FLAGS += $(shell $(PKGCONFIG) --cflags v8)
# CPP_FLAGS += $(shell $(PKGCONFIG) --cflags v8_libplatform)
# LN_FLAGS += $(shell $(PKGCONFIG) --libs v8)
# LN_FLAGS += $(shell $(PKGCONFIG) --libs v8_libplatform)
#END

#BEGIN LUA SUPPORT
CPP_FLAGS += -DSCRIPT_ENGINE_LUA
LUAPKG := $(shell for p in lua5.4 lua-5.4 lua54 lua5.3 lua-5.3 lua53 lua5.2 lua-5.2 lua52 lua5.1 lua-5.1 lua51 lua ; do $(PKGCONFIG) --exists $$p && echo $$p && break ; done)
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags $(LUAPKG))
LN_FLAGS += $(shell $(PKGCONFIG) --libs $(LUAPKG))
#END

ODIR = build

CPP_FLAGS += --std=c++17
CPP_FLAGS += -DCMS_NO_REGISTER_KEYWORD

# FLAGS += -m32 # uncomment for 32-bit build
ifeq ($(DEBUG),true)
FLAGS += -Og -g -D_DEBUG -rdynamic # debug build
POSTBUILD =
else
FLAGS += -O3 # release build
POSTBUILD = $(STRIP) $(DOTTO)
endif

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

$(DOTTO): $(OBJ)
	$(LN) $(FLAGS) $^ -o $@ $(LN_FLAGS)
	$(POSTBUILD)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
