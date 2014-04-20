
lappend auto_path [pwd]
package require dport

namespace import ::dport::util::crc16_ccitt
namespace import ::dport::util::num2bin
namespace import ::dport::util::bin2num
namespace import ::dport::util::bin2list

#
# reset device.
#
#
proc devreset {} {
    #
    # NOTE 
    #     500 ms timeout on device, so wait a little longer
    reset
    after 800
}

#
# write image to FLASH
#
proc wf {} {
    set image main.bin

    # get length
    set length [file size $image]

    # calculate crc
    set fd [open $image r] 
    fconfigure $fd -translation binary
    set crc [crc16_ccitt 0xffff [bin2list [read $fd]]]
    close $fd

    puts "LENGTH  [format %08X $length]"
    puts "CRC     [format %04X $crc]"

    writeflash 0x8000 [num2bin $length 4][num2bin $crc 2]
    writeimage main.bin 0x9000

    # reset device
    devreset
}

#
# write image to SDRAM
#
proc wr {} {
    set image main.bin

    set RTC_GPREG     0x40024044 
    set SDRAM_OFFSET  0xa0000000

    # write magic number to RTC memory
    writemem $RTC_GPREG [num2bin 0x4444BEEF 4]

    # reset device
    devreset

    # check that we actualy in right point of program
    set bkpt [bin2num [readmem [expr {[readreg PC] - 2}] 2] 0 2]
    puts "bkpt = [format %04X $bkpt]"
    if {$bkpt != 0xbeab} {
        puts stderr "bkpt instruciton mismatch, load to SDRAM failed"
        return
    }

    # write file data to SDRAM
    set fd [open $image r] 
    fconfigure $fd -translation binary
    writemem $SDRAM_OFFSET [read $fd]
    close $fd

#    puts "dump memory"
#
#    set fd [open /dev/shm/mem.dat w]
#    fconfigure $fd -translation binary
#    puts -nonewline $fd [readmem $SDRAM_OFFSET [file size $image]]
#    close $fd
#
#    puts "done"

    # continue execution
    cont
}

#
# print tasks state
#
proc pt {} {
    userf 0
}

#quit ; # exit from program

