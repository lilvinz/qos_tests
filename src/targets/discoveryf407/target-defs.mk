
# add board to target lists
BL_BOARDS += discoveryf407
FW_BOARDS += discoveryf407
EF_BOARDS += discoveryf407
FT_BOARDS += discoveryf407
SIM_BOARDS += discoveryf407

# add dependencies for top level makefile
ef_discoveryf407_all: fw_discoveryf407_all bl_discoveryf407_all
fw_discoveryf407_all: bl_discoveryf407_all
ft_discoveryf407_all: ef_discoveryf407_all

fw_discoveryf407_program: fw_discoveryf407_all
bl_discoveryf407_program: bl_discoveryf407_all
ef_discoveryf407_program: ef_discoveryf407_all
