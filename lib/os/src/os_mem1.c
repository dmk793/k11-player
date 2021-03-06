/*
 *     Yet another operating system for microcontrollers.
 *     Dynamic memory manipulation functions.
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
 *
 * Allocate only implementation.
 */
#include "os_private.h"
#include "os_mem.h"

#ifdef OS_CONFIG_DYNMEM_1

STATIC BASE_TYPE pdmem;

/*
 * 
 */
void *os_malloc(BASE_TYPE size)
{
    void *ret;

    /* NOTE align size */
    size += (sizeof(BASE_TYPE) - (size % sizeof(BASE_TYPE)));

    OS_DISABLE_IRQ();
#ifdef OS_CONFIG_TRAP_DYNMEM
    if ((pdmem + size) > OS_CONFIG_DYNMEM_SIZE)
    {
        os_trapinfo.err = OS_TRAP_ERR_DYNMEM_EXHAUST;
        os_trap();
    }
#endif

    {
        ret = &dmem[pdmem];
        pdmem += size;
    }
    OS_ENABLE_IRQ();
    return ret;
}

/*
 *
 */
void os_mfree(void *p)
{
    /* NOTE not implemented */
}

#endif /* OS_CONFIG_DYNMEM_1 */

