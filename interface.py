#!/usr/bin/env python2
import serial

s=serial.Serial("/dev/ttyACM0",9600,timeout=1)
s.flush()

def read():
	s.write("A")
	data=map(ord,s.read(2))
	data=data[0]*256+data[1]
	#return ((data - 673.0) * 423) / 1024
	return data

if __name__=="__main__":
	try:
		import time
		#orig=read()
		while 1:
			try:
				print read()
			except IndexError:
				print "timeout"
			time.sleep(0.1)
	except KeyboardInterrupt:
		s.close()
