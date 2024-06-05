
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_HYPERVISOR,   0x1a
.equ    CPSR_MODE_UNDEFINED,    0x1b
.equ    CPSR_MODE_SYSTEM,       0x1f
.equ    CPSR_MODE_MASK,         0x1f

.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

.equ    IFSR_DEBUG_EVENT,       0b0010

.equ    DSCR_MONITOR_DEBUG_ENABLE,  0xf
.equ    DSCR_MODE_SELECT,           0xe

.equ    MMU_ENABLE,               (1 << 0)
.equ    DATA_CACHE_ENABLE,        (1 << 2)
.equ    INSTRUCTION_CACHE_ENABLE, (1 << 12)
.equ    HIGH_VECTORS_ENABLE,      (1 << 13)
.equ    SUBPAGES_ENABLE,          (1 << 23)

