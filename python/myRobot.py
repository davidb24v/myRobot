#!/usr/bin/python
# 

import time
import signal
import sys
import Robot

def pause():
    time.sleep(0.1)

robot = Robot.Arduino("/dev/ttyAMA0")

robot.bothMotors(0)

def signal_handler(signal, frame):
    global robot
    robot.bothMotors(0)
    pause()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

# Load a guidance module
control = Robot.Avoid(robot,speed=150,threshold=10)

control.start()

