#ifndef __INCLUDE_X86_64_H__
#define __INCLUDE_X86_64_H__

#include "pts_api.h"

// ————————————————————————————— Compatibility —————————————————————————————— //
#define _AT(T, X) ((T)(X))
#define __AC(X, Y) (X##Y)
#define _AC(X, Y) __AC(X, Y)
#typedef entry_t pteval_t;

// Copied from linux/arch/x86/include/asm/pgtable_types.h
// Add whatever yoou might need.
// ——————————————————————————— Page Configuration ——————————————————————————— //
/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT 12
#define PAGE_SIZE (_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

/*
 * PGDIR_SHIFT determines what a top-level page table entry can map
 */
#define PGDIR_SHIFT 39
#define PTRS_PER_PGD 512
#define MAX_PTRS_PER_P4D 1

#endif /* CONFIG_X86_5LEVEL */

/*
 * 3rd level page
 */
#define PUD_SHIFT 30
#define PTRS_PER_PUD 512

/*
 * PMD_SHIFT determines the size of the area a middle-level
 * page table can map
 */
#define PMD_SHIFT 21
#define PTRS_PER_PMD 512

#define PMD_PAGE_SIZE (_AC(1, UL) << PMD_SHIFT)
#define PMD_PAGE_MASK (~(PMD_PAGE_SIZE - 1))

#define PUD_PAGE_SIZE (_AC(1, UL) << PUD_SHIFT)
#define PUD_PAGE_MASK (~(PUD_PAGE_SIZE - 1))

#define __VIRTUAL_MASK ((1UL << __VIRTUAL_MASK_SHIFT) - 1)

/* Cast *PAGE_MASK to a signed type so that it is sign-extended if
   virtual addresses are 32-bits but physical addresses are larger
   (ie, 32-bit PAE). */
#define PHYSICAL_PAGE_MASK (((signed long)PAGE_MASK) & __PHYSICAL_MASK)
#define PHYSICAL_PMD_PAGE_MASK (((signed long)PMD_PAGE_MASK) & __PHYSICAL_MASK)
#define PHYSICAL_PUD_PAGE_MASK (((signed long)PUD_PAGE_MASK) & __PHYSICAL_MASK)

#define HPAGE_SHIFT PMD_SHIFT
#define HPAGE_SIZE (_AC(1, UL) << HPAGE_SHIFT)
#define HPAGE_MASK (~(HPAGE_SIZE - 1))
#define HUGETLB_PAGE_ORDER (HPAGE_SHIFT - PAGE_SHIFT)

#define HUGE_MAX_HSTATE 2

// ——————————————————————————— Page bits for flags —————————————————————————— //

#define _PAGE_BIT_PRESENT 0    /* is present */
#define _PAGE_BIT_RW 1         /* writeable */
#define _PAGE_BIT_USER 2       /* userspace addressable */
#define _PAGE_BIT_PWT 3        /* page write through */
#define _PAGE_BIT_PCD 4        /* page cache disabled */
#define _PAGE_BIT_ACCESSED 5   /* was accessed (raised by CPU) */
#define _PAGE_BIT_DIRTY 6      /* was written to (raised by CPU) */
#define _PAGE_BIT_PSE 7        /* 4 MB (or 2MB) page */
#define _PAGE_BIT_PAT 7        /* on 4KB pages */
#define _PAGE_BIT_GLOBAL 8     /* Global TLB entry PPro+ */
#define _PAGE_BIT_SOFTW1 9     /* available for programmer */
#define _PAGE_BIT_SOFTW2 10    /* " */
#define _PAGE_BIT_SOFTW3 11    /* " */
#define _PAGE_BIT_PAT_LARGE 12 /* On 2MB or 1GB pages */
#define _PAGE_BIT_SOFTW4 58    /* available for programmer */
#define _PAGE_BIT_PKEY_BIT0 59 /* Protection Keys, bit 1/4 */
#define _PAGE_BIT_PKEY_BIT1 60 /* Protection Keys, bit 2/4 */
#define _PAGE_BIT_PKEY_BIT2 61 /* Protection Keys, bit 3/4 */
#define _PAGE_BIT_PKEY_BIT3 62 /* Protection Keys, bit 4/4 */
#define _PAGE_BIT_NX 63        /* No execute: only valid after cpuid check */

#define _PAGE_BIT_SPECIAL _PAGE_BIT_SOFTW1
#define _PAGE_BIT_CPA_TEST _PAGE_BIT_SOFTW1
#define _PAGE_BIT_UFFD_WP _PAGE_BIT_SOFTW2    /* userfaultfd wrprotected */
#define _PAGE_BIT_SOFT_DIRTY _PAGE_BIT_SOFTW3 /* software dirty tracking */
#define _PAGE_BIT_DEVMAP _PAGE_BIT_SOFTW4

/* If _PAGE_BIT_PRESENT is clear, we use these: */
/* - if the user mapped it with PROT_NONE; pte_present gives true */
#define _PAGE_BIT_PROTNONE _PAGE_BIT_GLOBAL

// —————————————————————————————— Actual Flags —————————————————————————————— //

#define _PAGE_PRESENT (_AT(pteval_t, 1) << _PAGE_BIT_PRESENT)
#define _PAGE_RW (_AT(pteval_t, 1) << _PAGE_BIT_RW)
#define _PAGE_USER (_AT(pteval_t, 1) << _PAGE_BIT_USER)
#define _PAGE_PWT (_AT(pteval_t, 1) << _PAGE_BIT_PWT)
#define _PAGE_PCD (_AT(pteval_t, 1) << _PAGE_BIT_PCD)
#define _PAGE_ACCESSED (_AT(pteval_t, 1) << _PAGE_BIT_ACCESSED)
#define _PAGE_DIRTY (_AT(pteval_t, 1) << _PAGE_BIT_DIRTY)
#define _PAGE_PSE (_AT(pteval_t, 1) << _PAGE_BIT_PSE)
#define _PAGE_GLOBAL (_AT(pteval_t, 1) << _PAGE_BIT_GLOBAL)
#define _PAGE_SOFTW1 (_AT(pteval_t, 1) << _PAGE_BIT_SOFTW1)
#define _PAGE_SOFTW2 (_AT(pteval_t, 1) << _PAGE_BIT_SOFTW2)
#define _PAGE_SOFTW3 (_AT(pteval_t, 1) << _PAGE_BIT_SOFTW3)
#define _PAGE_PAT (_AT(pteval_t, 1) << _PAGE_BIT_PAT)
#define _PAGE_PAT_LARGE (_AT(pteval_t, 1) << _PAGE_BIT_PAT_LARGE)
#define _PAGE_SPECIAL (_AT(pteval_t, 1) << _PAGE_BIT_SPECIAL)
#define _PAGE_CPA_TEST (_AT(pteval_t, 1) << _PAGE_BIT_CPA_TEST)
#define _PAGE_PKEY_BIT0 (_AT(pteval_t, 1) << _PAGE_BIT_PKEY_BIT0)
#define _PAGE_PKEY_BIT1 (_AT(pteval_t, 1) << _PAGE_BIT_PKEY_BIT1)
#define _PAGE_PKEY_BIT2 (_AT(pteval_t, 1) << _PAGE_BIT_PKEY_BIT2)
#define _PAGE_PKEY_BIT3 (_AT(pteval_t, 1) << _PAGE_BIT_PKEY_BIT3)

// ——————————————————————— Short Versions from Linux ———————————————————————— //

#define __PP _PAGE_PRESENT
#define __RW _PAGE_RW
#define _USR _PAGE_USER
#define ___A _PAGE_ACCESSED
#define ___D _PAGE_DIRTY
#define ___G _PAGE_GLOBAL
#define __NX _PAGE_NX

#define __pg(x) _AT(pteval_t, x)

#define PAGE_NONE __pg(0 | 0 | 0 | ___A | 0 | 0 | 0 | ___G)
#define PAGE_SHARED __pg(__PP | __RW | _USR | ___A | __NX | 0 | 0 | 0)
#define PAGE_SHARED_EXEC __pg(__PP | __RW | _USR | ___A | 0 | 0 | 0 | 0)
#define PAGE_COPY_NOEXEC __pg(__PP | 0 | _USR | ___A | __NX | 0 | 0 | 0)
#define PAGE_COPY_EXEC __pg(__PP | 0 | _USR | ___A | 0 | 0 | 0 | 0)
#define PAGE_COPY __pg(__PP | 0 | _USR | ___A | __NX | 0 | 0 | 0)
#define PAGE_READONLY __pg(__PP | 0 | _USR | ___A | __NX | 0 | 0 | 0)
#define PAGE_READONLY_EXEC __pg(__PP | 0 | _USR | ___A | 0 | 0 | 0 | 0)

// ————————————————————————————————— Masks —————————————————————————————————— //
/* Extracts the PFN from a (pte|pmd|pud|pgd)val_t of a 4KB page */
#define PTE_PFN_MASK ((pteval_t)PHYSICAL_PAGE_MASK)

/*
 *  Extracts the flags from a (pte|pmd|pud|pgd)val_t
 *  This includes the protection key value.
 */
#define PTE_FLAGS_MASK (~PTE_PFN_MASK)
// ——————————————————————————————— Functions ———————————————————————————————— //

callback_t x86_64_how(entry_t entry, level_t level, pt_profile_t* profile);

index_t x86_64_get_index(addr_t addr, level_t level);

entry_t x86_64_next(entry_t entry, level_t curr_level);
