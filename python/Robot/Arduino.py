
import serial
import time
import subprocess

class Arduino:
    """
    
    Functions for talking to the Arduino
    
    HC-SR04 sensors use the NewPing library which means we need
    to allow a timeout to see if we get an echo. If we take the 
    speed of sound as 300 m/s and allow for a 4m round trip we
    need a delay of 4/300 = 0.013333. Let's call that 0.015 which
    gives us a maximum ping rate of about 67Hz - probably more
    than is needed for this.
    
    """
    def __init__(self, device=None):
        if device:
            self.device = device
        else:
            Exception('fatal','device not specified')
        
        # open the port
        ser = serial.Serial(port=self.device,baudrate=57600,
                            timeout=0.05,
                            bytesize=8, parity='N', stopbits=1)
        self.ser = ser
        ser.setTimeout = 0.1
        print "Opened Arduino on port " + device
        
        # reset
        subprocess.call('/home/pi/gertduino/reset')
        #subprocess.call('/usr/local/bin/ard-reset-arduino')
        time.sleep(2.0)
        
        self.leftSpeed = 0
        self.rightSpeed = 0
        
        # Number of sonar sensors
        self.numSonar = 5
        
        # Sonar order
        self.sonarMap =[4, 2, 0, 1, 3]
        
        # Sonar angles
        self.sonarAngles = [-60.0, -30.0, 0.0, 30.0, 60.0]
        
        # Make sure motors are off
        self.bothMotors(0)
        
        # wait for "READY"
        print "Waiting for MPU-6050 yaw to settle..."
        test = ""
        while test != "READY":
            test = self.readln()
            print test

    def write(self,s):
        self.ser.write(unicode(str(s)+"\n"))
        self.ser.flush()
    
    # blocking read
    def readln(self):
        while True:
            result = self.ser.readline()
            if result:
                break
        return result.strip()

    # non-blocking read
    def read(self):
        try:
            result = self.ser.readline()
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
            result = "NAK"
            pass

        return result.strip()

    # Yaw from MPU-6050
    def yaw(self):
        self.write("yaw")
        try:
            return float(self.readln())
        except:
            return 0.0
        
    # HC SR-04 range finders
    def ping(self,sensor):
        sonar = self.sonarMap[sensor]
        self.write("P")
        self.write(sonar)
        result = self.read()
        return result.split()
        
    # format speed value
    def formatSpeed(self,speed):
        if speed < 0 :
            return '-%03d\n' % -speed
        else:
            return ' %03d\n' % speed
        
    # motor speed control
    def leftMotors(self,speed):
        if speed <> self.leftSpeed:
            self.write("L"+self.formatSpeed(speed))
            self.leftSpeed = speed

    def rightMotors(self,speed):
        if speed <> self.rightSpeed:
            self.write("R"+self.formatSpeed(speed))
            self.rightSpeed = speed

    def bothMotors(self,speed):
        self.write("B"+self.formatSpeed(speed))
        self.leftSpeed = speed
        self.rightSpeed = speed

