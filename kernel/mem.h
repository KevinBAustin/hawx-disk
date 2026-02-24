#ifndef MEM_H
#define MEM_H

/*
 * Initialize Virtual memory and activate paging.
 * Parameters: None
 * Returns: None
 */
void vm_init(void);

/*
 * Allocate one 4096-byte page of physical memory.
 * Parameters: None
 * Returns:
 *  - A pointer that the kernel can use or 0 if the memory cannot be allocated.
 */
void* vm_page_alloc(void);

/*
 * Free the page of physical memory pointed at by pa, which normally should have
 * been returned by a call to vm_page_alloc().
 * Parameters:
 *  - pa: The physical address of the page to free.
 * Returns: None
 */
void vm_page_free(void *pa);

/*
 * Create an empty page table.
 * Parameters: None
 * Returns:
 *  - A pointer to the new page table or 0 if out of memory.
 */
pagetable_t vm_create_pagetable(void);

/*
 * Look up a virtual address and return the physical address it maps to.
 * Parameters:
 *  - pagetable: The page table to perform the lookup in.
 *  - va: The virtual address to look up.
 * Returns:
 *  - The physical address mapped to the virtual address or 0 if not mapped.
 */
uint64 vm_lookup(pagetable_t pagetable, uint64 va);

/*
 * Insert a mapping into the page table for a virtual address to map to a
 * physical address with the given permissions.
 * Parameters:
 *  - pagetable: The page table to insert the mapping into.
 *  - va: The virtual address to map.
 *  - pa: The physical address to map to.
 *  - perm: The permissions to apply to the mapping.
 * Returns:
 *  - 0 on success or -1 on failure.
 */
int vm_page_insert(pagetable_t pagetable, uint64 va, uint64 pa, int perm);

/*
 * Remove a range of mappings from a page table.
 * Parameters:
 *  - pagetable: The page table to remove the mappings from.
 *  - va: The virtual address to start removing pages.
 *  - npages: The number of pages to remove.
 *  - do_free: If non-zero, also free the physical memory.
 * Returns: None
 */
void vm_page_remove(pagetable_t pagetable, uint64 va, uint64 npages, int do_free);

/*
 * Map a range of virtual memory with the specified permissions.
 * Parameters:
 *  - pagetable: The page table to place the mapping into.
 *  - va: The starting virtual address to map.
 *  - size: The size of the virtual address range to map.
 *  - perm: The permissions to apply to the mapped range.
 * Returns:
 *  - 0 on success or -1 on failure.
 */
int vm_map_range(pagetable_t pagetable, uint64 va, uint64 size, int perm);

/*
 * Run unit tests on the virtual memory system.
 * Parameters: None
 * Returns: None
 */
void vm_test(void);

/* Return the address of the PTE in page table pagetable
 * that corresponds to virtual address va.  If alloc!=0,
 * create any required page-table pages.
 * 
 * The risc-v Sv39 scheme has three levels of page-table
 * pages. A page-table page contains 512 64-bit PTEs.
 * A 64-bit virtual address is split into five fields:
 *   39..63 -- must be zero.
 *   30..38 -- 9 bits of level-2 index.
 *   21..29 -- 9 bits of level-1 index.
 *   12..20 -- 9 bits of level-0 index.
 *    0..11 -- 12 bits of byte offset within the page.
*/
pte_t * walk_pgtable(pagetable_t pagetable, uint64 va, int alloc);

/* 
 * Copy from user to kernel.
 * Copy len bytes to dst from virtual address srcva in a given page table.
 * Return 0 on success, -1 on error.
 */
int vm_copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len); 

/*
 * Copy from kernel to user.
 * Copy len bytes from src to virtual address dstva in a given page table.
 * Return 0 on success, -1 on error.
 */
int vm_copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len);

extern pagetable_t kernel_pagetable;
#endif // MEM_H
