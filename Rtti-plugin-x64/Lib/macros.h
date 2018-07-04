#pragma once

#define ADDPTR(ptr, offset, type) (type)((((unsigned char*)ptr) + offset))
#define SUBPTR(ptr, sub) PVOID((PBYTE(ptr) - size_t(sub)))

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))