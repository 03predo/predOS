# TODO

Main goal is to get mmu up and running. Executables have a default addr of 0x8000, so we want our kernel code to operate in the 0x8000000 addr range so an app can still access sys calls. 

Need to create linker file to place small bootloader at 0x8000 which will setup mmu, then we can call functions that use the 0x8000000 addr range once the mmu is setup. 


```
plink.exe -serial COM6 -sercfg 115200
```
