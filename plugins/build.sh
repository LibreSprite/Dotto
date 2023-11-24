#!/bin/bash

set -e

cd "$(dirname -- "${BASH_SOURCE[0]}")"
PLUGINS_DIR="$(pwd)"
VM_HEAP_SIZE=1024

function ld_supports_flag() {
    echo "" | arm-none-eabi-g++ -Wl,--unresolved-symbols=ignore-all -Wl,$1 -o /dev/null -x c - 2> /dev/null
}

if ld_supports_flag --no-warn-rwx-segments ; then
    LD_OPT_FLAGS="-Wl,--no-warn-rwx-segments"
fi

CPP_FLAGS="-I../../ports/include --std=c++17 -Os -fno-rtti -Wno-pointer-arith -c -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -MMD -fno-delete-null-pointer-checks -fomit-frame-pointer -mcpu=cortex-m0plus -mthumb -Wno-psabi -Wno-conversion-null -Wno-narrowing -Wno-write-strings"

LD_FLAGS="-Wl,--gc-sections -Wl,-n --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus $LD_OPT_FLAGS -mthumb -T ../link.ld -Wl,--start-group -L../../ports/lib -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -Wl,--end-group"

CPP_FLAGS=($CPP_FLAGS)
LD_FLAGS=($LD_FLAGS)

arm-none-eabi-g++ init.cpp -I../ports/include -DVM_HEAP_SIZE=$VM_HEAP_SIZE "${CPP_FLAGS[@]}" -o init.o

for PAYLOAD in */ ; do
    PROJECT="$(basename "$PAYLOAD")"
    pushd $PROJECT > /dev/null

    touch compiling.o
    touch compiling.drt
    touch compiling.elf
    rm *.o *.elf *.drt

    echo "compiling $PROJECT"

    if [ -f "compile_flags.txt" ] ; then
        LOCAL_CPP_FLAGS="${CPP_FLAGS[@]} $(< compile_flags.txt)"
    else
        LOCAL_CPP_FLAGS="${CPP_FLAGS[@]}"
    fi
    LOCAL_CPP_FLAGS=($LOCAL_CPP_FLAGS)

    if [ -f "link_flags.txt" ] ; then
        LOCAL_LD_FLAGS="${LD_FLAGS[@]} $(< link_flags.txt)"
    else
        LOCAL_LD_FLAGS="${LD_FLAGS[@]}"
    fi
    LOCAL_LD_FLAGS=($LOCAL_LD_FLAGS)

    for SRC in *.cpp ; do
        echo CXX $SRC "${LOCAL_CPP_FLAGS[@]}" -o $SRC.o
        arm-none-eabi-g++ $SRC "${LOCAL_CPP_FLAGS[@]}" -o $SRC.o
    done

    echo LD *.o ../init.o "${LOCAL_LD_FLAGS[@]}" --output $PROJECT.elf
    arm-none-eabi-g++ *.o ../init.o "${LOCAL_LD_FLAGS[@]}" --output $PROJECT.elf

    arm-none-eabi-objcopy -O binary $PROJECT.elf $PROJECT.drt
    arm-none-eabi-objdump -lSd $PROJECT.elf > $PROJECT.s
    arm-none-eabi-size $PROJECT.elf

    mkdir -p ../../data/plugins/$PROJECT/

    cp $PROJECT.drt ../../data/plugins/$PROJECT/
    if [ -f settings.ini ] ; then
        cp settings.ini ../../data/plugins/$PROJECT/
    fi

    popd > /dev/null
done

cd ..

make -k -j4 #DEBUG=true
# ./dirt 42
