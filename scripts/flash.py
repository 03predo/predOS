import serial
import time
import math
from dotenv import dotenv_values

def serial_log(ser):
    log = ser.readline().decode('utf-8')
    if(log.find('\n') == -1):
        print("timeout")
        exit
    print(log, end="")

def flash():
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

    ser = serial.Serial(timeout=5)
    ser.port = SERIAL_PORT
    ser.baudrate = BAUDRATE
    ser.open()

    ser.write(b'L\r') # load command
    serial_log(ser) # L
    serial_log(ser) # ready to receive image
    assert(ser.write(num_blocks.to_bytes(4, 'little')) == 4)
    serial_log(ser) # image size is

    for x in range(num_blocks):
        if((x % 512) == 0):
            print("sent " + str(x) + " blocks", end="\r")

        b = ser.write(buf[(512*x):(512*(x+1))])
        log = ser.readline().decode('utf-8')
        if(log.find('\n') == -1):
            print("timeout")
            exit

    print(ser.readline().decode('utf-8'), end="") # write complete
    print(ser.readline().decode('utf-8'), end="") # received image of size
    print("flash complete")


if __name__ == "__main__":
    flash()

