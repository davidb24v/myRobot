#!/usr/bin/python
# 

import time
import signal
import sys
import Robot

def pause():
    time.sleep(0.1)

robot = Robot.Arduino("/dev/ttyAMA0")
#robot = Robot.Arduino("/dev/ttyACM0")
#robot = Robot.Arduino("/dev/ttyUSB0")
robot.idle()
robot.bothMotors(0)
pause()
robot.idle()

def signal_handler(signal, frame):
    global robot
    robot.bothMotors(0)
    pause()
    robot.red()
    pause()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

# Load a guidance module
#control = Robot.Avoid(robot,speed=150,threshold=10)

#control.start()
robot.amber()
while True:
    print robot.yaw()
    time.sleep(0.5)


sys.exit()

for i in xrange(0,255):
    robot.bothMotors(i)
    time.sleep(0.05)

robot.idle()
time.sleep(1)
robot.amber()

for i in xrange(255,0,-1):
    robot.bothMotors(i)
    time.sleep(0.05)

robot.amber()

for i in xrange(0,255):
    robot.bothMotors(-i)
    time.sleep(0.05)

robot.idle()
time.sleep(1)
robot.amber()

for i in xrange(255,0,-1):
    robot.bothMotors(-i)
    time.sleep(0.05)

robot.idle()
time.sleep(1)
robot.amber()

robot.bothMotors(0)

robot.idle()
time.sleep(0.1)
robot.idle()
sys.exit()


control.start()

#print step.ping(-80)
#print step.ping(80)
#print step.ping(45)
#print step.ping(-45)
#print step.ping(0)

robot.off()

exit()

# We're going to do things in 15 degree increments since
# the sonar has a 30 degree beam
inc = 15
half_angle = ((step.angle/2)//inc)*inc
step.setAngle(-half_angle)

for angle in xrange(-half_angle, half_angle+inc, inc):
    print step.ping(angle)
    # print robot.heading().split()

# re-position stepper
#step.centre()

exit()

# try to rotate 360 degrees

heading = robot.heading()
start = heading.split()

origin = float(start[0])
print "Starting at " + str(origin) + " degrees"

# start turning anti-clockwise)
speed = 75 
robot.leftMotor(-speed)
robot.rightMotor(speed)

# let the robot get going
pause()
robot.amber()
time.sleep(0.5)

delay = 0.0

# Detect wraparound at 0
last = origin
while True:
    current = robot.heading()
    print current
    now = float(current.split()[0])
    if now > last+2:
        break
    print step.pingOnly()
    last = now

print "*********** Found zero ****************"
print now, last
print "***************************************"

# detect when we go past the origin
while True:
    current = robot.heading()
    now = float(current.split()[0])
    print current
    if now < origin:
        break
    print step.pingOnly()

robot.bothMotors(0)
pause()
robot.off()
