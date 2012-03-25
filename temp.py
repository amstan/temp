#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import serial

class Temp():
	_history=[0.0]
	HISTORYSIZE=2**9
	
	def __init__(self,port="/dev/ttyACM0"):
		self.port=port
		self.serial=serial.Serial(self.port,9600,timeout=1)
		self.serial.flush()
	
	def prunehistory(self):
		"""Prunes self._history until it becomes of HISTORYSIZE."""
		if len(self._history)>self.HISTORYSIZE:
			self._history=self._history[-self.HISTORYSIZE:]
	
	@property
	def temp(self):
		"""Current temperature, in deg C"""
		return sum(self._history)/len(self._history)
	
	def read(self):
		"""Gets temperature from serial and updates. Returns self.temp"""
		try:
			#ask for temperature
			self.serial.flush()
			self.serial.write("A")
			
			#read and parse the data to an 16 bit int
			data=map(ord,self.serial.read(2))
			data=data[0]*256+data[1]
			
			#convert the data to deg C
			temp=((data - 673.0) * 423) / 1024
			
			#add the data to the history
			self._history.append(temp)
			self.prunehistory()
		except IndexError:
			raise IOError("Timeout for %r." % self)
		
		return self.temp
	
	def __str__(self):
		"""Returns nicely formatted temperature."""
		return "%2.2f°C" % self.temp
	
	def __repr__(self):
		return "Temp Mon<%s>: %s" % (self.port, self)

if __name__=="__main__":
	import time,sys
	
	temp=Temp(sys.argv[1])
	print "Reading as fast as we can from %r, and printing temps every time the averaging buffer gets cycled(%d times)." % (temp,temp.HISTORYSIZE)
	while 1:
		try:
			for _ in range(temp.HISTORYSIZE):
				temp.read()
			print repr(temp)
		except Exception as e:
			print repr(e)