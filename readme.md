# PredOS
Pull down dev environment image
```
docker pull 03predo/predos:v1.0
```

Start container
```
docker run --rm -id -v $PWD/:/root/predOS --name predos 03predo/predos:v1.0 
```

Enter container
```
docker exec -it predos bash
```