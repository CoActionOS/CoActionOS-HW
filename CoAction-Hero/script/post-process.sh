#!/bin/bash
PROJ=$1
PREFIX=$2
HOST=$3
$PREFIX/$HOST-objcopy -j .text -j .priv_code -j .data -j .vectors -j .startup -O binary ./$PROJ.elf ./$PROJ.bin
$PREFIX/$HOST-objdump -S -j .text -j .priv_code -j .data -j .bss -j .sysmem -d $PROJ.elf > $PROJ.s
$PREFIX/$HOST-size ./$PROJ.elf 
