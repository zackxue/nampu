/*
 * hm_signal.c
 *
 * This file contains functions which set handlers of signals that we care.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
 *
 * Actions of signal handlers:
 *  SIGPIPE: ignored
 *  SIGSEGV: dump stack trace info to file, then core dump
 *  SIGUSR1: special purpose of application
 *  SIGUSR2: special purpose of application
 *  SIGINT : record to file then exit
 *
*/

#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "nmp_signal.h"
#include "nmp_debug.h"

#define MAX_FRAME			32

#if !defined __CYGWIN__
#	if defined REG_RIP || defined REG_EIP
#  		include <ucontext.h>
#  		define ARCH_X86_WITH_CONTEXT
#  		if defined REG_RIP
#   		define ARCH_X86_64
#   		define REG_WIGTH "%016lx"
#		else
#   		define ARCH_X86_32
#   		define REG_WIGTH "%08x"
#  		endif
# 	else
#  		include <execinfo.h>
# 	endif
#endif

#if defined ARCH_X86_WITH_CONTEXT

typedef struct _HmStackFrame HmStackFrame;
struct _HmStackFrame	/* for walking stack frames */
{
	HmStackFrame	*next_frame;
	void *ip;
};


static __inline__ void
hm_dump_registers_to_file(FILE *file, ucontext_t *context)
{
#define __dump(...) fprintf(file, __VA_ARGS__)
#define __regs context->uc_mcontext.gregs

#ifdef ARCH_X86_64
    __dump("r8 :\t"REG_WIGTH"\n", __regs[REG_R8]);
    __dump("r9 :\t"REG_WIGTH"\n", __regs[REG_R9]);
    __dump("r10:\t"REG_WIGTH"\n", __regs[REG_R10]);
    __dump("r11:\t"REG_WIGTH"\n", __regs[REG_R11]);
    __dump("r12:\t"REG_WIGTH"\n", __regs[REG_R12]);
    __dump("r13:\t"REG_WIGTH"\n", __regs[REG_R13]);
    __dump("r14:\t"REG_WIGTH"\n", __regs[REG_R14]);
    __dump("r15:\t"REG_WIGTH"\n", __regs[REG_R15]);
    __dump("rax:\t"REG_WIGTH"\n", __regs[REG_RAX]);
    __dump("rbx:\t"REG_WIGTH"\n", __regs[REG_RBX]);
    __dump("rcx:\t"REG_WIGTH"\n", __regs[REG_RCX]);
    __dump("rdx:\t"REG_WIGTH"\n", __regs[REG_RDX]);
    __dump("rsi:\t"REG_WIGTH"\n", __regs[REG_RSI]);
    __dump("rdi:\t"REG_WIGTH"\n", __regs[REG_RDI]);
    __dump("rbp:\t"REG_WIGTH"\n", __regs[REG_RBP]);
    __dump("rsp:\t"REG_WIGTH"\n", __regs[REG_RSP]);
    __dump("rip:\t"REG_WIGTH"\n", __regs[REG_RIP]);
#else
    __dump("eax:\t"REG_WIGTH"\n", __regs[REG_EAX]);
    __dump("ebx:\t"REG_WIGTH"\n", __regs[REG_EBX]);
    __dump("ecx:\t"REG_WIGTH"\n", __regs[REG_ECX]);
    __dump("edx:\t"REG_WIGTH"\n", __regs[REG_EDX]);
    __dump("esi:\t"REG_WIGTH"\n", __regs[REG_ESI]);
    __dump("edi:\t"REG_WIGTH"\n", __regs[REG_EDI]);
    __dump("ebp:\t"REG_WIGTH"\n", __regs[REG_EBP]);
    __dump("esp:\t"REG_WIGTH"\n", __regs[REG_ESP]);
    __dump("eip:\t"REG_WIGTH"\n", __regs[REG_EIP]);
    __dump("cs :\t"REG_WIGTH"\n", __regs[REG_CS]);
    __dump("ds :\t"REG_WIGTH"\n", __regs[REG_DS]);
    __dump("es :\t"REG_WIGTH"\n", __regs[REG_ES]);
    __dump("ss :\t"REG_WIGTH"\n", __regs[REG_SS]);
#endif

#undef __dump
#undef __regs
}


static __inline__ void
hm_dump_stack_to_file(FILE *file, ucontext_t *context)
{
    int max_f = MAX_FRAME, f = 0; /* frame no */
    Dl_info dlinfo;
    HmStackFrame *frame;
    void *ip;	

#define __dump(...) fprintf(file, __VA_ARGS__)
#define __regs context->uc_mcontext.gregs

#ifdef ARCH_X86_64
    ip = (void*)__regs[REG_RIP];
    frame = (HmStackFrame*)__regs[REG_RBP];
#else
    ip = (void*)__regs[REG_EIP];
    frame = (HmStackFrame*)__regs[REG_EBP];
#endif

    __dump("Stack trace:\n");

    while (frame && ip)
    {
        if (!dladdr(ip, &dlinfo))
            break;

        __dump("% 2d: %p <%s+%x> (%s)\n",
            ++f,
            ip,
            dlinfo.dli_sname,
            (unsigned)(ip - dlinfo.dli_saddr),
            dlinfo.dli_fname
        );

		if (--max_f <= 0)
			break;

        ip = frame->ip;
        frame = frame->next_frame;
	}

	__dump("End of stack trace\n");

#undef __dump
#undef __regs
}

#else


static __inline__ void
hm_dump_stack_to_file(FILE *file)
{
	void *bt[MAX_FRAME];
    char **strings;
    size_t sz;
    int i;

#define __dump(...) fprintf(file, __VA_ARGS__)

    __dump("Stack trace (non-dedicated):\n");

    sz = backtrace(bt, MAX_FRAME);
    strings = backtrace_symbols(bt, sz);
	
	if (strings)
	{
	    for(i = 0; i < sz; ++i)
	        __dump("%s\n", strings[i]); 
	
	    free(strings);
	}

    __dump("End of stack trace\n");

#undef __dump
}
	

#endif	/* ARCH_X86_WITH_CONTEXT */

extern FILE *hm_debug_get_log_file( void );

/*
 * Dump stack and registers when SIGSEGV catched. The following values can 
 * be placed in si_code for a SIGSEGV signal: 
 *  SEGV_MAPERR: address not mapped to ojbect.
 *  SEGV_ACCERR: invalid permissions for mapped object.
 *
 * In source codes, SEGV_MAPERR is defined to __SI_FAULT|1 and SEGV_ACCERR
 *  is __SI_FAULT|2. In user space, __SI_FAULT is 0.
 */
static void
hm_sig_sigsegv_handler(int signum, siginfo_t *info, void *ptr)
{
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};
    FILE *file;

    file = hm_debug_get_log_file();

#define __dump(...) fprintf(file, __VA_ARGS__)
    __dump("Segmentation Fault.\n");
    __dump("info.si_signo = %d\n", signum);
    __dump("info.si_errno = %d\n", info->si_errno);
    __dump("info.si_code  = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    __dump("info.si_addr  = %p\n", info->si_addr);

#if defined ARCH_X86_WITH_CONTEXT
    ucontext_t *ucontext = (ucontext_t*)ptr;
	hm_dump_registers_to_file(file, ucontext);
	hm_dump_stack_to_file(file, ucontext);
#else
	hm_dump_stack_to_file(file);
#endif

    fflush(file);

#undef __dump
}


/*
 * Dump stack and registers when SIGABRT catched. This usually works when
 * assert() failed, or abort invoked.
*/
static void
hm_sig_sigabrt_handler(int signum, siginfo_t *info, void *ptr)
{
    FILE *file;

    file = hm_debug_get_log_file();

#define __dump(...) fprintf(file, __VA_ARGS__)
    __dump("Aborted.\n");
    __dump("info.si_signo = %d\n", signum);
    __dump("info.si_errno = %d\n", info->si_errno);
    __dump("info.si_code  = %d\n", info->si_code);
    __dump("info.si_addr  = %p\n", info->si_addr);

#if defined ARCH_X86_WITH_CONTEXT
    ucontext_t *ucontext = (ucontext_t*)ptr;
	hm_dump_registers_to_file(file, ucontext);
	hm_dump_stack_to_file(file, ucontext);
#else
	hm_dump_stack_to_file(file);
#endif

    fflush(file);

#undef __dump
}


static __inline__ void
hm_sig_setup_sigsegv( void )
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = hm_sig_sigsegv_handler;
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;

    if (sigaction(SIGSEGV, &action, NULL))
        hm_warning("<signal> SIGSEGV handler wasn't set!");
}


static __inline__ void
hm_sig_setup_sigint( void )
{
	
}


static __inline__ void
hm_sig_setup_sigpipe( void )
{
	signal(SIGPIPE, SIG_IGN);
}


static __inline__ void
hm_sig_setup_sigabrt( void )
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = hm_sig_sigabrt_handler;
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;

    if (sigaction(SIGABRT, &action, NULL))
        hm_warning("<signal> SIGABRT handler wasn't set!");	
}


void
hm_sig_setup_signals( void )
{
    hm_sig_setup_sigsegv();
    hm_sig_setup_sigint();
    hm_sig_setup_sigpipe();
    hm_sig_setup_sigabrt();
}


//:~ End
