#BOARD_TAG    = uno
#MONITOR_PORT = /dev/ttyACM0
#ARDUINO_LIBS = Wire/utility Wire SendOnlySoftwareSerial NewPing MemoryFree MPU6050 I2Cdev
#
#include $(ARDMK_DIR)/Arduino.mk


BOARD_TAG    = gertduino328
MONITOR_PORT = /dev/ttyAMA0
MONITOR_BAUDRATE = 9600
ARDUINO_LIBS = Wire/utility Wire SendOnlySoftwareSerial NewPing MemoryFree MPU6050 I2Cdev

ISP_PROG   = arduino
AVRDUDE_ISP_BAUDRATE = 57600
AVRDUDE = /home/pi/gertduino/gertduino-serial
ISP_PORT = /dev/ttyAMA0

include $(ARDMK_DIR)/Arduino.mk


