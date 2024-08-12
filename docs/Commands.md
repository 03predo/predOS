```
git submodule update --init --recursive
```
```
plink.exe -serial COM6 -sercfg 115200
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
```
docker build --tag <name>:<tag> .
```
```
docker tag <name>:<tag> <username>/<repo>:<tag>
```
```
docker push <username>/<repo>:<tag>
```
```
docker pull <username>/<repo>:<tag>
```
```
docker run -id -v $PWD/:/root/predOS --name predos 03predo/predos:v1.1
```
```
docker exec -it predos bash
```

```
uint8_t* buf = (uint8_t*)(pcb.text_frame);
for(uint32_t i = 0x8000; i < 0x1500c; ++i){
  if((i % 16) == 0){
    LOGI("\n0x%08x", buf + i);
  }
  if((i % 8) == 0){
    LOGI(" ");
  }
  LOGI("0x%02x ", buf[i]);
}
```
