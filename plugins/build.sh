#!/bin/bash

set -e

cd "$(dirname -- "${BASH_SOURCE[0]}")"
VM_HEAP_SIZE=1024

CPP_FLAGS="-DVM_HEAP_SIZE=$VM_HEAP_SIZE --std=c++17 -Os -fno-rtti -Wno-pointer-arith -c -fno-exceptions -fno-builtin -ffunction-sections -fdata-sections -funsigned-char -MMD -fno-delete-null-pointer-checks -fomit-frame-pointer -mcpu=cortex-m0plus -mthumb -Wno-psabi -Wno-conversion-null -Wno-narrowing -Wno-write-strings"

LD_FLAGS="-Os -Wl,--gc-sections -Wl,-n --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -Wl,--no-warn-rwx-segments -mthumb -T ../link.ld -Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys -Wl,--end-group"

CPP_FLAGS=($CPP_FLAGS)
LD_FLAGS=($LD_FLAGS)

arm-none-eabi-g++ init.cpp "${CPP_FLAGS[@]}" -o init.o

for PAYLOAD in */ ; do
    PROJECT="$(basename "$PAYLOAD")"
    pushd $PROJECT > /dev/null

    touch compiling.o
    touch compiling.drt
    touch compiling.elf
    rm *.o *.elf *.drt

    echo "compiling $PROJECT"
    for SRC in *.cpp ; do
        arm-none-eabi-g++ $SRC "${CPP_FLAGS[@]}" -o $SRC.o
    done

    arm-none-eabi-g++ *.o ../init.o "${LD_FLAGS[@]}" --output $PROJECT.elf

    arm-none-eabi-objcopy -O binary $PROJECT.elf $PROJECT.drt
    arm-none-eabi-objdump -lSd $PROJECT.elf > $PROJECT.s
    size $PROJECT.elf

    popd > /dev/null
done

cd ..

make -k -j4 #DEBUG=true
./gate 42
