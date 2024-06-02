/*
 * Copyright (c) 2012-2016, Juniper Networks, Inc.
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

#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "dptrace.h"
#include "gdb_interface.h"
#include "memory.h"
#include "target.h"
#include "macros.h"

int ptrace_threadextrainfo_query(int64_t thread_id, char *out_buf)
{
	int ret = RET_ERR;
	int index;
	index = target_index((pid_t)thread_id);
	if (index >= 0) {
		sprintf(out_buf, "LWP %d", (pid_t)thread_id);
		ret = RET_OK;
	} else {
		/* gdb can also pass in the process id */
		if (target_is_alive_process(thread_id)) {
			sprintf(out_buf, "Main Process");
			ret = RET_OK;
		}
	}
	return ret;
}

static int ptrace_support_multiprocess()
{
	return _target.multiprocess;
}

int ptrace_set_ctrl_thread(int64_t process_id, int64_t thread_id)
{
	if (process_id == -1) {
		/* Every thread can be the control thread, so nothing to do here.. */
		return RET_OK;
	}
	/* Nothing here yet */
	return RET_NOSUPP;
}

int ptrace_is_thread_alive(int64_t pid, int64_t tid, int *alive)
{
	int64_t key;
	int out_alive = 0; /* dead */
	if (ptrace_support_multiprocess()) {
		/* pid and tid are valid */
		key = tid;
	} else {
		/* only pid is valid, but it is really tid */
		key = pid;
	}

	out_alive = target_is_alive_thread(key);
	if (!out_alive) {
		/* gdb can pass in a process id, so check processes too */
		out_alive = target_is_alive_process(key);
	}

	*alive = out_alive;
	return RET_OK;
}

int ptrace_process_query(unsigned int *mask, gdb_thread_ref *arg,
			 rp_thread_info *info)
{
	/* Similar to qThreadExtraInfo, nothing to add here */
	return RET_NOSUPP;
}

int ptrace_list_query(int first, gdb_thread_ref *arg, gdb_thread_ref *result,
		      size_t max_num, size_t *num, int *done)
{
	return RET_ERR;
}

int ptrace_current_thread_query(int64_t *pid, int64_t *tid)
{
	*pid = CURRENT_PROCESS_PID;
	*tid = CURRENT_PROCESS_TID;
	return RET_OK;
}

static int ptrace_query_current_signal(int *s)
{
	int ret = RET_ERR;
	if (s) {
		int sig;
		/* The raw signal */
		sig = ptrace_get_signal();
		/* The gdb translation */
		*s = ptrace_arch_signal_to_gdb(sig);
		ret = RET_OK;
	}
	return ret;
}

gdb_target ptrace_target = {
	.next = NULL,
	.name = "ptrace",
	.desc = "native ptrace target",
	.help = ptrace_help,
	.open = ptrace_open,
	.attach = ptrace_attach,
	.detach = ptrace_detach,
	.detach_wait = ptrace_detach_wait,
	.close = ptrace_close,
	.connect = ptrace_connect,
	.disconnect = ptrace_disconnect,
	.kill = ptrace_kill,
	.quick_kill = ptrace_quick_kill,
	.restart = ptrace_restart,
	.stop = ptrace_stop,
	.set_gen_thread = ptrace_set_gen_thread,
	.set_ctrl_thread = ptrace_set_ctrl_thread,
	.is_thread_alive = ptrace_is_thread_alive,
	.read_registers = ptrace_read_registers,
	.write_registers = ptrace_write_registers,
	.read_single_register = ptrace_read_single_register,
	.write_single_register = ptrace_write_single_register,
	.read_mem = memory_read_gdb,
	.write_mem = memory_write_gdb,
	.resume_from_current = ptrace_resume_from_current,
	.resume_from_addr = ptrace_resume_from_addr,
	.go_waiting = ptrace_go_waiting,
	.wait = ptrace_wait,
	.quick_signal = ptrace_quick_signal,
	.process_query = ptrace_process_query,
	.list_query = ptrace_list_query,
	.current_thread_query = ptrace_current_thread_query,
	.offsets_query = ptrace_offsets_query,
	.crc_query = ptrace_crc_query,
	.get_tls_address = ptrace_get_tls_address,
	.add_break = ptrace_add_break,
	.remove_break = ptrace_remove_break,
	.threadinfo_query = ptrace_threadinfo_query,
	.threadextrainfo_query = ptrace_threadextrainfo_query,
	.supported_features_query = ptrace_supported_features_query,
	.query_current_signal = ptrace_query_current_signal,
	.support_multiprocess = ptrace_support_multiprocess,
	.get_xml_register_string = ptrace_get_xml_register_string,
	.set_xml_register_reporting = ptrace_set_xml_register_reporting,
	.register_info = ptrace_register_info,
	.memory_region_info = memory_region_info_gdb,
	.read_auxv = ptrace_read_auxv,
};

void target_init(struct gdb_target_s **target)
{
	*target = &ptrace_target;
}
void target_cleanup()
{ /* Add cleanup code here */
}
