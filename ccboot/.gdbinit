target remote localhost:1234
file ./build/ccboot.elf
layout src
layout regs
break boot1_cmain
continue
