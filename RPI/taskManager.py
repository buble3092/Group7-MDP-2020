import queue
import _thread
import os
import time
import re
import io
from PIL import Image

from arduino import Arduino
from android import Android
from algorithm import Algorithm
from imager import Imager
from config import *


class TaskManager:
	def __init__(self):

		print("Initializing task manager")
		
		self.arduino = Arduino()  
		self.algorithm = Algorithm()
		self.android = Android()
		self.imager = Imager()


		#self.imager = Imager(port = 8090)


		#initialize message queue for arduino, PC and android tablet
		self.arduinoMsg = queue.Queue()
		self.algorithmMsg = queue.Queue()
		self.androidMsg = queue.Queue()
		self.coordinateMsg = queue.Queue()



	def start(self):

		print("Initializing connections with arduino, PC and android tablet")
		self.arduino.connect()
		self.algorithm.connect()
		self.android.connect()
		self.imager.connect()


		#robot autocalibration on 3x3 grid at starting zone
		#print("Start Auto Calibration......")
		#self.arduino.write("Y")
		#self.algorithm.write("ex")

		#self.arduino.write("N")

		#start multi-threading
		try:
			_thread.start_new_thread(self.readArduino, ())
			_thread.start_new_thread(self.readAlgorithm, ())
			_thread.start_new_thread(self.readAndroid, ())
			_thread.start_new_thread(self.readImager, ())
			#_thread.start_new_thread(self.writeArduino, ())
			#_thread.start_new_thread(self.writeAlgorithm, ())
			#_thread.start_new_thread(self.writeAndroid, ())

			#_thread.start_new_thread(self.sendImage, ())
			print("Start Multiprocess Communication......")
		except Exception as error:
			print("Failed to start multi-threading: " + str(error))
			raise error

		while 1:
			pass


	def readArduino(self):
		temp = 1
		while 1:
			message = self.arduino.read()

			'''if message[0:9] == "0,0,0,0,0" and temp == 1:
				print("Calibrating......")
				self.arduino.write("I")
				temp = 0
				continue

			elif message[0:5] == "0,0,0" and temp == 1:
				print("Calibrating......")
				self.arduino.write("P")
				#self.arduinoMsg.put_nowait("P")
				temp = 0
				continue
			'''

			self.algorithm.write(message)
			#self.algorithmMsg.put_nowait(message)
			temp = 1


	def readAlgorithm(self):
		mode = "ex"
		while 1:
			message = self.algorithm.read()
			if not message == None:
				if message == "se" or message == "sf":
					if message == "se":
						self.arduino.write("Y")

						image = Image.open("stop.jpeg")
						file_path = io.BytesIO();
						image.save(file_path,'JPEG');
						self.imager.sendImage(file_path)
						
						mode = "fp"

					self.android.write(message)
					#self.androidMsg.put_nowait(message)
					continue

				
				#make the RPI take a photo each step and send to PC
				

				direction = 0

				temp = message.split(",")
				x = int(temp[-3])
				y = int(temp[-2])

				'''if message[0] == "1":
					if message[-1] == "0":
						y = y - 1
					elif message[-1] == "1":
						x = x - 1
					elif message[-1] == "2":
						y = y + 1
					else: x = x + 1
				elif message[0] == "D":
					direction = direction - 1
					if direction == -1:
						direction = 3
				elif message[0] == "A":
					direction = direction + 1
					if direction == 4:
						direction = 0
				'''
				#imageCoordinate = self.getImageCoodinate(x, y, temp[-1])

				#self.coordinateMsg.put_nowait(imageCoordinate)
				


				'''if message[0] == "A" or message[0] == "D":
					#self.imager.sendImage()
					self.arduino.write(message[0])
				else:
					steps = int(message[0])
					for i in range(steps):
						#self.imager.sendImage()
						self.arduino.write("1")
					self.arduino.write(message[0])
				'''
				self.arduino.write(message[0])

				self.android.write("grid," + message[2:])
				#self.androidMsg.put_nowait("grid," + message[2:])

				if mode == "ex":		
					if x==1 and message[-1]=="0":
						continue
					elif x == 13 and message[-1]=="2":
						continue
					elif y == 1 and message[-1]=="3":
						continue
					elif y == 18 and message[-1] == "1":
						continue

					time.sleep(0.3)
					print("taking photo!")
					self.imager.sendImage1()
					imageCoordinate = self.getImageCoodinate(x, y, temp[-1])

					self.coordinateMsg.put_nowait(imageCoordinate)
				
				#self.arduinoMsg.put_nowait(message[0])

							


	def readAndroid(self):
		temp1 = 1
		temp2 = 0
		while 1:
			message = self.android.read()
			self.algorithm.write(message)
			#self.algorithmMsg.put_nowait(message)

			if message == "ex":
				if(temp1):
					self.arduino.write("N")
					print("Start Exploration!")
					temp1 = 0
			elif message == "W":
				self.arduino.write("1")
			elif message == "A":
				self.arduino.write("A")
			elif message == "D":
				self.arduino.write("D")
			else:
				if(temp2):
					x = message.split(',')[1]
					y = message.split(',')[2]
					self.algorithm.write("pf," + x + "," + y)
					print("Start Fastest Path Exploration")
					temp2 = 0

	def readImager(self):
		while 1:
			imageID = self.imager.read()
			if not self.coordinateMsg.empty():
				coordinate = self.coordinateMsg.get_nowait()
				if imageID != "!":
					self.android.write("ID " + imageID + "," + coordinate)

	def writeArduino(self):
		#self.arduino.write("M")
		#self.arduino.write("T")
		#self.arduino.write("I")
		#self.arduino.write("U")
		while 1:
			if not self.arduinoMsg.empty():

				#self.imager.sendImage()

				message = self.arduinoMsg.get_nowait()
				self.arduino.write(message)

	def writeAlgorithm(self):
		while 1:
			if not self.algorithmMsg.empty():
				message = self.algorithmMsg.get_nowait()
				self.algorithm.write(message)

	def writeAndroid(self):
		while 1:
			if not self.androidMsg.empty():
				message = self.androidMsg.get_nowait()
				self.android.write(message)
		

	def getImageCoodinate(self, x, y, direction):
		if direction == '0':
			x = x - 2
			y = y - 1
		elif direction == '1':
			y = y + 2
			x = x - 1
		elif direction == '2':
			x = x + 2
			y = y + 1
		elif direction == '3':
			y = y - 2
			x = x + 1

		result = str(x) + "," + str(y) + "," + direction

		return result


