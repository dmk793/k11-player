#!/bin/sh
#\
exec tclsh "$0" "$@"

#
# convert binary string to list of hex numbers
#
proc bin2list {bin} {
    set ret [list]

    # XXX will this function work if UTF encoding is used?
    while {[string length $bin]} {
        if {[binary scan $bin H2 byte]} {
            lappend ret 0x$byte
            set bin [string range $bin 1 end]
        } else {
            tk_messageBox -message "can't convert byte" -type ok -icon error
            exit 1
        }
    }

    return $ret
}

#
# convert list of hexnumbers (LSB first) to one big number
#
proc list2num {data} {
    set ret 0
    set idx 0

    if {[llength $data] == 0} {
        tk_messageBox -message "[dict get [info frame 0] proc]: empty list specified" -type ok -icon error
    }

    foreach byte $data {
        set ret [expr {$ret | ($byte << (8 * $idx))}]
        incr idx
    }

    return 0x[format %02X $ret]
}

#
# conver one big number to list (LSB - first element of list) 
#
proc num2list {num size} {
    set ret [list]
    for {set i 0} {$i < $size} {incr i} {
        lappend ret 0x[format %02x [expr {($num >> (8*$i)) & 0xff}]]
    }
    return $ret
}

#================================================
#
#

set fd [open [lindex $argv 0] r+]
fconfigure $fd -translation binary

set crc 0
foreach {b0 b1 b2 b3} [bin2list [read $fd [expr {4 * 7}]]] {
    incr crc [expr {$b0 | ($b1 << 8) | ($b2 << 16) | ($b3 << 24)}]
}

set crc [num2list [expr {0xffffffff - ($crc & 0xffffffff) + 1}] 4]
puts $crc

foreach byte $crc {
    append value [binary format c $byte]
}

seek $fd 0x1c start
puts -nonewline $fd $value

close $fd

