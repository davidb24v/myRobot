
import serial
import io
import time

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
        ser = serial.Serial(port=self.device,baudrate=115200,
                            timeout=0.05,
                            bytesize=8, parity='N', stopbits=1)
        self.ser = ser
        self.io = io.TextIOWrapper(io.BufferedRWPair(ser,ser))
        print "Opened Arduino on port " + device
        
        # reset
        ser.setDTR(False)
        time.sleep(0.04)
        ser.setDTR(True)
        time.sleep(2)
        
        self.leftSpeed = 0
        self.rightSpeed = 0
        
        # wait for "READY"
        print "Waiting for MPU-6050 yaw to settle..."
        test = ""
        while test != "READY":
            test = self.readln()
            print test

    def write(self,s):
        self.io.write(unicode(str(s)+"\n"))
        self.io.flush()
    
    # blocking read
    def readln(self):
        while True:
            result = self.io.readline()
            if result:
                break
        return result.strip()

    # non-blocking read
    def read(self):
        try:
            result = self.io.readline()
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
            result = "NAK"
            pass

        return result.strip()

    # Yaw from MPU-6050
    def yaw(self):
        self.write("yaw")
        return self.readln()
        
    # HC SR-04 range finders
    def ping(self,sensor):
        self.write("P")
        self.write(sensor)
        result = self.read()
        return result.split()
        
    # Report number of sensors
    def numSonar(self):
        return 5
        
    # return angle of each sensor
    def angle(self,sensor):
        if sensor == 0:
            return 0
        elif sensor == 1:
            return -30
        elif sensor == 2:
            return 30
        elif sensor == 3:
            return -60
        elif sensor == 4:
            return 60
        else:
            Exception("Invalid sensor number")

        
    # format speed value
    def formatSpeed(self,speed):
        if speed < 0 :
            return '-%03d\n' % -speed
        else:
            return ' %03d\n' % speed
        
    # motor speed control
    def leftMotor(self,speed):
        if speed <> self.leftSpeed:
            self.write("L"+self.formatSpeed(speed))
            self.leftSpeed = speed

    def rightMotor(self,speed):
        if speed <> self.rightSpeed:
            self.write("R"+self.formatSpeed(speed))
            self.rightSpeed = speed

    def bothMotors(self,speed):
        self.write("B"+self.formatSpeed(speed))
        self.leftSpeed = speed
        self.rightSpeed = speed

    def off(self):
        self.write("off")

    def idle(self):
        self.write("idle")

    def amber(self):
        self.write("amber")

    def red(self):
        self.write("red")

    def redblue(self):
        self.write("redblue")

