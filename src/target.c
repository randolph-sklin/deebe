/*
 * Copyright (c) 2013-2014 Juniper Networks, Inc.
 * All rights reserved.
 *
 * You may distribute under the terms of :
 *
 * the BSD 2-Clause license
 *
 * Any patches released for this software are to be released under these
 * same license terms.
 *
 * BSD 2-Clause license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>

#include "target.h"

target_state _target = {
	.no_ack = 0, /* ack until it is ok not to */
	.multiprocess = 0, /* default to supporting multiple processes */
	.syscall_enter = false,
	.current_signal = SIGTRAP,
	.flag_attached_existing_process = 1,
	.ps = PS_START,
	.reg_size = 0,
	.freg_size = 0,
	.fxreg_size = 0,
	.dbreg_size = 0,
	.reg_rw = NULL, /* TODO : FREE THIS */
	.freg_rw = NULL, /* TODO : FREE THIS */
	.fxreg_rw = NULL, /* TODO : FREE THIS */
	.dbreg_rw = NULL, /* TODO : FREE THIS */
	.number_processes = 0,
	.current_process = 0,
	.process = NULL, /* TODO : FREE THIS */
};

bool target_new_thread(pid_t pid, pid_t tid)
{
    bool ret = false;
    void *try_process = NULL;
    
    /* Allocate registers for the process */
    try_process = realloc(_target.process,
			  (_target.number_processes + 1) *
			  sizeof(struct target_process_rec));
    if (try_process) {
	_target.process = try_process;
	_target.current_process = _target.number_processes;
	CURRENT_PROCESS_PID   = pid;
	CURRENT_PROCESS_TID   = tid;
	CURRENT_PROCESS_ALIVE = true;
	_target.number_processes++;
	
	ret = true;
	
    } else {
	/* TODO : HANDLE ERROR */
    }

    return ret;
}

int target_number_threads()
{
    int ret = 0;
    int index;
    
    for (index = 0; index < _target.number_processes; index++) {
	if (PROCESS_ALIVE(index))
	    ret++;
    }
    return ret;
}

pid_t target_get_pid() 
{
    pid_t ret = -1;
    ret = PROCESS_PID(0);

    return ret;
}

bool target_dead_thread(pid_t tid) 
{
    bool ret = false;
    int index;
   
    for (index = 0; index < _target.number_processes; index++) {
	if (tid == PROCESS_TID(index)) {
	    PROCESS_ALIVE(index) = false;
	    ret = true;
	    break;
	}
    }
    return ret;
}

void target_all_dead_thread(pid_t tid) 
{
    int index;
    for (index = 0; index < _target.number_processes; index++) {
	PROCESS_ALIVE(index) = false;
    }
}

bool target_alive_thread(pid_t tid) 
{
    bool ret = false;
    int index;
   
    for (index = 0; index < _target.number_processes; index++) {
	if (tid == PROCESS_TID(index)) {
	    PROCESS_ALIVE(index) = true;
	    ret = true;
	    break;
	}
    }
    return ret;
}

bool target_is_tid(pid_t tid)
{
    bool ret = false;
    int index;
   
    for (index = 0; index < _target.number_processes; index++) {
	if (tid == PROCESS_TID(index)) {
	    PROCESS_ALIVE(index) = true;
	    ret = true;
	}
    }
    return ret;
}

void _target_debug_print() {
    int index;
   
    for (index = 0; index < _target.number_processes; index++) {
	fprintf(stderr, "%d %x %x %d\n", index, PROCESS_PID(index), PROCESS_TID(index),
		PROCESS_ALIVE(index));
    }
}
