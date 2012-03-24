#!/usr/bin/env python2
import serial

s=serial.Serial("/dev/ttyACM0",9600,timeout=1)
s.flush()

def read():
	s.write("A")
	data=map(ord,s.read(2))
	data=data[0]*256+data[1]
	return ((data - 673.0) * 423) / 1024
	return data

if __name__=="__main__":
	try:
		temps=[]
		import time
		while 1:
			try:
				temps.append(read())
				if len(temps)>1000:
					temps.pop(0)
				print "%2.2f" % (sum(temps)/len(temps),)
			except IndexError:
				print "timeout"
	except KeyboardInterrupt:
		s.close()

