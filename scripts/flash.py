import serial
import time
from dotenv import dotenv_values

config = dotenv_values("scripts/.env")
SERIAL_PORT = config["SERIAL_PORT"]
BAUDRATE = int(config["BAUDRATE"])
IMG_FILE = config["IMG_FILE"]


file = open(IMG_FILE, "rb")
buf = bytearray(file.read())
num_blocks = int(len(buf) / 512)
print("Number of blocks: " + str(num_blocks))
print(num_blocks.to_bytes(4, 'little'))

ser = serial.Serial(timeout=10)
ser.port = SERIAL_PORT
ser.baudrate = BAUDRATE
ser.open()

ser.write(b'L\r')
print(ser.readline().decode('utf-8'), end="") # L
print(ser.readline().decode('utf-8'), end="") # ready to receive image

b = ser.write(num_blocks.to_bytes(4, 'little'))
assert(b == 4)
print(ser.readline().decode('utf-8'), end="")

for x in range(num_blocks):
    print("sent " + str(x) + " blocks", end="\r")
    b = ser.write(buf[(512*x):(512*(x+1))])
    ser.readline().decode('utf-8')

while(1):
    log = ser.readline().decode('utf-8')
    if(log.find('\n') == -1):
        print("timeout")
        break
    print(log, end="")

