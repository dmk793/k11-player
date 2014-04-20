#!/bin/sh
#\
exec tclsh "$0" "$@"

#
# generate list of ignored files for Fossil SCM
#


proc usage {} {
    puts {Usage:}
    puts {}
    puts {# puts list of ignored files}
    puts "       [file tail [info script]] list"
    puts {}
    puts {# puts comma separated list of ignored files}
    puts "       [file tail [info script]] comma"
    puts {}
    exit 1
}

if {$argc < 1} {
    usage
}

set ::TYPE [lindex $argv 0]
if {$::TYPE ne "list" && $::TYPE ne "comma"} {
    usage
}

proc fordir {dir files} {
    foreach f $files {
        lappend ::IGNORE $dir$f
    }
}

#===========================================================
#= Files to ignore are below
#=========================== 

lappend ::IGNORE *session.vim
lappend ::IGNORE *tags
lappend ::IGNORE *.o
lappend ::IGNORE *.hex
lappend ::IGNORE *.bin
lappend ::IGNORE *.map
lappend ::IGNORE *.lst
lappend ::IGNORE *.out
lappend ::IGNORE *.a
lappend ::IGNORE *depfile.mk

#fordir doc [list *.dvi *.pdf *.aux *.log *.out *.toc *.eps *.jpg *.png]
#fordir doc/geda [list *.pcb- *.sch~]
#
#fordir firmware [list *.cod *.hex *.lst funclist *.cof *.hxl *.sym *.sdb *.p1 *.pre *.obj *.as *.rlf *.o]

#lappend ::IGNORE utils/cyssc/cyssc
#fordir utils/cyssc/test [list *.hex *.cod *.lst]
#fordir utils [list *.o *.gdb]


#-----------------------------------------------------------

for {set idx 0} {$idx < [llength $::IGNORE]} {incr idx} {
    puts -nonewline [lindex $::IGNORE $idx]

    if {$idx < ([llength $::IGNORE] - 1)} {
        if {$::TYPE eq "comma"} {
            puts -nonewline {,}
        } else {
            puts -nonewline { }
        }
    }
}

