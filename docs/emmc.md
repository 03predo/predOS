# Embedded Multi Media Controller

## EMMC Initialization Procedure
### 1. Reset EMMC
Reset the complete host circuit and disable the SD clock and the EMMC internal clock by setting the corresponding bits in the CONTROL1 register of the EMMC. Wait one second before checking that reset is successful.

> **Note:** this process is not documented anywhere, but taken from the rpi-boot EMMC driver implementation

### 2. Unmask Interrupts
Unmask interrupts so the corresponding bits are set in the INTERRUPT register of the EMMC when the events occur.

**TODO:** update this section to detail which interrupts we enable (ie. let the ISR handle)
### 3. Validate Card Inserted
Validate that a card is inserted by checking the corresponding bit in the STATUS register of the EMMC. 

> **Note:** this card inserted bit is marked as reserved in the BCM2835 peripherals datasheet but its position is documented in the SD Host Controller Spec 2.2.9

### 4. Enable Clock
The clock supply sequence is documented in the SD Host Controller Spec 3.2.1. It outlines the following steps:
1. **Calculate a Divisor:** The divisor will determine the frequency of the SD clock. We assume the base SD clock frequency is 100MHz. Our target frequency is 400kHz which is the identification frequency (Physical Layer Spec 4.2.1), we need to operate at this frequency for some of the following steps in the initialization process, thus our divider is 250(or 0xFA in hex).
2. **Set Internal Clock Enable and Clock Divisor**: Write to both of these fields in the CONTROL1 register of the EMMC to start the internal clock.
3. **Check Internal Clock Stable**: Check the clock stable field in the CONTROL1 register until it indicates the internal clock is stable.
4. **Set SD Clock Enable**: Setting this field in CONTROL1 will supply the SD Clock to the card.

> **Note:** The assumption that the base SD clock frequency is 100MHz comes from the rpi-boot implementation of the EMMC driver. Usually you would be able to check the Capabilities register for the base clock value but the BCM2835 does not include this register in the datasheet

### 5. Card Reset
The card reset step is documented in the Physical Layer Spec 4.2.1. To reset all cards we send the GO_IDLE_STATE command (CMD0). This tells each card to go into the idle state, in the idle state each card has a default driver strength, and 400kHz clock frequency.

### 6. Operating Condition Validation
This step is documented in the Physical Layer Spec 4.2.2. This step ensures that the card is compatible with the Host Controller. The first thing we do in this step is send the SEND_IF_COND command (CMD8), in the argument of this command we send the current supplied voltage and a check pattern. On response to this command we check the check pattern in the response to ensure it is the same as the one we sent. Receipt of this command lets the card know that we support a certain version of the physical layer spec and it can enable any relevant features accordingly.

The next step is to send the SD_SEND_OP_COND command (ACMD41), in the argument of this command we send the desired operating voltage range to the card. If the card is compatible with this range it will respond with its OCR (operating condition register). The card will then begin its power up process, the status of the card power up is indicated in the OCR so we continuously poll the card by sending SD_SEND_OP_COND until power up is complete.

### 7. Card Identification
This step is documented in the Physical Layer Spec 4.2.3. This step allows us to identify the card and assigns the card it's relative card address which is used in subsequent commands. First we send the ALL_SEND_CID command (CMD2), this will return the Card Identification Register(CID), which contains information about the card (manufacturer, product name, serial number).

After this command we send the SEND_RELATIVE_ADDR command (CMD3) which will return the cards relative card address (RCA), the RCA is useful when you can have multiple cards accessed by the same host controller. The SEND_RELATIVE_ADDR command also transitions the card into the standby state which means it is ready for data transfers, this means we can set the clock frequency up to 25MHz which is the max frequency for microSDXC cards(ie. the card im using).



## EMMC Data Transfer Procedure


## References
BCM2835 Peripheral Sheet
```
https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf
```

Download the simplified SD specs
- Part1_Physical_Layer_Simplified_Specification_Ver3.01.pdf 
- PartA2_SD Host_Controller_Simplified_Specification_Ver3.00.pdf 
From this website
```
https://www.sdcard.org/downloads/pls/archives/
```

Here is another implementation of the emmc driver
```
https://github.com/jncronin/rpi-boot/blob/master/emmc.c
```

