LOCALE = 'UTF-8'

# Arduino USB connection settings
# Symbolic link to always point to the correct port that arduino is connected to
#SERIAL_PORT = '/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_75232303235351F091C0-if00'
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200


# Algorithm Wifi connection settings
WIFI_IP = '0.0.0.0'
WIFI_PORT1 = 8080
WIFI_PORT2 = 8090
ALGORITHM_SOCKET_BUFFER_SIZE = 512


# Android BT connection settings
RFCOMM_CHANNEL = 6
RPI_MAC_ADDR = 'b8:27:eb:b0:19:fe'
UUID = 'a9899c78-8c6c-41d2-8ffa-2a0ad8bf0c49'
ANDROID_SOCKET_BUFFER_SIZE = 512