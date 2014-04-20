#################################
#
#################################

#
# NOTE
#     vector table offset should be alligned to 128 bytes boundary
#     (bits 0:6 of VTOR not implemented)
#

BLOADER_MAXIMAGE_SIZE = 0x00008000 # maximum size of bootloader image
#
# NOTE
#     SIZE, CRC and IMAGE are on the separate sectors of flash
#
MAIN_SIZE_OFFSET      = 0x00008000 # address where size of main applicaiton are stored
MAIN_CRC_OFFSET       = 0x00008004 # address where CRC of main applicaiton are stored, XXX unused
MAIN_IMAGE_OFFSET     = 0x00009000 # where image with main program are stored
MAIN_LOAD_OFFSET      = 0xA0000000 # where main program should be loaded, SDRAM

#
# TODO draw memory usage map
#
#

