
# internal flash
BL_ORIGIN           := 0x08000000 # @    0 KiB
BL_SIZE             := 0x00010000 #     32 KiB (2 sectors of  16 KiB)
EE_ORIGIN           := 0x08008000 # @   32 KiB
EE_SIZE             := 0x00008000 #     32 KiB (2 sectors of  16 KiB)
# 1 sector of 64 KiB is unused
FW_ORIGIN           := 0x08020000 # @  128 KiB
FW_SIZE             := 0x00060000 #    384 KiB (3 sectors of 128 KiB)
# note: the remaining space is being unused for now
EF_ORIGIN           := 0x08000000 # @    0 KiB
EF_SIZE             := 0x00080000 #    512 KiB

OPENOCD_JTAG_CONFIG := stlink-v2.cfg
OPENOCD_TRANSPORT   := hla_swd
OPENOCD_CONFIG      := stm32f4x.cfg
