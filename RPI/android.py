import bluetooth as bt
import os

from config import ANDROID_SOCKET_BUFFER_SIZE
from config import LOCALE
from config import RFCOMM_CHANNEL
from config import UUID


'''
Rapsberry Pi serves as socket server, N7 will need a client socket script
as well to establish connection. Should be able to send and receive messages
via the server/client.
'''

# bluetooth reference: https://people.csail.mit.edu/albert/bluez-intro/index.html

class Android:
    def __init__(self):
        self.server_sock = None
        self.client_sock = None
        
        self.server_sock = bt.BluetoothSocket(bt.RFCOMM)
        self.server_sock.bind(("", RFCOMM_CHANNEL))

        self.server_sock.listen(RFCOMM_CHANNEL)
        """
        bt.advertise_service(
            self.server_sock, 
            'MDP_Group_7_RPi',
            service_id = UUID,
            service_classes = [UUID, bt.SERIAL_PORT_CLASS],
            profiles = [bt.SERIAL_PORT_PROFILE]
        )
        """
        print('server socket:', str(self.server_sock))
        
    def connect(self):

        os.system("sudo hciconfig hci0 piscan")
        os.system("sudo rfcomm listen /dev/rfcomm6 6")

        retry = True
        while retry:
            try:
                print('Establishing connection with Android N7 Tablet...')

                if self.client_sock is None:
                    self.client_sock, address = self.server_sock.accept()
                    print("Successfully connected to Android at address: " + str(address))
                    retry = False

            except Exception as error:	
                print("Connection with Android failed: " + str(error))

                if self.client_sock is not None:
                    self.client_sock.close()
                    self.client_sock = None
                
                retry = True
            
    def disconnect(self):
        try:
            if self.client_sock is not None:
                self.client_sock.close()
                self.client_sock = None

            print("Successfully disconnected with Android")

        except Exception as error:	
            print("Failed to disconnect with Android: " + str(error))
            
    def disconnect_all(self):
        try:
            if self.client_sock is not None:
                self.client_sock.close()
                self.client_sock = None

            if self.server_sock is not None:
                self.server_sock.close()
                self.server_sock = None

'''I'm working on the multitthread programming to do the communications among different components, 
trying to 
'''
            print("Successfully disconnected with Android")

        except Exception as error:	
            print("Failed to disconnect with Android: " + str(error))
        
    def read(self):
        try:
            message = self.client_sock.recv(ANDROID_SOCKET_BUFFER_SIZE).decode("unicode_escape").strip()
            print('From android:')
            print("\t" + message)
            
            if message is None:
                return None

            if len(message) > 0:
                return message
            
            return None
            
        except Exception as error:
            print('Android read failed: ' + str(error))
            raise error
      
    def write(self, message):
        try:
            print('To Android:\n\t' + message)
            self.client_sock.sendall(message.encode('utf-8'))

        except Exception as error:	
            print('Android write failed: ' + str(error))
            raise error
