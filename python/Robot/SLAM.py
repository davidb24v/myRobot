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
        self.d2r = np.pi/180.0
        self.sonarRadians = np.array(robot.sonarAngles)*self.d2r
        
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
        
        # Do a 360
        self.rotate(-360)
        
    def ping(self, sonar):
        dist = self.robot.ping(sonar)
        self.angle += self.robot.yaw()
        if dist:
            d = float(dist[0])
            theta = np.pi/2.0-self.d2r*self.angle-self.sonarRadians[sonar]
            x = d*math.cos(theta)
            y = d*math.sin(theta)
        else:
            x = y = np.NaN
        return np.array([x, y])

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
