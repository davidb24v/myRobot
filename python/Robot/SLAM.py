import numpy as np
import time

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
        self.rotate(360)

    def scan(self):
        d = np.zeros(self.robot.numSonar)
        for sonar in range(self.robot.numSonar):
            dist = self.robot.ping(sonar)
            if dist:
                d[sonar] = dist[0]
            else:
                d[sonar] = self.noDistance
        return d
    
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
            angle = -angle
            speed = -speed

        # Start your engines
        self.robot.leftMotors(-speed)
        self.robot.rightMotors(speed)
        
        t1 = time.time()
        while theta <= angle:
            yaw = abs(self.robot.yaw())
            t2 = time.time()
            dt = t2-t1
            t1 = t2
            thetaDot = yaw/dt
            theta += yaw
            print theta, yaw, thetaDot, self.scan()
        
        self.robot.bothMotors(0)
