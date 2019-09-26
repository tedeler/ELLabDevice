################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\hardware\sam\1.6.11\cores\arduino\avr\dtostrf.c 

C_DEPS += \
.\core\core\avr\dtostrf.c.d 

AR_OBJ += \
.\core\core\avr\dtostrf.c.o 


# Each subdirectory must supply rules for building sources it contributes
core\core\avr\dtostrf.c.o: C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\hardware\sam\1.6.11\cores\arduino\avr\dtostrf.c
	@echo 'Building file: $<'
	@echo 'Starting C compile'
	"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1/bin/arm-none-eabi-gcc" -c -g -Os -Wall -Wextra -std=gnu11 -ffunction-sections -fdata-sections -nostdlib --param max-inline-insns-single=500 -Dprintf=iprintf -MMD -mcpu=cortex-m3 -mthumb -DF_CPU=84000000L -DARDUINO=10802 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM  -D__SAM3X8E__ -mthumb -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON "-DUSB_MANUFACTURER=\"Arduino LLC\"" "-DUSB_PRODUCT=\"Arduino Due\"" "-IC:\Users\torsten\eclipse\cpp-photon\eclipse\/arduinoPlugin/packages/arduino/hardware/sam/1.6.11/system/libsam" "-IC:\Users\torsten\eclipse\cpp-photon\eclipse\/arduinoPlugin/packages/arduino/hardware/sam/1.6.11/system/CMSIS/CMSIS/Include/" "-IC:\Users\torsten\eclipse\cpp-photon\eclipse\/arduinoPlugin/packages/arduino/hardware/sam/1.6.11/system/CMSIS/Device/ATMEL/"   -I"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\hardware\sam\1.6.11\cores\arduino" -I"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\hardware\sam\1.6.11\variants\arduino_due_x" -I"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\libraries\MCUFRIEND_kbv\2.9.8\utility" -I"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\libraries\MCUFRIEND_kbv\2.9.8" -I"C:\Users\torsten\Documents\Arduino\libraries\Adafruit_GFX" -I"C:\Users\torsten\eclipse\cpp-photon\eclipse\arduinoPlugin\packages\arduino\hardware\sam\1.6.11\libraries\SPI\src" -I"C:\Users\torsten\Documents\Arduino\libraries\Adafruit_TFTLCD" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


