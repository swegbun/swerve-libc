/*
 * This file is part of the swerve libc.
 *
 * Copyright (C) 2019 the swerve development team (see AUTHORS.md).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct MemNode {
	char free;
	void *ptr;
	size_t size;
	struct MemNode *next;
};

struct MemNode *top;

#ifdef __is_libk
/* 
 * initialize memory allocator. if this isn't done before a malloc(),
 * bad things will probably happen, so beware hackers.
 */
void mem_init(void)
{
	top = (struct MemNode *) KERN_MEMORY_BASE;
	memset(top, 0, sizeof(struct MemNode));
	top->ptr = (void *) PROC_MEMORY_BASE;
	top->size = 1;
	top->free = 0;
	top->next = NULL;
}
#endif

/* 
 * malloc(3p) implementation
 * TODO safety check for overflow of kernel memory
 * TODO base off a memory map for safety on actual hardware (may differ)
 */
void *malloc(size_t size)
{
	struct MemNode *node = top;

	while (node->next != NULL) {
		node = node->next;
		if (node->size >= size && node->free)
			return node->ptr;
	}

	node->next = (struct MemNode *) (node + sizeof(struct MemNode));
	node->next->ptr = (void *) (((size_t) node->ptr) + node->size);
	node = node->next;
	node->size = size;
	node->free = 0;
	node->next = NULL;
	return node->ptr;
}

/* 
 * free(3p) implementation
 */
void free(void *mem)
{
	struct MemNode *node = top;

	while (node->ptr != mem && node != NULL)
		node = node->next;
	node->free = 1;
}
