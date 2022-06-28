import serial
import time

from config import SERIAL_PORT
from config import BAUD_RATE
from config import LOCALE


"""
Arduino will need an accompanying script to receive the data from Rpi
Communication has to be two ways, Rpi send, Arduino receive and reply, Rpi receive
"""


class Arduino:
    def __init__(self, serial_port=SERIAL_PORT, baud_rate=BAUD_RATE):
        self.serial_port = serial_port
        self.baud_rate = baud_rate
        self.connection = None

    def connect(self):
        retry = True
        while retry:
            try:
                print('Establishing connection with Arduino')
                self.connection = serial.Serial(self.serial_port, self.baud_rate)
                time.sleep(5)
                if self.connection is not None:
                    print('Successfully connect with Arduino: ' + str(self.connection.name))
                    retry = False
            except Exception as error:
                    print('Failed to connect with Arduino: ' + str(error))

    def disconnect(self):
        try:
            if self.connection is not None:
                self.connection.close()
                self.connection = None

                print('Successfully disconnect with Arduino')

        except Exception as error:
            print('Failed to disconnect with Arduino: ' + str(error))
            
    def read(self):
        try:
            message = self.connection.readline().decode("unicode_escape").strip()

            print('From Arduino:')
            print("\t" + message)

            if len(message) > 0:
                return message

            return None
       
        except Exception as error:
            print('Failed to read from Arduino: ' + str(error))
            raise error
    
    def write(self, message):
        try:
            print('To Arduino:\n\t' + message)
            self.connection.write(message.encode())

        except Exception as error:
            print('Failed to write to Arduino: ' + str(error))
            raise error
