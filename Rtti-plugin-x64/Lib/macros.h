#pragma once

#define ADDPTR(ptr, offset, type) (type)((((unsigned char*)ptr) + offset))
#define SUBPTR(ptr, add) PVOID((PBYTE(ptr) - size_t(add)))