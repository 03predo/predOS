import serial
import time
import math
from dotenv import dotenv_values

config = dotenv_values("scripts/.env")
SERIAL_PORT = config["SERIAL_PORT"]
BAUDRATE = int(config["BAUDRATE"])
IMG_FILE = config["IMG_FILE"]

file = open(IMG_FILE, "rb")
buf = bytearray(file.read())

# need to check if prev img was bigger and write that size
last_zero = len(buf)
for x in range(len(buf) - 1, 0, -1):
    if(buf[x] != 0):
        last_zero = x
        break

print("last_zero: " + str(x))
num_blocks = math.ceil((last_zero + 1) / 512)
print("Number of blocks: " + str(num_blocks))
print(num_blocks.to_bytes(4, 'little'))

ser = serial.Serial(timeout=3)
ser.port = SERIAL_PORT
ser.baudrate = BAUDRATE
ser.open()

ser.write(b'L\r') # load command
log = ser.readline().decode('utf-8')
if(log.find('\n') == -1):
    print("timeout")
    exit
print(log, end="") # L

log = ser.readline().decode('utf-8')
if(log.find('\n') == -1):
    print("timeout")
    exit
print(log, end="") # ready to receive image

b = ser.write(num_blocks.to_bytes(4, 'little'))
assert(b == 4)
print(ser.readline().decode('utf-8'), end="") # image size is

for x in range(num_blocks):
    if((x % 512) == 0):
        print("sent " + str(x) + " blocks", end="\r")

    b = ser.write(buf[(512*x):(512*(x+1))])


print(ser.readline().decode('utf-8'), end="") # write complete
print(ser.readline().decode('utf-8'), end="") # received image of size

ser.write(b'R\r') # reset command
print("finished flash")

while(1):
    log = ser.readline().decode('utf-8')
    if(log.find('\n') == -1):
        print("timeout")
        break
    print(log, end="")

