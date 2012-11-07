################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Engine.cpp \
../src/World.cpp \
../src/gameoflife-openmpi-stitch.cpp 

OBJS += \
./src/Engine.o \
./src/World.o \
./src/gameoflife-openmpi-stitch.o 

CPP_DEPS += \
./src/Engine.d \
./src/World.d \
./src/gameoflife-openmpi-stitch.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	mpic++ -Impi -I/usr/lib/openmpi/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


