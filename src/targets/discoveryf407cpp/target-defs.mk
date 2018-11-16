
# add board to target lists
BL_BOARDS += discoveryf407cpp
FW_BOARDS += discoveryf407cpp
EF_BOARDS += discoveryf407cpp
FT_BOARDS += discoveryf407cpp
SIM_BOARDS += discoveryf407cpp

# add dependencies for top level makefile
ef_discoveryf407cpp_all: fw_discoveryf407cpp_all bl_discoveryf407cpp_all
fw_discoveryf407cpp_all: bl_discoveryf407cpp_all
ft_discoveryf407cpp_all: ef_discoveryf407cpp_all

fw_discoveryf407cpp_program: fw_discoveryf407cpp_all
bl_discoveryf407cpp_program: bl_discoveryf407cpp_all
ef_discoveryf407cpp_program: ef_discoveryf407cpp_all
