#ifndef ECSACT_CODEGEN_WRITE_H
#define ECSACT_CODEGEN_WRITE_H

#include <stdint.h>

extern "C" void (*ecsact_codegen_write)
	( const char*  str
	, int32_t      str_len
	) = nullptr;

#endif//ECSACT_CODEGEN_WRITE_H
