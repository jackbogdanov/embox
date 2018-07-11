
/**
 * @file
 * @brief
 *
 * @author  Bogdanov Evgenij
 * @date    09.07.2018
 */

#include <stdio.h>

#include <mem/phymem.h>


int main() {

	char *const phymem_alloc_start = phymem_allocated_start();
	char *const phymem_alloc_end = phymem_allocated_end();
	const size_t mem_len = phymem_alloc_end - phymem_alloc_start;

	printf(" start=%p\n end=%p\n size=%zu\n", phymem_alloc_start, phymem_alloc_end, mem_len);

	return 0;
}
