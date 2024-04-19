# Embedded Multi Media Controller

The EMMC registers loosely map to register in the SD Host Controller spec

## Block Size Register
- Set the size of each block for a given transfer

## Block Count Register
- Set the number of blocks for a given transfer

## Argument Register
- Specified as bit39-8 0f Command-Format in the Physical Layer Spec(need to see if this is important)

## Transfer Mode Register
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

## Command Register
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

# Response Register
- Provides command response based on mapping in spec

