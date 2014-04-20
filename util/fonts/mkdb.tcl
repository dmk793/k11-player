#!/bin/sh
#\
exec tclsh "$0" "$@"

#
# Raster font converter.
# PBM image template to binary data.
#
# Input template should contatin symbols table. Vertical
# size of character should be multiple of 8.
#
# For auto detect of character dimensions first character in input file
# shoud be in form like that (example is 9x16 character):
#
#    111111110
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    1xxxxxxxx
#    0xxxxxxxx
#
# x - don't care
#
#
# Outpu binary format
# ===================
#
# 7x8 character layout:
#
#         byte1
#           |    +--byte6
#   byte0--+|    |
#   |      ||    |
#   |      0000000
#   |      1111111
#   |      2222222
#   |      3333333
#   |      4444444
#   |      5555555
#   |      6666666
#   +------7777777
#
#     
# 9x16 character layout:
#
#        byte2
#           |
#   byte0--+|
#   |      ||
#   |      000000000
#   |      111111111
#   |      222222222
#   |      333333333
#   |      444444444
#   |      555555555
#   |      666666666
#   +------777777777
#    +-----000000000
#    |     111111111
#    |     222222222
#    |     333333333
#    |     444444444
#    |     555555555
#    |     666666666
#    |     777777777
#    |     ||      |
#   byte1--+|      +---byte17
#   byte3---+
#
# 

if {$argc < 2} {
    puts "Usage: "
    puts "   [file tail [info script]] \[outfmt\] \[infile\]"
    puts " "
    puts "outfmt: C, ASM or BIN"
    puts "   "

    exit 0
}

set outfmt  [lindex $argv 0]
set infile  [lindex $argv 1]

switch $outfmt {
    C -
    ASM -
    BIN {}
    default {
        puts "(E) Unknown output format \"$outfmt\""
    }
}

# open input file
if {[catch {set in  [open $infile  r]} result]} {puts stderr $result; exit 1}
fconfigure $in  -translation binary


# read PBM format string, should be P1
if {[string trim [gets $in]] ne "P1"} {
    puts stderr "(E) Invalid format of input file"
}

# get input file dimensions
while {[gets $in line] >= 0} {
    set line [string trim $line]
    # skip comment
    if {[string index $line 0] eq "#"} {
        continue
    }
    lassign [split $line { }] XRES YRES
    break
}

# get lines, remove spaces
while {[gets $in line] >= 0} {
    set line [string trim $line]
    # skip comment
    if {[string index $line 0] eq "#"} {continue}

    # remove unnecessary space characters
    set symb [list]
    foreach s [split $line {}] {
        if {[string length [string trim $s]]} {
            lappend symb $s
        }
    }

    lappend LINES [join $symb {}]
}

# get character dimension from first character
set CH_WIDTH  0
set CH_HEIGHT 0
# horizontal 
for {set i 0} {$i < [string length [lindex $LINES 0]]} {incr i} {
    incr CH_WIDTH
    if {[string index [lindex $LINES 0] $i] eq "0"} {
        break
    }
}
# vertical
for {set i 0} {$i < [llength $LINES]} {incr i} {
    incr CH_HEIGHT
    if {[string index [lindex $LINES $i] 0] eq "0"} {
        break
    }
}


#
# We have here following variables:
#
# XRES        horizontal resolution of input PBM file
# YRES        vertical resolution of input PBM file
# CH_WIDTH    character width in pixels
# CH_HEIGHT   character height in pixels
# LINES       list of lines of input file without space characters
#

puts stderr "XRES       $XRES"
puts stderr "YRES       $YRES"
puts stderr "CH_WIDTH   $CH_WIDTH"
puts stderr "CH_HEIGHT  $CH_HEIGHT"

# sanity check of result dimensions
if {($XRES % $CH_WIDTH) != 0} {
    puts stderr "(E) Horizontal dimensions mismatch, XRES should be multiple of CH_WIDTH"
}
if {($YRES % $CH_HEIGHT) != 0} {
    puts stderr "(E) Vertical dimensions mismatch, YRES should be multiple of CH_HEIGHT"
}
if {($CH_HEIGHT % 8) != 0} {
    puts stderr "(E) Invalid character height, CH_HEIGHT should be multiple of 8"
}

# loop thru vertical character indexes
# TODO throw error if not enough data available in input file
set BYTES [list]
set lineidx 0
for {set vidx 0} {$vidx < [expr {$YRES / $CH_HEIGHT}]} {incr vidx} {
    # loop thru horizontal character indexes
    set stringidx 0
    for {set hidx 0} {$hidx < [expr {$XRES / $CH_WIDTH}]} {incr hidx} {
        # loop thru horizontal character data
        for {set x 0} {$x < $CH_WIDTH} {incr x} {
            set sidx [expr {$stringidx + $x}] ; # index in string
            # loop thru vertical character data
            for {set y 0} {$y < ($CH_HEIGHT / 8)} {incr y} {
                set byte 0
                for {set i 0; set mask 0x01} {$mask < 0x100} {incr i; set mask [expr {$mask << 1}]} {
                    set lidx [expr {$lineidx + $i + $y * 8}]   ; # index of line
                    if {[string index [lindex $LINES $lidx] $sidx] eq "1"} {
                        set byte [expr {$byte | $mask}]
                    }
                }
                lappend BYTES 0x[format %02X $byte]
            }
        }
        incr stringidx $CH_WIDTH
    }
    incr lineidx $CH_HEIGHT
}

# output data in corresponding format
switch $outfmt {
    C {
        puts -nonewline "const char data\[[llength $BYTES]\] = {"
        set i 0
        foreach byte $BYTES {
            if {($i % 16) == 0} {
                puts {}
                puts -nonewline [join [list {    } /* [format %04X $i] */ {    }] {}]
            }
            incr i
            puts -nonewline "$byte, "
        }
        puts {}
        puts "};"
    }
    ASM {
        puts -nonewline "data:"
        set i 0
        foreach byte $BYTES {
            if {($i % 16) == 0} {
                puts {}
                puts -nonewline {    db }
            }
            incr i
            puts -nonewline $byte
            if {($i % 16) != 0} {
                puts -nonewline ", "
            }
        }
        puts {}
    }
    BIN {
        # configure stdout to produce binary data
        fconfigure stdout -translation binary
        foreach byte $BYTES {
            puts -nonewline stdout [binary format c $byte]
        }
    }
    default {
        puts "(E) Unknown output format \"$outfmt\""
    }
}

