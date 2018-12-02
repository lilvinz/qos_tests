
# add board to target lists
BL_BOARDS += discoveryf072
FW_BOARDS += discoveryf072
EF_BOARDS += discoveryf072
FT_BOARDS += discoveryf072
SIM_BOARDS += discoveryf072

# add dependencies for top level makefile
ef_discoveryf072_all: fw_discoveryf072_all bl_discoveryf072_all
fw_discoveryf072_all: bl_discoveryf072_all
ft_discoveryf072_all: ef_discoveryf072_all

fw_discoveryf072_program: fw_discoveryf072_all
bl_discoveryf072_program: bl_discoveryf072_all
ef_discoveryf072_program: ef_discoveryf072_all
