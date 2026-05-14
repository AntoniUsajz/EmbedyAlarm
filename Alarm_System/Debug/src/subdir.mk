################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/app_alarm.c \
../src/app_buzzer.c \
../src/app_menu.c \
../src/app_temp.c \
../src/app_time.c \
../src/cr_startup_lpc17.c \
../src/main.c 

C_DEPS += \
./src/app_alarm.d \
./src/app_buzzer.d \
./src/app_menu.d \
./src/app_temp.d \
./src/app_time.d \
./src/cr_startup_lpc17.d \
./src/main.d 

OBJS += \
./src/app_alarm.o \
./src/app_buzzer.o \
./src/app_menu.o \
./src/app_temp.o \
./src/app_time.o \
./src/cr_startup_lpc17.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__NEWLIB__ -I"C:\Users\Antoni\Documents\MCUXpressoIDE_25.6.136\G06_NEW\Lib_CMSISv1p30_LPC17xx\inc" -I"C:\Users\Antoni\Documents\MCUXpressoIDE_25.6.136\G06_NEW\Lib_EaBaseBoard\inc" -I"C:\Users\Antoni\Documents\MCUXpressoIDE_25.6.136\G06_NEW\Lib_MCU\inc" -O0 -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -D__NEWLIB__ -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/app_alarm.d ./src/app_alarm.o ./src/app_buzzer.d ./src/app_buzzer.o ./src/app_menu.d ./src/app_menu.o ./src/app_temp.d ./src/app_temp.o ./src/app_time.d ./src/app_time.o ./src/cr_startup_lpc17.d ./src/cr_startup_lpc17.o ./src/main.d ./src/main.o

.PHONY: clean-src

