/*This file is part of the Makesmith Control Software.

    The Makesmith Control Software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Makesmith Control Software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Makesmith Control Software.  If not, see <http://www.gnu.org/licenses/>.
    
    Copyright 2014-2016 Bar Smith*/


#include "Arduino.h"
#include "Axis.h"
#include "PID_v1.h"

#define FORWARD 1
#define BACKWARD -1

//PID setup 
double _pidSetpoint, _pidInput, _pidOutput;
double Kp=300, Ki=0, Kd=10;
PID _pidController(&_pidInput, &_pidOutput, &_pidSetpoint, Kp, Ki, Kd, DIRECT);

Axis::Axis(int pwmPin, int directionPin1, int directionPin2, int encoderDirection, int encoderPin, String axisName){
    Serial.println("begin this is a long string so you can see some of it");
    
    
    //initialize motor
    _motor      = GearMotor();
    _motor.setupMotor(pwmPin, directionPin1, directionPin2);
    //_motor.write(0);
    
    //initialize pins
    pinMode(encoderPin, INPUT);
    
    //initialize PID controller
    
    double _pidSetpoint, _pidInput, _pidOutput;
    double Kp=300, Ki=0, Kd=10;
    PID _pidController(&_pidInput, &_pidOutput, &_pidSetpoint, Kp, Ki, Kd, DIRECT);
    
    _pidController.SetMode(AUTOMATIC);
    _pidController.SetOutputLimits(-90, 90);
    Serial.println("PID controller mode:");
    Serial.println(_pidController.GetMode());
    
    //initialize variables
    _direction    = encoderDirection;
    _encoderPin   = encoderPin;
    _axisName     = axisName;
    _axisPosition = 0.0;
    _axisTarget   = 0.0;
    
}

int    Axis::write(float targetPosition){
    
    _pidInput      =  0;
    _pidSetpoint   =  1000;
    
    
    
    bool pidreturn = _pidController.Compute();
    //Serial.println(pidreturn);
    //Serial.println(_pidController.GetMode());
    
    Serial.print(_pidInput);
    Serial.print(" ");
    Serial.print(_pidSetpoint);
    Serial.print(" ");
    Serial.print(_pidOutput);
    Serial.print(" ");
    Serial.print(_pidController.GetMode());
    Serial.print(" ");
    Serial.println(pidreturn);
    
    _motor.write(90 + _pidOutput);
    
    return 1;
}

float  Axis::read(){
    return 1.234;
}

int    Axis::set(float newAxisPosition){
    Serial.println (" would manually reset axis pos here");
}

int    Axis::updatePositionFromEncoder(){

/*I would like to rename this function "updateMachineLocation" The SetPos() function updates the machine's position by essentially integrating 
the input from the encoder*/

    int maxJump = 400;
    static int currentAngle;
    static int previousAngle;

    if(abs(currentAngle - previousAngle) <= maxJump){ //The encoder did not just transition from 0 to 360 degrees
        _axisPosition = _axisPosition + (currentAngle - previousAngle)/1023.0; //The position is incremented by the change in position since the last update.
    }
    else{//The transition from 0 to 360 (10-bit value 1023) or vice versa has just taken place
        if(previousAngle < 200 && currentAngle > 850){ //Add back in any dropped position
            currentAngle = 1023;
            _axisPosition = _axisPosition + (0 - previousAngle)/1023.0;
        }
        if(previousAngle > 850 && currentAngle < 200){
            currentAngle = 0;
            _axisPosition = _axisPosition + (1023 - previousAngle)/1023.0;
        }
    }
    

    previousAngle = currentAngle; //Reset the previous angle variables

    if(_direction == FORWARD){ //Update the current angle variable. Direction is set at compile time depending on which side of the rod the encoder is positioned on.
        currentAngle = _PWMread(_encoderPin);
    }
    else{
        currentAngle = 1023 - _PWMread(_encoderPin);
    }
}

int    Axis::_PWMread(int pin){

/*PWMread() measures the duty cycle of a PWM signal on the provided pin. It then
takes this duration and converts it to a ten bit number.*/

    int duration = 0;
    int numberOfSamplesToAverage = 1;
    int i = 0;
    
    while (i < numberOfSamplesToAverage){
        duration = duration + pulseIn(pin, HIGH, 2000); //This returns the pulse duration
        i++;
    }
    
    duration = duration/numberOfSamplesToAverage;
    
    duration = (int)((float)duration*1.23); //1.23 scales it to a ten bit number
    
    
    if (duration >= 1023){
        duration = 1023;
    }

    if (duration < 10){
        duration = 0;
    }

    return duration;
}

int Axis::returnPidMode(){
    return _pidController.GetMode();
}