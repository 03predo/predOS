import serial
import time
import math
from dotenv import dotenv_values

config = dotenv_values("scripts/.env")
SERIAL_PORT = config["SERIAL_PORT"]
BAUDRATE = int(config["BAUDRATE"])

print("serial port: " + SERIAL_PORT + ", baud: " + str(BAUDRATE))

