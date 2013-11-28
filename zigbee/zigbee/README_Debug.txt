BITCLOUD
========

Debug Parameter einstellen in folgenden Dateien:

BitCloud\Components\HAL\Configuration 




AT91SAM7X
=========

Debug-Vorbereitungen mit OpenOCD


Ggf. NVM-bit 2 setzen

Vorsicht: Wenn der ERASE-Pin des SAM7X gebraucht wurde, muss man erst das NVM-Bit 2 setzen. Ansonsten startet der SAM7X immer im Bootloadermodus und nie aus dem FLASH.

Befehl:

> at91sam7 gpnvm 2 set


Lock-Bits entfernen

> flash protect 0 0 15 off


Flash Info muss wie folgt aussehen:

> flash info 0
#0 : at91sam7 at 0x00100000, size 0x00040000, buswidth 4, chipwidth 0
        #  0: 0x00000000 (0x4000 16kB) not protected
        #  1: 0x00004000 (0x4000 16kB) not protected
        #  2: 0x00008000 (0x4000 16kB) not protected
        #  3: 0x0000c000 (0x4000 16kB) not protected
        #  4: 0x00010000 (0x4000 16kB) not protected
        #  5: 0x00014000 (0x4000 16kB) not protected
        #  6: 0x00018000 (0x4000 16kB) not protected
        #  7: 0x0001c000 (0x4000 16kB) not protected
        #  8: 0x00020000 (0x4000 16kB) not protected
        #  9: 0x00024000 (0x4000 16kB) not protected
        # 10: 0x00028000 (0x4000 16kB) not protected
        # 11: 0x0002c000 (0x4000 16kB) not protected
        # 12: 0x00030000 (0x4000 16kB) not protected
        # 13: 0x00034000 (0x4000 16kB) not protected
        # 14: 0x00038000 (0x4000 16kB) not protected
        # 15: 0x0003c000 (0x4000 16kB) not protected

 at91sam7 driver information: Chip is AT91SAM7X256
 Cidr: 0x275b0940 | Arch: 0x0075 | Eproc: ARM7TDMI | Version: 0x000 | Flashsize: 0x00040000
 Master clock (estimated): 47923 KHz | External clock: 18432 KHz
 Pagesize: 256 bytes | Lockbits(16): 0 0x0000 | Pages in lock region: 0
 Securitybit: 0 | Nvmbits(3): 1 0x4


 
 ----------------------
 
arm7_9 dbgrq enable
gdb_breakpoint_override hard 


