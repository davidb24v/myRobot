from numpy import *
import time

class Avoid:
    "Try to avoid obstacles using just a few ping directions"
    def __init__(self, robot=None,
                 threshold=40.0,speed=80,extent=60.0,
                 nodistance=200):

        if robot:
            self.robot = robot
        else:
            raise Exception("Must specify robot")
            
        # number of directions to use
        self.ndir = robot.numSonar()
        
        # save motor speed factor
        self.speed = speed
	
        # sample data
        self.d = zeros( [self.ndir] )
        
        # distance threshold, store 1/(threshold^2)
        self.threshold = threshold
        
        # what value to use if we don't get a sonar return
        self.nodistance = nodistance
        
        # Convert maximum extent of stepper to radians
        r_extent = extent*pi/180.0

        # pre-compute sine and cosine of corresponding angles
        # anti-clockwise is positive for stepper
        # In X,Y space angle increases from +X axis so adjust
        # the radians that we'll be using for consistency
        self.angle_rad = linspace( pi/2+r_extent, pi/2-r_extent, self.ndir )
        self.angle_deg = linspace( extent, -extent, self.ndir )
        
        # Define sensor order
        self.map = [ 4, 2, 0, 1, 3 ]
        #self.map = [ 2, 0, 1 ]
        
    def start(self):
        # get initial sonar scan
        for i in xrange(0,self.ndir):
            result = self.robot.ping(self.map[i])
            if result:
                self.d[i] = result[0]
            else:
                self.d[i] = self.nodistance

        # Continually scan and update motor speeds
        while True:
            self.step()
        
    def step(self):
        for i in xrange(0,self.ndir):
            result = self.robot.ping(self.map[i])
            if result:
                self.d[i] = result[0]
            else:
                self.d[i] = self.nodistance
        self.__compute()
                
    def __compute(self):
        # if all readings too close for comfort, spin around for a bit
        if self.d.min() < self.threshold:
            self.robot.redblue()
            self.robot.bothMotors(0)
            print "REVERSE"
            self.robot.bothMotors(-100)
            time.sleep(0.5)
            self.robot.bothMotors(0)
            print "TURN"
            self.robot.leftMotor(-100)
            time.sleep(0.01)
            self.robot.rightMotor(100)
            time.sleep(0.5)
            self.robot.bothMotors(0)
            print "SENSOR SWEEP"
            # update all sensors
            for i in xrange(0,self.ndir):
                result = self.robot.ping(self.map[i])
                if result:
                    self.d[i] = result[0]
                else:
                    self.d[i] = self.nodistance
            print "CONTINUE"
            self.robot.amber()
            return
        
        # Repulsion
        idx = self.d < self.threshold
        self.d[idx] *= -1
        
        # distance
        dn = sqrt(dot(self.d,self.d))
        
        # weighted sum of angle (remove pi/2 so median is zero)
        ws = self.d*(self.angle_rad-pi/2)/dn
        
        # If the heading was pi/2 we would want the robot to
        # drive left and right motors at the same speed. Rotate
        # by -pi/4 (45 degrees clockwise) to get this to happen
        # since we need to add back pi/2, just add back pi/4 to
        # allow for this

        # heading
        heading = ws.sum()+pi/2
        left = int(self.speed*cos(heading-pi/4))
        right = int(self.speed*sin(heading-pi/4))
        self.robot.leftMotor(left)
        print "heading: "+str(heading*180/pi), self.d, left, right
        print self.orbot.yaw()
        time.sleep(0.01)
        self.robot.rightMotor(right)
