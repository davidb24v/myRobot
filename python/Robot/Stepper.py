import time
import math

class Stepper:
    "Functions for driving the stepper easily"
    def __init__(self, robot=None, maxSteps=400):

        self.robot = robot
        self.maxSteps = maxSteps
	
        # Calibrate the stepper
        self.delay = 3000
        self.robot.write("SD")
        self.robot.write(self.delay)
        self.robot.write("CS")
	
	# get limit and time for 2*limit steps
	limit = self.robot.readln()
	elapsed = self.robot.readln()
        
        # Store some useful data
        self.limit = limit
	self.timePerStep = 1.1*(0.5*float(elapsed)/float(limit))/1000
        self.stepsPerDegree = maxSteps/360.0
        self.angle = int(360*2*float(limit)/maxSteps/10.0)*10
        self.halfAngle = int(360*2*float(limit)/maxSteps/10.0)*5
        self.pos = 0
        
        print "limit = " + str(limit)
        print "elapsed = " + str(elapsed)
        print "angle = " + str(self.angle)
        
    def centre(self):
        self.setAngle(0)

    def pos2Angle(self,pos):
        return math.floor(pos/self.stepsPerDegree)
    
    def setAngle(self,angle):
        self.robot.write("SP")
        pos = int(angle*self.stepsPerDegree)
        self.robot.write(str(pos))
	steps = abs(pos-self.pos)
	time.sleep(steps*self.timePerStep)
        self.pos = pos
	
    def ping(self,angle):
        self.robot.write("P")
        pos = int(angle*self.stepsPerDegree)
        self.robot.write(str(pos))
	steps = abs(pos-self.pos)
	time.sleep(steps*self.timePerStep)
        self.pos = pos
        result = self.robot.read().strip()
        return result.split()

    def pingOnly(self):
        self.robot.write("p")
        result = self.robot.read().strip()
        return result.split()

    def delay(self,delay):
        if int(delay) != self.delay:
            self.robot.write("SD")
            self.robot.write(str(delay))
            self.delay = int(delay)
