# Embedded Multi Media Controller

## EMMC Initialization Procedure
We need to configure the emmc via the control registers

## Control0
- Fields:
    - ALT_BOOT_EN: 0 -> disabled
    - BOOT_EN: 0 -> stop boot mode access
    - SPI_MODE: 0 -> normal mode
    - GAP_IEN: 0 -> disabled
    - READWAIT_EN: 0 -> disabled
    - GAP_RESTART: 0 -> ignore
    - GAP_STOP: 0 -> ignore
    - HCTL_8BIT: 0 -> disabled
    - HCTL_HS_EN: 0 -> disabled
    - HCTL_DWIDTH: 1 -> 4-bit mode (micro sd has 4 data lines)

## Control1
- Fields:
    - SRST_DATA: 0 -> disabled
    - SRST_CMD: 0 -> disabled
    - SRST_HC: 0 -> disabled
    - DATA_TOUNIT: 1111b -> disabled
    - CLK_FREQ8: 0 -> base clock
    - CLK_FREQ_MS2: 0 -> base clock
    - CLK_GENSEL: 0 -> divided
    - CLK_EN: 0 -> enabled
    - CLK_INTLEN: 0 -> disabled

## Control2
- Fields:
    - TUNED: 0 -> no
    - TUNEON: 0 -> no
    - UHS_MODE: 011b -> SDR104

## Read Procedure
Set the block size, block count, arugment, transfer mode and command registers, a write to the command sends the command so must be performed last.

## Block Size
- we will use the max block size 512

## Argument
- For a read the argument register contains the address on the chip, according to the physical layer spec the sdxc cards use 512 block unit addresses, so addr 0x0 corresponds to the first 512 bytes on the sd card. 



## Registers
The EMMC registers loosely map to register in the SD Host Controller spec

### Block Size Register
- Set the size of each block for a given transfer

### Block Count Register
- Set the number of blocks for a given transfer

### Argument Register
- Specified as bit39-8 0f Command-Format in the Physical Layer Spec(need to see if this is important)

### Transfer Mode Register
- Control operation of data transfers. Should be set before issuing a command. 
- Fields:
    - Multi/Single Block Select: bit is set when issuing multiple-block transfer commands
    - Data Transfer Direction Select: 1 indicates read(Card to Host), 0 indicates write(Host to Card)
    - Auto CMD Enable: determines use of auto command functions
        - 00b -> Auto Command Disabled
        - 01b -> Auto CMD12 Enable: Host Controller issues CMD12 automatically when last block transfer is complete
        - 10b -> Audo CMD23 Enable: Host Controller issues CMD23 automatically before issuing a command in the command register
    - Block Counter Enable: enables the block count register
    - DMA Enable: 1 indicates DMA Data transfer, 0 indicates Non DMA data transfer

### Command Register
- Host Driver must check Command Inhibit(DATA) and Commmand Inhibit(CMD) bit in the Present State register before writing to this register. Writing to the upper byte of this register triggers SD command generation. 
- Fields:
    - Command Index: set the command number(CMD0-63, ACMD0-63) that is specified in bits 45-40 of the Command-Format in the physical layer spec.
    - Command Type: set based on whether command is Suspend, Resume, Abort, or regular
    - Data Present Select: 1 indicates data is present and will be transferred using the data line, 0 otherwise
    - Command Index Check Enable: If 1 Host controller checks to see if the response has the same value as the command index
    - Command CRC Check Enable: If 1 Host Controller checks CRC field in the response
    - Response Type Select: 
        - 00b -> No Response
        - 01b -> Response Length 136
        - 10b -> Response length 48
        - 11b Response Length 48 check busy after response

### Response Register
- Provides command response based on mapping in spec


