import socket
import time
import io
import struct
#import picamera
from picamera.array import PiRGBArray




from config import LOCALE
from config import ALGORITHM_SOCKET_BUFFER_SIZE
from config import WIFI_IP
from config import WIFI_PORT1


'''
Algorithm will need an accompanying (reference available in playgrounds pc_client.py
Algorithm.connect() will wait for Algorithm to connect before proceeding
'''

class Algorithm:
    def __init__(self, host = WIFI_IP, port = WIFI_PORT1):
        self.host = host
        self.port = port

        self.client_sock = None
        self.socket = None
        self.address = None
        self.connection = None

        #self.camera = picamera.PiCamera()
        #self.camera.resolution = (640, 480)
        #self.camera.start_preview()
        #time.sleep(2)

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        self.socket.bind((self.host, self.port))
        self.socket.listen(1)
        
    def connect(self):
        retry = True
        while retry:
            try:
                print('Establishing connection with Algorithm')

                if self.client_sock is None:
                    self.client_sock, self.address = self.socket.accept()
                    self.connection = self.client_sock.makefile('wb')
                    print('Successfully connect with Algorithm: ' + str(self.address))
                    retry = False

            except Exception as error:
                print('Failed to connect with Algorithm: ' + str(error))

                if self.client_sock is not None:
                    self.client_sock.close()
                    self.client_sock = None
                retry = True

    def disconnect(self):
        try:
            if self.client_sock is not None:
                self.client_sock.close()
                self.client_sock = None
            
            print("Successfully disconnect with Algorithm")

        except Exception as error:
            print("Failed to disconnect with Algorithm: " + str(error))

    def disconnect_all(self):
        try:
            if self.client_sock is not None:
                self.client_sock.close()
                self.client_sock = None

            if self.socket is not None:
                self.socket.close()
                self.socket = None

            print("Successfully disconnect with Algorithm")

        except Exception as error:
            print("Failed to disconnect with Algorithm: " + str(error))

    def read(self):
        try:
            message = self.client_sock.recv(ALGORITHM_SOCKET_BUFFER_SIZE).strip()
            message = message.decode("utf-8")

            if len(message) > 0:
                print('From Algorithm: ')
                print("\t" + message)
                return message

            return None

        except Exception as error:
            print('Failed to read from Algorithm: '+ str(error))
            raise error

    def write(self, message):
        try:
            print('To Algorithm:')
            print("\t" + message)
            message = message + "\n"
            self.client_sock.sendall(message.encode('utf-8'))

        except Exception as error:
            print('Failed to write to Algorithm: '+ str(error))
            raise error
        

    def sendImage(self):
        stream = io.BytesIO()
        self.camera.capture(stream, format='jpeg')
        self.connection.write(struct.pack('<L', stream.tell()))
        self.connection.flush()
        stream.seek(0)
        self.connection.write(stream.read())
        stream.seek(0)
        stream.truncate()

        


