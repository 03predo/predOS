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

todo:
docker container use latest nvimSetup
add git config --global core.autocrlf true