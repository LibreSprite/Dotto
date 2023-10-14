LN_FLAGS :=
CPP_FLAGS :=
C_FLAGS :=
LIB_DIRS :=
SRC_DIRS :=

ifeq ($(OS),Windows_NT)
    CC = /mingw32/bin/i686-w64-mingw32-gcc
    CXX = /mingw32/bin/i686-w64-mingw32-g++
    OBJC = /mingw32/bin/i686-w64-mingw32-g++
    LN = /mingw32/bin/i686-w64-mingw32-g++
    STRIP = strip
    FIND = find src -type f -name
    PKGCONFIG = pkg-config
    PROJECT = dirt.exe

    CPP_FLAGS += -D__WINDOWS__

    LN_FLAGS += -lopengl32
    LN_FLAGS += -lglew32

    LN_FLAGS += -lole32
    LN_FLAGS += -lws2_32
    LN_FLAGS += -lcrypt32
    ifeq ($(DEBUG),true)
        LN_FLAGS += -mconsole
    else
        LN_FLAGS += -mwindows
    endif

else
    CC = gcc
    CXX = g++
    OBJC = g++
    LN = g++
    STRIP = strip
    FIND = find src -type f -name
    UNAME_S := $(shell uname -s)
    PROJECT = dirt

    ifeq ($(OLDGCC),true)
        LN_FLAGS += -lstdc++fs
    endif

    ifeq ($(UNAME_S),Linux)
	PKGCONFIG = pkg-config
        CCFLAGS += -D LINUX
        BITS := $(shell getconf LONG_BIT)

        ifeq ($(DEBUG),true)
            FLAGS += -rdynamic # debug build
        endif

	LN_FLAGS += -lGL
	LN_FLAGS += -lX11 -lXi
    endif

    ifeq ($(UNAME_S),Darwin)
	PKGCONFIG := PKG_CONFIG_PATH=$(shell for p in /opt/homebrew/opt/openssl/lib/pkgconfig /usr/local/opt/openssl/lib/pkgconfig ; do [ -d $$p ] && echo $$p && break ; done)
	PKGCONFIG += $(shell which pkg-config || echo "/usr/local/bin/pkg-config")

	CPP_FILES += $(shell $(FIND) '*.mm')
	CPP_FLAGS += -DGL_SILENCE_DEPRECATION
	LN_FLAGS += -framework OpenGL
	LN_FLAGS += -framework Foundation
    endif
endif

# LIB_DIRS += $(shell find dependencies -type d)
CPP_FLAGS += $(patsubst %,-I%,$(LIB_DIRS))

SRC_DIRS += $(shell find src -type d)
SRC_DIRS += $(LIB_DIRS)
CPP_FLAGS += -Isrc

CPP_FLAGS += -MMD -MP

CPP_FLAGS += -DCPPHTTPLIB_OPENSSL_SUPPORT
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags libssl)
LN_FLAGS += $(shell $(PKGCONFIG) --libs libssl)
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags libcrypto)
LN_FLAGS += $(shell $(PKGCONFIG) --libs libcrypto)
# CPP_FLAGS += $(shell $(PKGCONFIG) --cflags libpng)
# LN_FLAGS += $(shell $(PKGCONFIG) --libs libpng)

ifeq ($(BACKEND),SDL1)
    CPP_FLAGS += -DUSE_SDL1
    CPP_FLAGS += $(shell $(PKGCONFIG) --cflags sdl)
    LN_FLAGS += $(shell $(PKGCONFIG) --libs sdl)
else
    CPP_FLAGS += -DUSE_SDL2
    CPP_FLAGS += $(shell $(PKGCONFIG) --cflags sdl2)
    LN_FLAGS += $(shell $(PKGCONFIG) --libs sdl2)
endif

CPP_FLAGS += -DNO_FREETYPE
CPP_FLAGS += -DUSE_STBTTF

CPP_FILES += $(shell find src -type f -name '*.cpp')
# CPP_FILES += $(shell find dependencies -type f -name '*.cpp')

C_FLAGS += $(CPP_FLAGS)
C_FILES += $(shell find src -type f -name '*.c')
# C_FILES += $(shell find dependencies -type f -name '*.c')

LN_FLAGS += $(SO_FILES)

ODIR = build

CPP_FLAGS += --std=c++17

ifeq ($(DEBUG),true)
FLAGS += -O0 -g -D_DEBUG # debug build
POSTBUILD =
else
FLAGS += -Os # release build
POSTBUILD = $(STRIP) $(PROJECT)
endif

ifeq ($(PROFILE),true)
FLAGS += -DUSE_PROFILER
endif

FLAGS += -pthread
# LN_FLAGS += -lpng
LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
OBJ += $(patsubst %,$(ODIR)/%.o,$(C_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	$(info CXX - $<)
	@mkdir -p "$$(dirname "$@")"
	$(CXX) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

$(ODIR)/%.c.o: %.c
	$(info CC   - $<)
	@mkdir -p "$$(dirname "$@")"
	@$(CC) -c $< -o $@ $(FLAGS) $(C_FLAGS)

$(ODIR)/%.mm.o: %.mm
	$(info OBJC - $<)
	@mkdir -p "$$(dirname "$@")"
	@$(OBJC) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

$(PROJECT): $(OBJ)
	$(info Linking $@)
	@$(LN) $^ -o $@ $(FLAGS) $(LN_FLAGS)
	$(POSTBUILD)
	size $(PROJECT)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
