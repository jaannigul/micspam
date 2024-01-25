#ifndef STS_QUEUE_H
#define STS_QUEUE_H

/*
 * Simple thread safe queue (StsQueue) is just an experiment with
 * simulated "namespaces" in C.
 *
 * The idea is simple, just return the functions in a struct.
 * Leak exactly the information that is needed, meaning no special
 * types, structs or other things.
 */

#include <Windows.h>

typedef struct StsHeader StsHeader;

EXTERN_C_START

typedef struct {
	StsHeader* (* const create)();
	void (* const destroy)(StsHeader* handle);
	void (* const removeAll)(StsHeader* header);
	int (* const isEmpty)(StsHeader* header);
	int (* const getCurrentPriority)(StsHeader* header);
	void (* const push)(StsHeader* handle, void* elem, int priority);
	void* (* const pop)(StsHeader* handle);
} _StsQueue;

EXTERN_C _StsQueue const StsQueue;

EXTERN_C_END

#endif