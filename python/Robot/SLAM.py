import numpy as np
import time
import math

class SLAM:
    "Implement Simultaneous Location and Mapping "

    def __init__(self, robot=None,
                 threshold=10.0,speed=125,extent=60.0,
                 noDistance=200):

        if robot:
            self.robot = robot
        else:
            raise Exception("Must specify robot")
            
        # number of directions to use
        self.nDir = robot.numSonar
        
        # Sonar angles, in radians
        self.sonarRadians = np.radians(np.array(robot.sonarAngles))
        
        # save motor speed factor
        self.speed = speed
	
        # sample data
        self.d = np.zeros([self.nDir])
        
        # distance threshold
        self.threshold = threshold
        
        # what value to use if we don't get a sonar return
        self.noDistance = noDistance
        
        # track our angle
        self.angle = 0.0
        
        # Run through the sonar sensors
        self.__startSonar()
        
        # Calibration
        self.calibrate(2)
        #return
        
        # Experiment with forward/reverse motion
        self.move(2,30.0)
        time.sleep(2.0)
        self.move(2,40.0)
        time.sleep(2.0)
        self.move(2,60)
        time.sleep(1)
        self.move(2,10)
        return
        
        # Do a 360
        self.rotate(-360)
        
    def __startSonar(self):
        for sonar in range(self.nDir):
            self.robot.ping(sonar)
            self.robot.ping(sonar)
            
        
    def move(self, sonar, distance, speed=50):
        dist = None
        while not dist:
            dist = self.robot.ping(sonar)
        print "initial dist = ",dist
        dist = float(dist[0])-distance

        spd = np.sign(dist)*speed
        left = right = spd
        dist = abs(dist)
        self.robot.bothMotors(spd)
        yaw = 0.0
        while True:
            dist = None
            while not dist:
                dist = self.robot.ping(sonar)
            print "dist = ", dist
            dist = float(dist[0])-distance
            #yaw += self.robot.yaw()
            print dist, yaw, math.sin(np.radians(yaw))
            if spd < 0:
                if dist >= 0.0:
                    break
            else:
                if dist <= 0.0:
                    break
        self.robot.bothMotors(0)
                
        
    def ping(self, sonar):
        dist = self.robot.ping(sonar)
        self.angle += self.robot.yaw()
        angle = np.radians(self.angle)
        if dist:
            d = float(dist[0])
            theta = np.pi/2.0-angle-self.sonarRadians[sonar]
            x = d*math.cos(theta)
            y = d*math.sin(theta)
        else:
            x = y = np.NaN
        return np.array([x, y])
        
    def calibrate(self, sonar):
        for i in range(5):
            self.robot.ping(sonar)
        for i in range(50):
            print self.robot.ping(sonar)
        

    def scan(self):
        d = np.zeros(self.robot.numSonar)
        xy = list()
        for sonar in range(self.robot.numSonar):
            xy.append(self.ping(sonar))
        return x, y
    
    def start(self):
        # Sonar also seems to need a while to settle initially
        for i in range(3):
            self.scan()
        while True:
            print "Distances: ",self.scan(), "Yaw: ", self.robot.yaw()
            time.sleep(1.0)


    def rotate(self, angle, speed=60):
        theta = 0.0
        if angle < 0.0:
            speed = -speed

        # Start your engines
        self.robot.leftMotors(-speed)
        self.robot.rightMotors(speed)
        
        start = self.angle
        for i in range(3):
            self.ping(0)
        x0, y0 = self.ping(0)
        x1, y1 = self.ping(1)
        x2, y2 = self.ping(2)
        x3, y3 = self.ping(3)
        x4, y4 = self.ping(4)
        print self.angle, x0, y0, x1, y1, x2, y2, x3, y3, x4, y4

        while abs(self.angle-start) <= abs(angle):
            x0, y0 = self.ping(0)
            x1, y1 = self.ping(1)
            x2, y2 = self.ping(2)
            x3, y3 = self.ping(3)
            x4, y4 = self.ping(4)
            print self.angle, x0, y0, x1, y1, x2, y2, x3, y3, x4, y4
        
        self.robot.bothMotors(0)
