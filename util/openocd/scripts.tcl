#gdb_report_data_abort enable
gdb_breakpoint_override hard

proc upload_image {args} {

    set image [lindex $args 0]
    set addr  [lindex $args 1]

    if {![string length $image]} {
        set image main.bin
    }

    reset halt
    echo "========================================================="
    echo "uploading $image please wait\r\n"
    if {[string length $addr] != 0} {
        flash write_image erase $image $addr bin
    } else {
        flash write_image erase $image
    }
    echo "\r\nuploading of $image finished"
    echo "========================================================="
    reset run
    reset run ; # XXX
#    mreset
}

proc uimg {} {
    upload_image main.bin
}

#
#
#
proc mreset {} {
    # softaware reset 
    # AIRCR.SYSRESETREQ
    #

    mww 0xE000ED0C 0x04
}

