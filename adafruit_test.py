import board
import busio
import digitalio

from adafruit_mcp230xx.mcp23017 import MCP23017
import time


i2c = busio.I2C(board.SCL, board.SDA)
mcp = MCP23017(i2c)

pin0 = mcp.get_pin(0)

pin0.switch_to_output(value=True)

while True:
    pin0.value = True
    time.sleep(0.5)
    pin0.value = False
    time.sleep(0.5)