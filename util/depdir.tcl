#!/bin/sh
#\
exec tclsh "$0" "$@"

if {$argc < 2} {
    #puts stderr [join [list [file tail [info script]] {: No directory prefix specified}] {}]
    puts stderr [join [list [file tail [info script]] {: Not enough arguments}] {}]
    exit 1
}

set FILE    [lindex $argv 0]
set OBJECTS [lrange $argv 1 end]

set fd [open $FILE r]
set data [split [read $fd] '\n']
close $fd

set nline [list]
foreach line $data {
    if {[regexp {(^.*)(:.*$)} $line -> target prereq]} {
        foreach object $OBJECTS {
            if {[string match $target [file tail $object]]} {
                lappend nline $object$prereq
                break
            }
        }
    } else {
        lappend nline $line
    }
}

set fd [open $FILE w]
foreach line $nline {
    puts $fd $line
}
close $fd

