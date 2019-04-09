/**
 * @file
 * @brief
 *
 * @date 05.10.2012
 * @author Anton Bulychev
 */

#include <util/log.h>

#include <errno.h>
#include <assert.h>
#include <string.h>

#include <hal/mmu.h>
#include <util/binalign.h>
#include <mem/vmem.h>
#include <mem/vmem/vmem_alloc.h>

static void vmem_set_pte_flags(mmu_pte_t *pte, vmem_page_flags_t flags);
static int do_map_region(mmu_ctx_t ctx, mmu_paddr_t phy_addr, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags);
static int do_create_space(mmu_ctx_t ctx, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags);

int vmem_map_region(mmu_ctx_t ctx, mmu_paddr_t phy_addr, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags) {
	int res = do_map_region(ctx, phy_addr, virt_addr, reg_size, flags);

	if (res) {
		vmem_unmap_region(ctx, virt_addr, reg_size);
	}

	mmu_flush_tlb();
	return res;
}

int vmem_create_space(mmu_ctx_t ctx, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags) {
	int res = do_create_space(ctx, virt_addr, reg_size, flags);

	if (res) {
		vmem_unmap_region(ctx, virt_addr, reg_size);
	}

	return res;
}

int vmem_page_set_flags(mmu_ctx_t ctx, mmu_vaddr_t virt_addr, vmem_page_flags_t flags) {
	size_t pgd_idx, pmd_idx, pte_idx;
	mmu_pgd_t *pgd;
	mmu_pmd_t *pmd;
	mmu_pte_t *pte;

	// Actually, this is unnecessary
	virt_addr = virt_addr & (~MMU_PAGE_MASK);

	pgd = mmu_get_root(ctx);

	vmem_get_idx_from_vaddr(virt_addr, &pgd_idx, &pmd_idx, &pte_idx);

	if (!mmu_pgd_present(pgd + pgd_idx)) {
		return -ENOENT;
	}

	pmd = mmu_pgd_value(pgd + pgd_idx);

	if (!mmu_pmd_present(pmd + pmd_idx)) {
		return -ENOENT;
	}

	pte = mmu_pmd_value(pmd + pmd_idx);

	if (!mmu_pte_present(pte + pte_idx)) {
		return -ENOENT;
	}

	vmem_set_pte_flags(pte + pte_idx, flags);

	return ENOERR;
}

/**
 * @brief Change PTE attributes for given memory range: [virt_addr, virt_addr + len]
 *
 * @return Negative error value, zero if succeeded
 */
int vmem_set_flags(mmu_ctx_t ctx, mmu_vaddr_t virt_addr, size_t len, vmem_page_flags_t flags) {
	int ret;

	while (len > 0) {
		if ((ret = vmem_page_set_flags(ctx, virt_addr, flags))) {
			return ret;
		}

		virt_addr += MMU_PAGE_SIZE;
		len -= MMU_PAGE_SIZE;
	}

	return 0;
}

static void vmem_set_pte_flags(mmu_pte_t *pte, vmem_page_flags_t flags) {
	mmu_pte_set_writable(pte, flags & VMEM_PAGE_WRITABLE);
	mmu_pte_set_executable(pte, flags & VMEM_PAGE_EXECUTABLE);
	mmu_pte_set_cacheable(pte, flags & VMEM_PAGE_CACHEABLE);
	mmu_pte_set_usermode(pte, flags & VMEM_PAGE_USERMODE);
}

mmu_paddr_t vmem_translate(mmu_ctx_t ctx, mmu_vaddr_t virt_addr) {
	size_t pgd_idx, pmd_idx, pte_idx;
	mmu_pgd_t *pgd;
	mmu_pmd_t *pmd;
	mmu_pte_t *pte;

	pgd = mmu_get_root(ctx);

	vmem_get_idx_from_vaddr(virt_addr, &pgd_idx, &pmd_idx, &pte_idx);

	if (!mmu_pgd_present(pgd + pgd_idx)) {
		return 0;
	}

	pmd = mmu_pgd_value(pgd + pgd_idx);

	if (!mmu_pmd_present(pmd + pmd_idx)) {
		return 0;
	}

	pte = mmu_pmd_value(pmd + pmd_idx);

	if (!mmu_pte_present(pte + pte_idx)) {
		return 0;
	}

	return mmu_pte_value(pte + pte_idx) + (virt_addr & MMU_PAGE_MASK);
}

#define GET_PMD(pmd, pgd) \
	if (!mmu_pgd_present(pgd)) { \
		pmd = vmem_alloc_pmd_table(); \
		if (pmd == NULL) { \
			log_error("can't alloc pmd"); \
			return -ENOMEM; \
		} \
		mmu_pgd_set(pgd, pmd); \
	} else { \
		pmd = mmu_pgd_value(pgd); \
	}

#define GET_PTE(pte, pmd) \
	if (MMU_PMD_SHIFT != MMU_PTE_SHIFT && !mmu_pmd_present(pmd)) { \
		pte = vmem_alloc_pte_table(); \
		if (pte == NULL) { \
			log_error("can't alloc pte"); \
			return -ENOMEM; \
		} \
		mmu_pmd_set(pmd, pte); \
	} else { \
		pte = mmu_pmd_value(pmd); \
	}


static int do_map_region(mmu_ctx_t ctx, mmu_paddr_t phy_addr, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags) {
	mmu_pgd_t *pgd;
	mmu_pmd_t *pmd;
	mmu_pte_t *pte;
	mmu_paddr_t p_end = phy_addr + reg_size;
	size_t pgd_idx, pmd_idx, pte_idx;

	log_debug("ctx=%x phy_addr=%x virt_addr=%x reg_size=%x flags=%x",
			ctx, phy_addr, virt_addr, reg_size, flags);

	/* Considering that all boundaries are already aligned */
	assert(!(virt_addr & MMU_PAGE_MASK));
	assert(!(phy_addr  & MMU_PAGE_MASK));
	assert(!(reg_size  & MMU_PAGE_MASK));

	pgd = mmu_get_root(ctx);

	vmem_get_idx_from_vaddr(virt_addr, &pgd_idx, &pmd_idx, &pte_idx);

	for ( ; pgd_idx < MMU_PGD_ENTRIES; pgd_idx++) {
		GET_PMD(pmd, pgd + pgd_idx);

		for ( ; pmd_idx < MMU_PMD_ENTRIES; pmd_idx++) {
			GET_PTE(pte, pmd + pmd_idx);

			for ( ; pte_idx < MMU_PTE_ENTRIES; pte_idx++) {
				/* Considering that address has not mapped yet */
				assert(!mmu_pte_present(pte + pte_idx));

				mmu_pte_set(pte + pte_idx, phy_addr);
				vmem_set_pte_flags(pte + pte_idx, flags);

				phy_addr  += MMU_PAGE_SIZE;

				if (phy_addr >= p_end) {
					return ENOERR;
				}
			}

			pte_idx = 0;
		}

		pmd_idx = 0;
	}

	log_error("didn't mapped ctx=%x vaddr=%x size=%x flags=%x",
			ctx, virt_addr, reg_size, flags);

	return -EINVAL;
}

static int do_create_space(mmu_ctx_t ctx, mmu_vaddr_t virt_addr, size_t reg_size, vmem_page_flags_t flags) {
	mmu_pgd_t *pgd;
	mmu_pmd_t *pmd;
	mmu_pte_t *pte;
	mmu_paddr_t v_end = virt_addr + reg_size;
	size_t pgd_idx, pmd_idx, pte_idx;
	void *addr;

	/* Considering that all boundaries are already aligned */
	assert(!(virt_addr & MMU_PAGE_MASK));
	assert(!(reg_size  & MMU_PAGE_MASK));

	pgd = mmu_get_root(ctx);

	vmem_get_idx_from_vaddr(virt_addr, &pgd_idx, &pmd_idx, &pte_idx);

	for ( ; pgd_idx < MMU_PGD_ENTRIES; pgd_idx++) {
		GET_PMD(pmd, pgd + pgd_idx);

		for ( ; pmd_idx < MMU_PMD_ENTRIES; pmd_idx++) {
			GET_PTE(pte, pmd + pmd_idx);

			for ( ; pte_idx < MMU_PTE_ENTRIES; pte_idx++) {
				/* Considering that space has not allocated yet */
				assert(!mmu_pte_present(pte + pte_idx));

				if (!(addr = vmem_alloc_page())) {
					return -ENOMEM;
				}

				mmu_pte_set(pte + pte_idx, (mmu_paddr_t) addr);
				vmem_set_pte_flags(pte + pte_idx, flags);

				virt_addr += MMU_PAGE_SIZE;

				if (virt_addr >= v_end) {
					return ENOERR;
				}
			}

			pte_idx = 0;
		}

		pmd_idx = 0;
	}

	return -EINVAL;
}
