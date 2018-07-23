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

static void print_vmem(struct page_allocator *pgd_allocator,
		struct page_allocator *pmd_allocator, struct page_allocator *pte_allocator) {
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

static void print_phymem(struct page_allocator *allocator) {
	char *const phymem_alloc_start = phymem_allocated_start();
	char *const phymem_alloc_end = phymem_allocated_end();
	const size_t mem_len = phymem_alloc_end - phymem_alloc_start;

	printf("\nPHYSICAL MEMORY:\n");
	printf("start=%p end=%p size=%zu\n", phymem_alloc_start, phymem_alloc_end, mem_len);
	printf("-----------------pages-------------------\n");
	printf("first phy_page adr - %p\n", allocator->pages_start);
	printf("pages count / free - %zu / %zu\n", allocator->pages_n, allocator->free / allocator->page_size);
	printf("-----------------------------------------\n");
}

static void print_sections(void) {
	extern char _heap_vma, _text_vma, _rodata_vma, _data_vma, _bss_vma, _bss_reserve;
	extern char _heap_end, _text_end, _rodata_end, _data_end, _bss_end, _reserve_end;
	extern char _heap_len, _text_len, _rodata_len, _data_len, _bss_len, _bss_len_with_reserve;

	int isHeapOn = (int) &_heap_len;

	printf("TEXT:   begin - %p, end - %p, len - %p\n", &_text_vma, &_text_end ,&_text_len);
	printf("RODATA: begin - %p, end - %p, len - %p\n", &_rodata_vma, &_rodata_end ,&_rodata_len);
	printf("DATA:   begin - %p, end - %p, len - %p\n", &_data_vma, &_data_end, &_data_len);
	printf("BSS:    begin - %p, end - %p, len - %p\n        "
			"reserve - %p, end_with_reserve - %p, len_with_reserve - %p\n\n",
			&_bss_vma, &_bss_end, &_bss_len,
			&_bss_reserve, &_reserve_end, &_bss_len_with_reserve);

	if (isHeapOn) {
		printf("HEAP:   begin - %p, end - %p, len - %p\n", &_heap_vma, &_heap_end, &_heap_len);
	} else {
		printf("HEAP:   hasn't section region!\n");
	}

	printf("-----------------------------------------\n\n");
}

static void print_segments() {
	struct _segment *buff[MAX_SEGMENTS];;
	int size;

	printf("PERIPH SEGMENTS:\n");
	periph_description(buff, &size);

	for (int i = 0; i < size; i++) {
		if (!buff[i]->start) {
			break;
		}
		printf("seg_num %d: start - %x    end - %x\n", i, buff[i]->start, buff[i]->end);
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
