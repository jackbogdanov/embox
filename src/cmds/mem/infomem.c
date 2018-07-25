/**
 * @file
 * @brief
 *
 * @author  Evgenij Bogdanov
 * @date    09.07.2018
 */

#include <stdio.h>

#include <mem/phymem.h>
#include <mem/vmem/vmem_alloc.h>
#include <mem/vmem.h>

#include <drivers/common/memory.h>

#include <framework/mod/options.h>
#define HEAP_SIZE OPTION_STRING_GET(start_symbol)

static void print_vmem(struct page_allocator *pgd_allocator,
		struct page_allocator *pmd_allocator, struct page_allocator *pte_allocator) {
	int is_on_vmem = vmem_mmu_enabled();

	printf("VIRTUAL MEMORY:\n");
	printf("vmem is %s\n", is_on_vmem ? "ON" : "OFF");

	if (is_on_vmem) {
		if (pte_allocator != NULL) {
			printf("max pages count / free - %llu / %zu\n",
					(unsigned long long) (pgd_allocator->pages_n * pmd_allocator->pages_n) * pte_allocator->pages_n,
					(pgd_allocator->free / pgd_allocator->page_size) *
					(pmd_allocator->free / pmd_allocator->page_size) *
					(pte_allocator->free / pte_allocator->page_size));
		} else {
			printf("PTE table is off!\n");
			printf("max pages count / free - %llu / %zu\n",
					(unsigned long long) (pgd_allocator->pages_n * pmd_allocator->pages_n),
					(pgd_allocator->free / pgd_allocator->page_size) *
					(pmd_allocator->free / pmd_allocator->page_size));
		}
	}
	printf("-----------------------------------------\n");
}

static void print_phymem(struct page_allocator *allocator) {
	char *const phymem_alloc_start = phymem_allocated_start();
	char *const phymem_alloc_end = phymem_allocated_end();
	const size_t mem_len = phymem_alloc_end - phymem_alloc_start;

	printf("\nPHYSICAL MEMORY:\n");
	printf("start=%p end=%p size=%zu\n", phymem_alloc_start, phymem_alloc_end, mem_len);
	printf("-----------------pages-------------------\n");
	printf("first usable phy_page adr - %p\n", allocator->pages_start);
	printf("pages count / free - %zu / %zu\n", allocator->pages_n, allocator->free / allocator->page_size);
	printf("-----------------------------------------\n");
}

static void print_sections(void) {
	extern char _heap_vma, _text_vma, _rodata_vma, _data_vma, _bss_vma;
	extern char _heap_len, _text_len, _rodata_len, _data_len, _bss_len, _bss_len_with_reserve;
	extern char _bss_end, _reserve_end;

	int isHeapOn = (int) &_heap_len;
	printf("SECTIONS:\n");
	printf("TEXT:   begin - %p, end - %p, len - %p\n", &_text_vma, (int) &_text_vma + &_text_len, &_text_len);
	printf("RODATA: begin - %p, end - %p, len - %p\n", &_rodata_vma, (int) &_rodata_vma + &_rodata_len, &_rodata_len);
	printf("DATA:   begin - %p, end - %p, len - %p\n", &_data_vma, (int) &_data_vma + &_data_len, &_data_len);
	printf("BSS:    begin - %p, end - %p, len - %p\n        "
			"reserve - %p, end_with_reserve - %p, len_with_reserve - %p\n\n",
			&_bss_vma, &_bss_end, &_bss_len,
			- (int) &_bss_end + &_reserve_end , &_reserve_end, &_bss_len_with_reserve);

	if (isHeapOn) {
		printf("HEAP:   begin - %p, end - %p, len - %p\n", &_heap_vma, (int) &_heap_vma + &_heap_len, &_heap_len);
	} else {
		printf("HEAP:   hasn't section region!\n");
		printf("Used part of .bss.reserve section!\n");
	}

	printf("-----------------------------------------\n\n");
}

static void print_segments() {
	int size = periph_mem_size();

	printf("PERIPH SEGMENTS:\n");

	for (int i = 0; i < size; i++) {
		struct periph_memory_desc desk;
		periph_elem_desc(&desk, i);
		printf("seg_num %d: start - %x, end - %x, len - %x\n", i, desk.start, desk.start + desk.len, desk.len);
	}

	printf("-----------------------------------------\n");
}

int main() {
	print_phymem(phy_allocator());
	print_vmem(get_pgd_allocator(), get_pmd_allocator(), get_pte_allocator());
	print_segments();
	print_sections();
	return 0;
}
