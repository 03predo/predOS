```
plink.exe -serial COM6 -sercfg 3000000
```
```
hexdump -C build/img/predOS.img | head
```
```
mcopy -v -i build/img/fat readme.md ::readme.md
```
```
dd bs=512 if="build/img/mbr" of="build/img/predOS.img" count=2048
```
```
cat build/img/fat >> build/img/predOS.img
```