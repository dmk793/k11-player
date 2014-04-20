/*
 * dport - control tool for debug port.
 *
 * Copyright (c) 2013, Dmitry Kobylin
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <tcl.h>
#include <limits.h>
#include <sys/stat.h>
#include "commdev.h"
#include "version.h"
#include "errcode.h"
#include "debug.h"
#include "tcl_interface.h"

char script[PATH_MAX] = {0};
char dev[PATH_MAX]    = {0};
char elf[PATH_MAX]    = {0};
struct commdev_t commdev;

static void get_args(int argc, char** argv, Tcl_Interp *interp);
static void print_head();
static void print_help(int argc, char **argv);

#define DEFAULT_SCRIPT    "dport.tcl"
#define ENV_DPORT_TTY     "DPORT_TTY"

/*
 *
 */
int main(int argc, char** argv)
{
    int err;
    Tcl_Interp *interp;

    err = 0;

    print_head();

    interp = Tcl_CreateInterp();
    if (interp == NULL)
    {
        fprintf(stderr, "can't create interpreter\n");
        return ERRCODE_TCL;
    }

    /* get device name from environment */
    {
        char *denv;

        denv = getenv(ENV_DPORT_TTY);
        if (denv != NULL)
            strcpy(dev, denv);
    }

    /* check existance of default startup script */
    {
        struct stat statbuf;

        if (stat(DEFAULT_SCRIPT, &statbuf) == 0)
            strcpy(script, DEFAULT_SCRIPT);
    }

    /* parse cmd line */
    get_args(argc, argv, interp);

    if (strlen(dev) == 0)
    {
        fprintf(stderr, "communication port not specified, see --dev option\n");
        return ERRCODE_DEVSTR;
    }

    /* print miscellaneous variables */
    {
        if (strlen(script))
            printf("SCRIPT    %s\n", script);

        printf(    "TTY       %s\n", dev);
        printf("\n");
    }

    /* initialize communication port */
    commdev_init();
    commdev_open(&commdev, dev);

    /* initialize some debug structures */
    debug.commdev = &commdev;
    debug.interp  = interp;
    debug.run     = 1;
    debug.cont    = 0;

    /* 
     * Target's buffer size should be determined
     * befor we can execute some commands
     */
    if (debug_cmd_get_bufsize(&debug) < 0)
    {
        err = ERRCODE_BUFSIZE;
        goto err_getbufsize;
    }

    if (Tcl_AppInit(interp) != TCL_OK)
    {
        err = ERRCODE_TCL;
        goto err_tcl;
    }

    err = debug_loop();

err_getbufsize:
err_tcl:
    commdev_close(&commdev);
    commdev_exit();

    return err;
}

/*
 *
 */
static void get_args(int argc, char** argv, Tcl_Interp *interp)
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
        {
            print_help(argc, argv);
            exit(ERRCODE_PARAM);
        } else if (sscanf(argv[i], "--script=%s", script)) {

        } else if (sscanf(argv[i], "--dev=%s", dev)) {

        } else if (sscanf(argv[i], "--elf=%s", elf)) {

        } else {
            printf("unknown option \"%s\"\n", argv[i]);
            exit(ERRCODE_PARAM);
        }
    }
}

/*
 *
 */
static void print_head()
{
    printf("Control tool for debug port. Written and copyrights by Dmitry Kobylin.\n");
    printf("Version %u.%u.%u ("__DATE__")\n", MAJOR, MINOR, BUILD);
    printf("THIS SOFTWARE COMES WITH ABSOLUTELY NO WARRANTY! USE AT YOUR OWN RISK!\n");
    printf("\n");
}

/*
 *
 */
static void print_help(int argc, char **argv)
{
    printf("Usage: %s <options>\n", argv[0]);
    printf("\n");
    printf("Options supported:\n");
    printf("    --dev=<path>       communication port for device\n");
    printf("    --script=<script>  run script on startup, instead of \"dport.tcl\"\n");
    printf("    --elf=<path>       read debug symbols from file\n");
    printf("    -h, --help         print this help\n");
    printf("\n");
    printf("Examples:\n");
    printf("    %s --dev=/dev/ttyS0 --script=foobar.tcl\n", argv[0]);
    printf("\n");
    printf("Environment variables:\n");
    printf("    DPORT_TTY       tty to open at startup, overided by \"--dev\" option\n"
           "                    if specified\n");
}

/*
 *
 */
int Tcl_AppInit(Tcl_Interp *interp)
{
    if (Tcl_Init(interp) == TCL_ERROR)
        return TCL_ERROR;

    if (tcl_interface_init(interp, &debug) != TCL_OK)
    {
        fprintf(stderr, "%s, tcl interface init error", __FUNCTION__);
        return TCL_ERROR;
    }

    if (strlen(script) && Tcl_EvalFile(interp, script) != TCL_OK)
    {
        char *result;

        result = Tcl_GetStringFromObj(Tcl_GetObjResult(interp), NULL);
        if (result)
        {
            printf("*************\n");
            Tcl_Eval(interp, "puts $::errorInfo");
            printf("*************\n");
        }

        return TCL_ERROR;
    }

    return TCL_OK;
}

