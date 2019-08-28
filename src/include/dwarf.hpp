#ifndef _DWARF_H
#define _DWARF_H

#include "../../external/libelfin/dwarf/dwarf++.hh"
#include "../../external/libelfin/elf/elf++.hh"


int 	db_inform_init(char* progname);


dwarf::die 	get_function_from_pc(long long pc);


dwarf::line_table::iterator 	get_line_entry_from_pc(long long pc);


void 	set_breakpoint_at_function(int pid, const std::string& name);


void 	print_source(const std::string& file_name, unsigned line, unsigned n_lines_context);


#endif