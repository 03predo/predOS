# File System
## Master Boot Record (MBR)
The MBR indicates how the disc's sectors are divided into partitions. Each partition contains its own file system. The structure of a modern standard MBR is shown below

| Address | Description                        | Size(bytes) |
| ------- | ---------------------------------- | ----------- |
| 0x0000  | Bootstap code area                 | 440         |
| 0x01B8  | 32-bit disk signature              | 4           |
| 0x01BC  | 0x0000 or 0x5A5A if copy-protected | 2           |
| 0x01BE  | Partition Entry 1                  | 16          |
| 0x01CE  | Partition Entry 2                  | 16          |
| 0x01DE  | Partition Entry 3                  | 16          |
| 0x01EE  | Partition Entry 4                  | 16          |
| 0x01FE  | 0x55AA (Boot signature)            | 2           |
This is the layout of one 16-byte partition entry.

| Offset | Field Length | Description                                                                                                   |
| ------ | ------------ | ------------------------------------------------------------------------------------------------------------- |
| 0x00   | 1 byte       | Status or physical drive (0x80 means active or bootable, 0x0 means inactive, and 0x01-0x7F means invalid)     |
| 0x01   | 3 bytes      | CHS address of first absolute sector in partition                                                             |
| 0x04   | 1 byte       | Partition Type (here is a list of all of the [partition types](https://en.wikipedia.org/wiki/Partition_type)) |
| 0x05   | 3 bytes      | CHS address of last absolute sector in partition                                                              |
| 0x08   | 4 bytes      | LBA of first absolute sector in the partition                                                                 |
| 0x0C   | 4 bytes      | Number of sectors in partition                                                                                |
|        |              |                                                                                                               |
Here is a sample of the hexdump from the disc image.
```
000001b0  00 00 00 00 00 00 00 00  1f 1a 81 c4 00 00 80 20
000001c0  21 00 0c 0a 08 02 00 08  00 00 00 78 00 00 00 00
```
The values of the fields are:
- Disk Signature: `0xc4811a1f`
- Partition 1 Status: `0x80`
- Partition 1 CHS First Sector: `0x002120`
- Partition 1 Partition Type: `0x0c` (FAT32)
- Partition 1 CHS Last Sector: `0x02080a`
- Partition 1 LBA First Sector: `0x00000800`
- Partition 1 Number of sectors: `0x00007800`

