# Memory Management Unit (MMU)
## Single-Level Page Table
The page table (also called translation table) provides the mappings from virtual to physical addresses, it is easiest to consider a single level page table. The figure below shows how the physical address is generated from the virtual address using the page table.
![[b4_4_section_translation.png]]
The base address of our page table is stored in the Translation Table Base Register (TTBR). The upper bits of the virtual address are used to index the page table. At the index of the page table is a first-level descriptor, which contains the upper bits of the physical address corresponding to the virtual address. The upper bits from the first-level descriptor and the lower bits from the virtual address are combined to create the final physical address.
### Section Descriptor
In a single-level page table the first-level descriptors are called section descriptors, their template is shown in the figure below. This figure also contains the Coarse page table descriptor which is used in a multi-level page table.
![[b4_2_first_level_descriptor_format.png]]
A section is a 1MB region in memory that is described by the various fields in the section descriptor.
#### Memory Attributes (C, B, S, and TEX)
The C, B, and TEX bits describe the attributes of the memory region. They stand for cacheable (C), bufferable (B), shareable (S), and type extension (TEX). The table below shows the encodings for these fields. The S bit is used to indicate shareable for only normal memory type memory.
![[b4_3_cb_tex_encodings.png]]
An in depth description of these memory attributes can be found in the ARMv6 architecture document, but a summary table is shown in the figure below.
![[b2_2_memory_attribute_summary.png]]

#### Memory Access Permissions (AP and APX)
The AP and APX bits describe the access permissions associated with the memory region. The encodings are shown in the table below, the S and R bits are deprecated.
![[b4_1_mmu_access_permissions.png]]


#### Memory Domains (Domain)
The domain field indicates which domain the memory region belongs to. A given process interacts with the access permissions of a memory region based on its domain. Below is a table showing the domain access types.
![[b4_2_domain_access_values.png]]
These domain access types are specified for each of the 16 domain in the Domain Access Control Register. Below is the format of the Domain Access Control.
![[b4_9_4_domain_access_control.png]]
So whenever a memory region is accessed the domain is checked to see what access type the current process has for that specific domain. Then based on the access type we either generate a fault, check the access permissions, or continue without checking the permissions.

#### Other Features (nG, XN, and IMP)
- The nG bit indicates if the memory region is not-global. If the bit is 0 the section will not be associated with a process when in the TLB, and if the bit is 1 it will be associated with the current process (indicated by the current ASID in the Process ID register) when in the TLB.
- The XN bit stands for execute-never and determines if the region is executable (0) or not-executable (1)
-  The IMP bit is not supported by the raspberry pi 1 B+ processor (ARM 1176JZF)
## Enabling the MMU
Perform the following step to enable the MMU
1. Setup the page table in memory and write the base address to the Translation Table Base Register (TTBR). There are two TTBRs, the Translation Table Base Control (TTBC) register determines which one is used on a given memory access. The value in TTBC is a 3 bits wide and is name N. If N is 0 then we always use TTBR0, If N > 0 then if the bits \[31:32-N] of the virtual address are 1 then we use TTBR1 otherwise we use TTBR0. So TTBC should be set based to indicate which addresses will use which TTBR. TTBR0 and TTB1 must be set with their corresponding page table base addresses.
2. Configure the Domain Access Control register to give the correct access types for each of the 16 domains.
3. Set the Context ID register, since the kernel is enabling the MMU we can set this register to be 0.
4. Disable and Invalidate the Instruction cache
5. Disable Subpages, and enable the MMU, then re-enable the Instruction cache.