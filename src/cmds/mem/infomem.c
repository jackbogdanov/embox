/**
 * @file
 * @brief
 *
 * @author  Bogdanov Evgenij
 * @date    09.07.2018
 */

#include <stdio.h>

#include <mem/phymem.h>
#include <mem/vmem/vmem_alloc.h>

#include <mem/vmem.h>
#include <drivers/common/memory.h>

static void print_vmem(struct page_allocator * pgd_allocator,
		struct page_allocator * pmd_allocator, struct page_allocator * pte_allocator) {
	int is_on_vmem = vmem_mmu_enabled();

		printf("VIRTUAL MEMORY:\n");
		printf("IS_ON - %d\n", is_on_vmem);


		if (is_on_vmem) {
			printf("max pages count / free - %llu / %zu\n",
				(unsigned long long) (pgd_allocator->pages_n * pmd_allocator->pages_n) * pte_allocator->pages_n,
				(pgd_allocator->free / pgd_allocator->page_size) *
				(pmd_allocator->free / pmd_allocator->page_size) *
				(pte_allocator->free / pte_allocator->page_size));
		}
		printf("-----------------------------------------\n");
}

static void print_phymem(struct page_allocator * allocator) {
	char *const phymem_alloc_start = phymem_allocated_start();
	char *const phymem_alloc_end = phymem_allocated_end();
	const size_t mem_len = phymem_alloc_end - phymem_alloc_start;

	printf("PHYSICAL MEMORY:\n");
	printf(" start=%p end=%p size=%zu\n", phymem_alloc_start, phymem_alloc_end, mem_len);
	printf("-----------------pages-------------------\n");
	printf(" first phy_page adr - %p\n", allocator->pages_start);
	printf(" pages count / free - %zu / %zu\n", allocator->pages_n, allocator->free / allocator->page_size);
	printf("-----------------------------------------\n");
}

int main() {



	print_phymem(phy_allocator());
	print_vmem(get_pgd_allocator(), get_pmd_allocator(), get_pte_allocator());
	print_segments();
	printf("-----------------------------------------\n");
	print_sections();
	printf("-----------------------------------------\n");
	//heap_info();

	return 0;
}


