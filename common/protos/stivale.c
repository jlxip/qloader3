#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <protos/stivale.h>
#include <lib/libc.h>
#include <lib/elf.h>
#include <lib/misc.h>
#include <lib/acpi.h>
#include <lib/config.h>
#include <lib/time.h>
#include <lib/print.h>
#include <lib/real.h>
#include <lib/uri.h>
#include <lib/fb.h>
#include <lib/term.h>
#include <sys/pic.h>
#include <sys/cpu.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/lapic.h>
#include <fs/file.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <stivale.h>
#include <drivers/vga_textmode.h>
#include <drivers/gop.h>

#define REPORTED_ADDR(PTR) \
    ((PTR) + ((stivale_hdr.flags & (1 << 3)) ? \
    direct_map_offset : 0))

bool stivale_load_by_anchor(void **_anchor, const char *magic,
                            uint8_t *file, uint64_t filesize) {
    struct stivale_anchor *anchor = NULL;
    size_t magiclen = strlen(magic);
    for (size_t i = 0; i < filesize; i += 16) {
        if (memcmp(file + i, magic, magiclen) == 0) {
            anchor = (void *)(file + i);
        }
    }
    if (anchor == NULL) {
        return false;
    }

    memmap_alloc_range(anchor->phys_load_addr, filesize, MEMMAP_KERNEL_AND_MODULES,
                       true, true, false, false);
    memcpy((void *)(uintptr_t)anchor->phys_load_addr, file, filesize);

    size_t bss_size = anchor->phys_bss_end - anchor->phys_bss_start;
    memmap_alloc_range(anchor->phys_bss_start, bss_size, MEMMAP_KERNEL_AND_MODULES,
                       true, true, false, false);
    memset((void *)(uintptr_t)anchor->phys_bss_start, 0, bss_size);

    *_anchor = anchor;

    return true;
}


pagemap_t stivale_build_pagemap(bool level5pg, bool nx, bool unmap_null, struct elf_range *ranges, size_t ranges_count,
                                bool want_fully_virtual, uint64_t physical_base, uint64_t virtual_base,
                                uint64_t direct_map_offset) {
    pagemap_t pagemap = new_pagemap(level5pg ? 5 : 4);

    if (ranges_count == 0) {
        // Map 0 to 2GiB at 0xffffffff80000000
        for (uint64_t i = 0; i < 0x80000000; i += 0x40000000) {
            map_page(pagemap, 0xffffffff80000000 + i, i, 0x03, Size1GiB);
        }
    } else {
        for (size_t i = 0; i < ranges_count; i++) {
            uint64_t virt = ranges[i].base;
            uint64_t phys;

            if (virt & ((uint64_t)1 << 63)) {
                if (want_fully_virtual) {
                    phys = physical_base + (virt - virtual_base);
                } else {
                    phys = virt - FIXED_HIGHER_HALF_OFFSET_64;
                }
            } else {
                panic(false, "stivale2: Protected memory ranges are only supported for higher half kernels");
            }

            uint64_t pf = VMM_FLAG_PRESENT |
                (ranges[i].permissions & ELF_PF_X ? 0 : (nx ? VMM_FLAG_NOEXEC : 0)) |
                (ranges[i].permissions & ELF_PF_W ? VMM_FLAG_WRITE : 0);

            for (uint64_t j = 0; j < ranges[i].length; j += 0x1000) {
                map_page(pagemap, virt + j, phys + j, pf, Size4KiB);
            }
        }
    }

    // Sub 2MiB mappings
    for (uint64_t i = 0; i < 0x200000; i += 0x1000) {
        if (!(i == 0 && unmap_null))
            map_page(pagemap, i, i, 0x03, Size4KiB);
        map_page(pagemap, direct_map_offset + i, i, 0x03, Size4KiB);
    }

    // Map 2MiB to 4GiB at higher half base and 0
    //
    // NOTE: We cannot just directly map from 2MiB to 4GiB with 1GiB
    // pages because if you do the math.
    //
    //     start = 0x200000
    //     end   = 0x40000000
    //
    //     pages_required = (end - start) / (4096 * 512 * 512)
    //
    // So we map 2MiB to 1GiB with 2MiB pages and then map the rest
    // with 1GiB pages :^)
    for (uint64_t i = 0x200000; i < 0x40000000; i += 0x200000) {
        map_page(pagemap, i, i, 0x03, Size2MiB);
        map_page(pagemap, direct_map_offset + i, i, 0x03, Size2MiB);
    }

    for (uint64_t i = 0x40000000; i < 0x100000000; i += 0x40000000) {
        map_page(pagemap, i, i, 0x03, Size1GiB);
        map_page(pagemap, direct_map_offset + i, i, 0x03, Size1GiB);
    }

    size_t _memmap_entries = memmap_entries;
    struct memmap_entry *_memmap =
        ext_mem_alloc(_memmap_entries * sizeof(struct memmap_entry));
    for (size_t i = 0; i < _memmap_entries; i++)
        _memmap[i] = memmap[i];

    // Map any other region of memory from the memmap
    for (size_t i = 0; i < _memmap_entries; i++) {
        uint64_t base   = _memmap[i].base;
        uint64_t length = _memmap[i].length;
        uint64_t top    = base + length;

        if (base < 0x100000000)
            base = 0x100000000;

        if (base >= top)
            continue;

        uint64_t aligned_base   = ALIGN_DOWN(base, 0x40000000);
        uint64_t aligned_top    = ALIGN_UP(top, 0x40000000);
        uint64_t aligned_length = aligned_top - aligned_base;

        for (uint64_t j = 0; j < aligned_length; j += 0x40000000) {
            uint64_t page = aligned_base + j;
            map_page(pagemap, page, page, 0x03, Size1GiB);
            map_page(pagemap, direct_map_offset + page, page, 0x03, Size1GiB);
        }
    }

    return pagemap;
}

noreturn void stivale_spinup_32(
                 int bits, bool level5pg, uint32_t pagemap_top_lv,
                 uint32_t entry_point_lo, uint32_t entry_point_hi,
                 uint32_t stivale_struct_lo, uint32_t stivale_struct_hi,
                 uint32_t stack_lo, uint32_t stack_hi, uint32_t local_gdt);

noreturn void stivale_spinup(
                 int bits, bool level5pg, pagemap_t *pagemap,
                 uint64_t entry_point, uint64_t _stivale_struct, uint64_t stack,
                 bool enable_nx, bool wp, uint32_t local_gdt) {
#if defined (BIOS)
    if (bits == 64) {
        // If we're going 64, we might as well call this BIOS interrupt
        // to tell the BIOS that we are entering Long Mode, since it is in
        // the specification.
        struct rm_regs r = {0};
        r.eax = 0xec00;
        r.ebx = 0x02;   // Long mode only
        rm_int(0x15, &r, &r);
    }
#endif

    if (enable_nx) {
        vmm_assert_nx();
    }

    pic_mask_all();
    io_apic_mask_all();

    irq_flush_type = IRQ_PIC_APIC_FLUSH;

    common_spinup(stivale_spinup_32, 12,
        bits, level5pg, enable_nx, wp, (uint32_t)(uintptr_t)pagemap->top_level,
        (uint32_t)entry_point, (uint32_t)(entry_point >> 32),
        (uint32_t)_stivale_struct, (uint32_t)(_stivale_struct >> 32),
        (uint32_t)stack, (uint32_t)(stack >> 32), local_gdt);
}
