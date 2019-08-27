#include "include/breakpoint.h"
#include "ds/lists.h"
#include <sys/user.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>

extern list_t* breakpoint_list;


int breakpoint_compare(void* address, void* list_node_value)
{
	if ( (long)address == ((breakpoint_t*)list_node_value)->address )
		return 1;
	return 0;
}
	


void breakpoints_init()
{
	breakpoint_list = list_create();
}



breakpoint_t* breakpoint_create(pid_t dbpid, void* value)
{
	breakpoint_t* bp = (breakpoint_t*)malloc(sizeof(breakpoint_t));
	bp->address = (long)value;
	bp->orig_data = ptrace(PTRACE_PEEKDATA, dbpid, bp->address, 0);
	list_add(breakpoint_list, bp);
	return bp;
}



int breakpoint_enable(pid_t dbpid, long address)
{
	list_node_t* node_bp = list_search(breakpoint_list, (void*) address, &breakpoint_compare);
	breakpoint_t* bp;

	if ( node_bp != NULL )
	{
		bp = (breakpoint_t*)node_bp->value;
	}
	else
	{
		bp = breakpoint_create(dbpid, (void*) address); 
	}

	
	if (bp->orig_data < 0)
	{
		fprintf(stderr, "ptrace peekdata error\n");
		return -1;
	}

	bp->enabled = 1;
	bp->address = address;

	if (ptrace(PTRACE_POKEDATA, dbpid, address, ((bp->orig_data & ~0xFF) | 0xCC)) < 0)
	{
		fprintf(stderr, "ptrace pokedata error\n");
		return -1;
	}
	
	return 0;
}



int breakpoint_disable(pid_t dbpid, long address)
{
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS, dbpid, 0, &regs);
	
	
	printf( "stopped at: 0x%08x\n", --regs.rip);
	ptrace(PTRACE_SETREGS, dbpid, 0, &regs);

	list_node_t* node_bp = list_search(breakpoint_list, (void*) address, &breakpoint_compare);
	breakpoint_t* bp;


	if (node_bp != NULL)
	{
		bp = (breakpoint_t*)(node_bp->value);

		if (ptrace(PTRACE_POKEDATA, dbpid, address, bp->orig_data) < 0)
		{
			fprintf(stderr, "ptrace pokedata error\n");
			return -1;
		}

		if (bp->enabled)
			bp->enabled = 0;
		else
		{
			fprintf(stderr, "disable breakpoint that was disabled");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "can't find the breakpoint with address 0x%08x\n", (long)address);
		return -1;
	}
	return 0;
}
