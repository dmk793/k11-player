#!/bin/sh
#\
exec tclsh "$0" "$@"

#
# for each directory run command
#

set DIRS [lrange $argv 0 end-1]
set TARGET [lindex $argv end]

fconfigure stdout -buffering none
fconfigure stdin -buffering none

if {[info exists env(PAUSE)]} {
    puts "PAUSE=$env(PAUSE)"
    set pause $env(PAUSE)
} else {
    set pause 0
}

foreach dir $DIRS {
    set cdir [pwd]
    cd $dir

    if {[catch {
            set fd [open "| make $TARGET 2>@1"]
            while {[gets $fd line] >= 0} {
                puts $line
            }
            close $fd
         } result]} {
        puts $result
        exit 1
    } else {

        if {$pause} {
            puts {}
            puts "target \"$TARGET\" for dir \"$dir\" complete, press any key"
            puts {...}
            gets stdin
        }
    }

    cd $cdir
}

