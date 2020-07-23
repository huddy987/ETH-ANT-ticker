from binance import Binance_API
from time import sleep
from datetime import datetime, time
from array import array
from math import modf
import random
import struct

from ant.easy.node import Node
from ant.easy.channel import Channel
from ant.base.message import Message

debug = False
demo = True

# ANT channel defines (mapped to the same as in the client)
ANT_CHANNEL_TYPE       =     0x10 # 0x10 is bidirectional transmit
ANT_CHANNEL_NUM        =     0
ANT_CHANNEL_RF_FREQ    =     67
ANT_TRANSMISSION_TYPE  =     1
ANT_DEVICE_TYPE        =     1
ANT_DEVICE_NUMBER      =     2020
ANT_PERIOD             =     8192 # 4 Hz
ANT_NETWORK_NUMBER     =     0
ANT_NETWORK_KEY        =     [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

binance = Binance_API()

node = Node()
node.set_network_key(ANT_NETWORK_NUMBER, [0x00])
channel = node.new_channel(Channel.Type.BIDIRECTIONAL_TRANSMIT)

# Used for tracking when to update the bcst buffer
current_time = 0

if (not demo):
    CANDLESTICK_DELAY = 300 # 5 minutes
    last_eth_price = 0
else:
    CANDLESTICK_DELAY = 5 # 5 seconds
    last_eth_price = 258.58

def get_ETH_buffer():
    """
    Gets the current ETH price from Binance and stores it in broadcast buffer.
    Format:
    LSB 0 0 0 <cents price byte> <dollar price byte1> <dollar price byte2> <dollar price byte3> <MSB dollar price byte> MSB
    """
    global last_eth_price
    # Interesting: https://stackoverflow.com/questions/35847673/fast-way-to-split-an-int-into-bytes
    int_to_four_bytes = struct.Struct('<I').pack

    if (not demo):
        current_eth_price = binance.get_ETH_price()

    else: # Spoof some data for demo
        current_eth_price = last_eth_price
        price_offset = random.uniform(0, 100)
        if(random.randint(0, 1)):
            current_eth_price += price_offset
        else:
            current_eth_price -= price_offset

    # Create the bcst buffer and fill it
    cents_part, dollar_part = modf(current_eth_price)
    cents_part = int(cents_part * 100) # Convert cents to int
    byte1, byte2, byte3, msb = 0, 0, 0, 0
    byte1, byte2, byte3, msb = int_to_four_bytes(int(dollar_part) & 0xFFFFFFFF)
    bcst_buffer = array('B', [0, 0, 0, cents_part, byte1, byte2, byte3, msb])

    # Print some stats
    print("The last ETH price was: " + str(last_eth_price))
    print("The current ETH price is: " + str(current_eth_price))
    last_eth_price = current_eth_price

    if (debug):
        print(bcst_buffer)

    return bcst_buffer

def on_tx_event(data):
    """
    Callback function for ANT
    Upon each TX time slot, a TX event is generated, and this function is called.
    Calculate the time between ticks, and add the time. If the time has reached the next
    candlestick, process the next event
    """
    global current_time, channel, node
    current_time += (ANT_PERIOD / (32768))

    if(current_time >= CANDLESTICK_DELAY):
        current_time = 0
        newBuffer = get_ETH_buffer()
        # Update the TX buffer
        channel.send_broadcast_data(newBuffer)
    else:
        pass

def main():
    # Configure ANT
    channel.on_TX_event = on_tx_event
    channel.set_period(ANT_PERIOD)
    channel.set_rf_freq(ANT_CHANNEL_RF_FREQ)
    channel.set_id(ANT_DEVICE_NUMBER, ANT_DEVICE_TYPE, ANT_TRANSMISSION_TYPE)
    try:
        channel.open()
        node.start()
    finally:
        node.stop()

if __name__ == "__main__":
    main()