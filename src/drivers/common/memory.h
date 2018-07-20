/**
 * @file memory.h
 * @brief
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version
 * @date 2016-08-11
 */

#ifndef _DRIVERS_COMMON_MEMORY_H
#define _DRIVERS_COMMON_MEMORY_H

#include <stdint.h>

#define MAX_SEGMENTS 64

struct periph_memory_desc {
	uint32_t start;
	uint32_t len;
};

struct _segment {
	uint32_t start;
	uint32_t end;
};

#define PERIPH_MEMORY_DEFINE(_mem_desc)	\
	ARRAY_SPREAD_DECLARE(const struct periph_memory_desc *, \
		__periph_mem_registry);	\
	ARRAY_SPREAD_ADD(__periph_mem_registry, \
		&_mem_desc)

extern void periph_description(struct _segment ** buff, int * size);

#endif /* _DRIVERS_COMMON_MEMORY_H */
