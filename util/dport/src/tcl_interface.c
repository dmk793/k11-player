/*
 * Tcl interface for dtool.
 *
 * TODO
 *     * help command
 */
#include <string.h>
#include <stdio.h>
#include <tcl.h>
#include "tcl_interface.h"
#include "tcl_def.h"
#include "debug.h"
#include "dport_proto.h"


static int TclCont_Cmd       (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclStep_Cmd       (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclHelp_Cmd       (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclQuit_Cmd       (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclRegs_Cmd       (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclReadReg_Cmd    (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclWriteReg_Cmd   (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclReset_Cmd      (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclPrintMem_Cmd   (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclReadMem_Cmd    (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclWriteMem_Cmd   (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclDisasm_Cmd     (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclWriteImage_Cmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclWriteFlash_Cmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int TclUserF_Cmd      (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);

#define EXT_ERROR_SIZE    4096
char exterror[EXT_ERROR_SIZE];

/*
 *
 */
int tcl_interface_init(Tcl_Interp *interp, struct debug_t *debug)
{
    Tcl_Namespace *dportns, *globalns;

    dportns  = Tcl_CreateNamespace(interp, "dport", (ClientData)0, NULL);
    globalns = Tcl_GetGlobalNamespace(interp);

    /* create commands */
    Tcl_CreateObjCommand(interp, "help",                TclHelp_Cmd,       (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "quit",                TclQuit_Cmd,       (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::reset",      TclReset_Cmd,      (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::cont",       TclCont_Cmd,       (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::step",       TclStep_Cmd,       (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::regs",       TclRegs_Cmd,       (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::readreg",    TclReadReg_Cmd,    (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::writereg",   TclWriteReg_Cmd,   (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::disasm",     TclDisasm_Cmd,     (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::printmem",   TclPrintMem_Cmd,   (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::readmem",    TclReadMem_Cmd,    (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::writemem",   TclWriteMem_Cmd,   (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::writeimage", TclWriteImage_Cmd, (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::writeflash", TclWriteFlash_Cmd, (ClientData)debug, NULL);
    Tcl_CreateObjCommand(interp, "::dport::userf",      TclUserF_Cmd,      (ClientData)debug, NULL);

    /* export commands from "dport" namespace */
    Tcl_Export(interp, dportns, "cont",       0);
    Tcl_Export(interp, dportns, "step",       0);
    Tcl_Export(interp, dportns, "reset",      0);
    Tcl_Export(interp, dportns, "regs",       0);
    Tcl_Export(interp, dportns, "readreg",    0);
    Tcl_Export(interp, dportns, "writereg",   0);
    Tcl_Export(interp, dportns, "disasm",     0);
    Tcl_Export(interp, dportns, "userf",      0);
    Tcl_Export(interp, dportns, "printmem",   0);
    Tcl_Export(interp, dportns, "readmem",    0);
    Tcl_Export(interp, dportns, "writemem",   0);
    Tcl_Export(interp, dportns, "writeimage", 0);
    Tcl_Export(interp, dportns, "writeflash", 0);

    /* import commands to global namespace */
    Tcl_Import(interp, globalns, "::dport::cont",       1);
    Tcl_Import(interp, globalns, "::dport::step",       1);
    Tcl_Import(interp, globalns, "::dport::reset",      1);
    Tcl_Import(interp, globalns, "::dport::regs",       1);
    Tcl_Import(interp, globalns, "::dport::readreg",    1);
    Tcl_Import(interp, globalns, "::dport::writereg",   1);
    Tcl_Import(interp, globalns, "::dport::disasm",     1);
    Tcl_Import(interp, globalns, "::dport::userf",      1);
    Tcl_Import(interp, globalns, "::dport::printmem",   1);
    Tcl_Import(interp, globalns, "::dport::readmem",    1);
    Tcl_Import(interp, globalns, "::dport::writemem",   1);
    Tcl_Import(interp, globalns, "::dport::writeimage", 1);
    Tcl_Import(interp, globalns, "::dport::writeflash", 1);

    return TCL_OK;
}

/*
 * help - print help
 */
static int TclHelp_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    char *subcmd;
    if (objc <= 1)
    {
        printf(PRINT_NEWLINE "Commands that output data to console:"                        PRINT_NEWLINE PRINT_NEWLINE);
        printf(PRINT_TABSTOP "cont        continue execution on target"                     PRINT_NEWLINE);
        printf(PRINT_TABSTOP "step        step one instruction on target"                   PRINT_NEWLINE);
        printf(PRINT_TABSTOP "quit        exit from program"                                PRINT_NEWLINE);
        printf(PRINT_TABSTOP "reset       reset target"                                     PRINT_NEWLINE);
        printf(PRINT_TABSTOP "regs        print registers of target"                        PRINT_NEWLINE);
        printf(PRINT_TABSTOP "writereg    write register of target"                         PRINT_NEWLINE);
        printf(PRINT_TABSTOP "disasm      disassemble instructions and print to the screen" PRINT_NEWLINE);
        printf(PRINT_TABSTOP "userf       call user-defined function"                       PRINT_NEWLINE);
        printf(PRINT_TABSTOP "printmem    print memory of target to the screen"             PRINT_NEWLINE);
        printf(PRINT_TABSTOP "writemem    write data to memory of target"                   PRINT_NEWLINE);
        printf(PRINT_TABSTOP "writeimage  write binary image file to flash of target"       PRINT_NEWLINE);
        printf(PRINT_TABSTOP "writeflash  write binary data to flash of target"             PRINT_NEWLINE);

        printf(PRINT_NEWLINE "Commands that returns value:"                                 PRINT_NEWLINE PRINT_NEWLINE);
        printf(PRINT_TABSTOP "readmem     read data from memory of target"                  PRINT_NEWLINE);
        printf(PRINT_TABSTOP "readreg     read register value"                              PRINT_NEWLINE);

        return TCL_OK;
    }

#define TCL_HELP_CMD_SUBCOMMAND_OBJC  1
    /* print help for subcommands */
    if (objc > 1)
    {
        subcmd = Tcl_GetString(objv[TCL_HELP_CMD_SUBCOMMAND_OBJC]);
        if (strcmp(subcmd, "disasm") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "disasm addr ?ninstr?"         PRINT_NEWLINE PRINT_NEWLINE);
            printf("Disassemble \"ninstr\" instructions at \"addr\" address." PRINT_NEWLINE);

        } else if (strcmp(subcmd, "userf") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "userf nfunc"                            PRINT_NEWLINE PRINT_NEWLINE);
            printf("Execute user defined function. Number of function is specified by " PRINT_NEWLINE
                    "\"nfunc\" argument." PRINT_NEWLINE);

        } else if (strcmp(subcmd, "printmem") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "printmem addr len"                 PRINT_NEWLINE PRINT_NEWLINE);
            printf("Read \"len\" bytes from target's memory at address specified " PRINT_NEWLINE
                   "by \"addr\" argument. Prints result to the screen"             PRINT_NEWLINE);

        } else if (strcmp(subcmd, "writereg") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "writereg regname value"              PRINT_NEWLINE PRINT_NEWLINE);
            printf("Write \"value\" to register specified by \"regname\" argument. " PRINT_NEWLINE
                   "Look at ouput of \"regs\" command for supported names. "         PRINT_NEWLINE);

        } else if (strcmp(subcmd, "readreg") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "readreg regname"                PRINT_NEWLINE PRINT_NEWLINE);
            printf("Read value of register specified by \"regname\" argument. " PRINT_NEWLINE
                   "Look at ouput of \"regs\" command for supported names. "    PRINT_NEWLINE);

        } else if (strcmp(subcmd, "readmem") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "readmem addr len"                  PRINT_NEWLINE PRINT_NEWLINE);
            printf("Read \"len\" bytes from target's memory at address specified " PRINT_NEWLINE
                   "by \"addr\" argument."                                         PRINT_NEWLINE);

        } else if (strcmp(subcmd, "writemem") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "writemem addr data"          PRINT_NEWLINE PRINT_NEWLINE);
            printf("Write \"data\" to target's memory at address specified " PRINT_NEWLINE
                   "by \"addr\" argument. \"data\" interpreted as binary."   PRINT_NEWLINE);

        } else if (strcmp(subcmd, "writeimage") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "writeimage image offset ?length?"      PRINT_NEWLINE PRINT_NEWLINE);
            printf("Write data from \"image\" to target's flash at address specified " PRINT_NEWLINE
                   "by \"offset\" argument."                                           PRINT_NEWLINE);

        } else if (strcmp(subcmd, "writeflash") == 0) {
            printf(PRINT_NEWLINE PRINT_TABSTOP "writeflash offset data"        PRINT_NEWLINE PRINT_NEWLINE);
            printf("Write data to target's flash at address specified "        PRINT_NEWLINE
                   "by \"offset\" argument. \"data\" interpreted as binary."   PRINT_NEWLINE);

        } else {
            printf(PRINT_NEWLINE "no help topic available for \"%s\" command" PRINT_NEWLINE, subcmd);
        }
        printf(PRINT_NEWLINE);
    }

    return TCL_OK;
}

/*
 * quit - quit from program
 */
static int TclQuit_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    struct debug_t *debug;
    debug = (struct debug_t*)clientData;

    debug->run  = 0;
    debug->cont = 1;

    /* NOTE send cont to target, but not raise error if failed */
    debug_cmd_cont(debug);
    return TCL_OK;
}

/*
 * cont - send continue command to target
 */
static int TclCont_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    if (debug_cmd_cont(debug) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

/*
 * step - send step command to target
 */
static int TclStep_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    if (debug_cmd_step(debug) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

struct qreg_t {
    uint8 regn;
    char name[5];
} qregs[] = {
    {DPORT_REGNUM_R0,    "r0  "}, 
    {DPORT_REGNUM_R1,    "r1  "}, 
    {DPORT_REGNUM_R2,    "r2  "}, 
    {DPORT_REGNUM_R3,    "r3  "}, 
    {DPORT_REGNUM_R4,    "r4  "}, 
    {DPORT_REGNUM_R5,    "r5  "}, 
    {DPORT_REGNUM_R6,    "r6  "}, 
    {DPORT_REGNUM_R7,    "r7  "}, 
    {DPORT_REGNUM_R8,    "r8  "}, 
    {DPORT_REGNUM_R9,    "r9  "}, 
    {DPORT_REGNUM_R10,   "r10 "}, 
    {DPORT_REGNUM_R11,   "r11 "}, 
    {DPORT_REGNUM_R12,   "r12 "}, 
    {DPORT_REGNUM_SP,    "SP  "}, 
    {DPORT_REGNUM_LR,    "LR  "}, 
    {DPORT_REGNUM_PC,    "PC  "}, 
    {DPORT_REGNUM_XPSR,  "xPSR"}, 
    {0xff, ""},
};

/*
 * regs - print registers of target
 */
static int TclRegs_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    uint32 value;
    struct qreg_t *qreg;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    qreg = qregs;

    while (qreg->regn != 0xff)
    {
        if (debug_cmd_read_reg(debug, qreg->regn, &value) < 0)
        {
            sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
            TCL_RETURN_ERROR(error);
        }
        printf(PRINT_TABSTOP "%s %08X (%u)" PRINT_NEWLINE, qreg->name, value, value);
        qreg++;
    }

    return TCL_OK;
}

/*
 * readreg - read register value
 *
 *     readreg regname
 */
static int TclReadReg_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    uint32 value;
    struct qreg_t *qreg;
    char *regname;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    qreg = qregs;

    if (objc < 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "regname");
        return TCL_ERROR;
    }

#define TCL_READREG_CMD_REGNAME_OBJC  1
    regname = Tcl_GetString(objv[TCL_READREG_CMD_REGNAME_OBJC]);

    while (qreg->regn != 0xff)
    {
        if (strncmp(qreg->name, regname, strlen(regname)) == 0)
            break;
        qreg++;
    }
    if (qreg->regn == 0xff)
    {
        sprintf(error, "(E) invalid register name \"%s\" (look at output of \"regs\" command for supproted names)", regname);
        TCL_RETURN_ERROR(error);
    }

    if (debug_cmd_read_reg(debug, qreg->regn, &value) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    TCL_RETURN_INT(value);
}

/*
 * writereg - write register of target
 *
 *     writereg regname value
 */
static int TclWriteReg_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    uint32 value;
    struct qreg_t *qreg;
    char *regname;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    qreg = qregs;

    if (objc < 3)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "regname value");
        return TCL_ERROR;
    }

#define TCL_WRITEREG_CMD_REGNAME_OBJC  1
#define TCL_WRITEREG_CMD_VALUE_OBJC   2
    regname = Tcl_GetString(objv[TCL_WRITEREG_CMD_REGNAME_OBJC]);

    while (qreg->regn != 0xff)
    {
        if (strncmp(qreg->name, regname, strlen(regname)) == 0)
            break;
        qreg++;
    }
    if (qreg->regn == 0xff)
    {
        sprintf(error, "(E) invalid register name \"%s\" (look at output of \"regs\" command for supproted names)", regname);
        TCL_RETURN_ERROR(error);
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_WRITEREG_CMD_VALUE_OBJC], (int*)&value) != TCL_OK)
    {
        sprintf(error, "invalid value of \"value\"");
        TCL_RETURN_ERROR(error);
    }

    if (debug_cmd_write_reg(debug, qreg->regn, value) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    TCL_RETURN_INT(value);
}

/*
 * reset - reset target
 */
static int TclReset_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

    if (debug_cmd_reset(debug) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    debug->cont = 1;
    return TCL_OK;
}

/*
 * readmem - read memory location at target
 *
 *     readmem addr len
 *
 * RETURN
 *     binary data
 */
static int TclReadMem_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    char *buf;
    uint32 len;
    uint32 addr;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_READMEM_CMD_ADDR_OBJC  1
#define TCL_READMEM_CMD_LEN_OBJC   2
    if (objc < 3)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "addr len");
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_READMEM_CMD_ADDR_OBJC], (int*)&addr) != TCL_OK)
    {
        sprintf(error, "invalid value of \"addr\"");
        TCL_RETURN_ERROR(error);
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_READMEM_CMD_LEN_OBJC], (int*)&len) != TCL_OK)
    {
        sprintf(error, "invalid value of \"len\"");
        TCL_RETURN_ERROR(error);
    }

    buf = Tcl_Alloc(len);

    if (debug_cmd_read_mem(debug, addr, len, (uint8*)buf) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        goto error;
    }

    TCL_RETURN_BYTEARRAY_FREE((unsigned char *)buf, len);
error:
    Tcl_Free(buf);
    TCL_RETURN_ERROR(error);
}

/*
 * writemem - write memory of target
 *
 *     writemem addr data
 *
 */
static int TclWriteMem_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    char *buf;
    uint32 len;
    uint32 addr;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_WRITEMEM_CMD_ADDR_OBJC  1
#define TCL_WRITEMEM_CMD_DATA_OBJC  2
    if (objc < 3)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "addr data");
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_WRITEMEM_CMD_ADDR_OBJC], (int*)&addr) != TCL_OK)
    {
        sprintf(error, "invalid value of \"addr\"");
        TCL_RETURN_ERROR(error);
    }

    buf = (char*)Tcl_GetByteArrayFromObj(objv[TCL_WRITEMEM_CMD_DATA_OBJC], (int*)&len);

    if (debug_cmd_write_mem(debug, addr, len, (uint8*)buf) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

/*
 * printmem - print targets memory to the screen
 *
 *     printmem addr len
 */
static int TclPrintMem_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    char *buf;
    uint32 len;
    uint32 addr;
    int i;
    char *format;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_READMEM_CMD_ADDR_OBJC  1
#define TCL_READMEM_CMD_LEN_OBJC   2
    if (objc < 3)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "addr len");
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_READMEM_CMD_ADDR_OBJC], (int*)&addr) != TCL_OK)
    {
        sprintf(error, "invalid value of \"addr\"");
        TCL_RETURN_ERROR(error);
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_READMEM_CMD_LEN_OBJC], (int*)&len) != TCL_OK)
    {
        sprintf(error, "invalid value of \"len\"");
        TCL_RETURN_ERROR(error);
    }

    buf = Tcl_Alloc(len);

    if (debug_cmd_read_mem(debug, addr, len, (uint8*)buf) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        goto error;
    }

    if ((addr + len) <= 256)
        format = PRINT_NEWLINE "%02X |";
    else if ((addr + len) <= 65536)
        format = PRINT_NEWLINE "%04X |";
    else
        format = PRINT_NEWLINE "%08X |";

    for (i = 0; i < len; i++)
    {
        if ((i % 16) == 0)
            printf(format, addr);
        if ((i % 4) == 0)
            printf(" ");
        printf("%02hhX ", buf[i]);
        addr++;
    }
    printf(PRINT_NEWLINE);

//    {
//        darm_t d; darm_str_t str;
//
//        if(darm_thumb2_disasm(&d, *((uint16*)buf), *((uint16*)(buf + 2))) == 0
//                && darm_str2(&d, &str, 1) == 0)
//        {
//            printf("-> %s\n", str.total);
//        }
//    }


    Tcl_Free(buf);
    return TCL_OK;
error:
    Tcl_Free(buf);
    TCL_RETURN_ERROR(error);
}

/*
 * disasm - disassemble instructions at target's memory
 *
 *     disasm addr ninst
 */
static int TclDisasm_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    uint32 ninstr;
    uint32 addr;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_DISASM_CMD_ADDR_OBJC  1
#define TCL_DISASM_CMD_LEN_OBJC   2
    if (objc < 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "addr ?len?");
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_DISASM_CMD_ADDR_OBJC], (int*)&addr) != TCL_OK) {
        sprintf(error, "invalid value of \"addr\"");
        TCL_RETURN_ERROR(error);
    }

    if (objc > 2)
    {
        if (Tcl_GetIntFromObj(interp, objv[TCL_DISASM_CMD_LEN_OBJC], (int*)&ninstr) != TCL_OK)
        {
            sprintf(error, "invalid value of \"ninstr\"");
            TCL_RETURN_ERROR(error);
        }
    } else {
        ninstr = 1;
    }

    if (debug_cmd_disasm(debug, addr, ninstr) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

/*
 * writeimage - write data from file to flash of target
 *
 *     writeimage image offset ?length?
 *
 */
static int TclWriteImage_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    char *file;
    int length;
    uint32 offset;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_WRITEIMAGE_CMD_FILE_OBJC    1
#define TCL_WRITEIMAGE_CMD_OFFSET_OBJC  2
#define TCL_WRITEIMAGE_CMD_LENGTH_OBJC  3
    if (objc < 3)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "image offset ?length?");
        return TCL_ERROR;
    }

    file = Tcl_GetString(objv[TCL_WRITEIMAGE_CMD_FILE_OBJC]);

    if (Tcl_GetIntFromObj(interp, objv[TCL_WRITEIMAGE_CMD_OFFSET_OBJC], (int*)&offset) != TCL_OK)
    {
        sprintf(error, "invalid value of \"offset\"");
        TCL_RETURN_ERROR(error);
    }

    if (objc > 3)
    {
        if (Tcl_GetIntFromObj(interp, objv[TCL_WRITEIMAGE_CMD_LENGTH_OBJC], &length) != TCL_OK)
        {
            sprintf(error, "invalid value of \"length\"");
            TCL_RETURN_ERROR(error);
        }
    } else {
        length = -1;
    }

    if (debug_cmd_write_image(debug, offset, file, length) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

/*
 * writeflash - write data to flash of target
 *
 *     writeflash offset data
 *
 */
static int TclWriteFlash_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    int length;
    uint32 offset;
    char *data;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;

#define TCL_WRITEFLASH_CMD_OFFSET_OBJC  1
#define TCL_WRITEFLASH_CMD_DATA_OBJC    2
    if (objc < 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "data offset");
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[TCL_WRITEFLASH_CMD_OFFSET_OBJC], (int*)&offset) != TCL_OK)
    {
        sprintf(error, "invalid value of \"offset\"");
        TCL_RETURN_ERROR(error);
    }

    data = (char*)Tcl_GetByteArrayFromObj(objv[TCL_WRITEFLASH_CMD_DATA_OBJC], (int*)&length);

    if (debug_cmd_write_flash(debug, data, offset, length) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

/*
 * userf - execute user defined function
 *
 *     userf nfunc
 */
static int TclUserF_Cmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *objp;
    struct debug_t *debug;
    char *error;
    int value;

    /* make compiller happy */
    error = exterror;

    debug = (struct debug_t*)clientData;


    if (objc < 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "nfunc");
        return TCL_ERROR;
    }

#define TCL_USERF_NFUNC_OBJC  1
    if (Tcl_GetIntFromObj(interp, objv[TCL_USERF_NFUNC_OBJC], (int*)&value) != TCL_OK)
    {
        sprintf(error, "invalid value of \"nfunc\"");
        TCL_RETURN_ERROR(error);
    }

    if (debug_cmd_userf(debug, value) < 0)
    {
        sprintf(error, "(E) command \"%s\" failed", Tcl_GetString(objv[0]));
        TCL_RETURN_ERROR(error);
    }

    return TCL_OK;
}

