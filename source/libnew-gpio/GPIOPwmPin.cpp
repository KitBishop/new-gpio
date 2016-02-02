#include <unistd.h>
#include <stdio.h>

#include "GPIOPwmPin.h"
#include "GPIOAccess.h"

GPIOPwmPin::GPIOPwmPin(int pinNum)
{
    pinNumber = pinNum;
    pwmRunning = false;
    pwmFreq = PWM_DEFAULT_FREQ;
    pwmDuty = PWM_DEFAULT_DUTY;
}

GPIOPwmPin::~GPIOPwmPin(void)
{
    stopPWM();
}

GPIO_Result GPIOPwmPin::setPWM(int freq, int duty) {
    if (freq <= 0) {
        return stopPWM();
    }
    
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        return res;
    }
    if ((dir != GPIO_OUTPUT) || (pwmDuty < 0) || (pwmDuty > 100)) {
        return GPIO_INVALID_OP;
    }
    pwmFreq = freq;
    pwmDuty = duty;

    // find the period (in ms)
    double period = (1.0f/((double)pwmFreq)) * 1000;
    double periodLow;
    double periodHigh;

    // find the low and high periods based on the duty-cycle
    periodHigh = period * ((double)pwmDuty / 100.0f);
    periodLow = period - periodHigh; //can also be: period * (1.0f - dutyCycle);

    // Set high and low times in micro seconds
    pwmMicroSecHigh = (int)(periodHigh * 1000);	
    pwmMicroSecLow = (int)(periodLow * 1000);	

    if (!pwmRunning) {
        int pres = pthread_create(&pwmThread, NULL, &GPIOPwmPin::pwmThreadRunner, this);
    }
    return GPIO_OK;
}

GPIO_Result GPIOPwmPin::startPWM() {
    return setPWM(pwmFreq, pwmDuty);
}

void * GPIOPwmPin::runPWM() {
    pwmRunning = true;
    while (pwmRunning) {
        // HIGH part of cycle
        GPIOAccess::set(pinNumber, 1);
	usleep(pwmMicroSecHigh);

        // LOW part of cycle
        GPIOAccess::set(pinNumber, 0);
	usleep(pwmMicroSecLow);
    }

    return NULL;
}

GPIO_Result GPIOPwmPin::stopPWM() {
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        return res;
    }
    
    if ((dir != GPIO_OUTPUT) || !pwmRunning) {
        return GPIO_INVALID_OP;
    }
    if (pwmRunning) {
        pwmRunning = false;
        pthread_join(pwmThread, NULL);
    }
    return GPIO_OK;
}

void * GPIOPwmPin::pwmThreadRunner(void * pvPin) {
    GPIOPwmPin * pPin = (GPIOPwmPin *)pvPin;
    return pPin->runPWM();
}

int GPIOPwmPin::getPWMFreq() {
    return pwmFreq;
}

int GPIOPwmPin::getPWMDuty() {
    return pwmDuty;
}

bool GPIOPwmPin::isPWMRunning() {
    return pwmRunning;
}

int GPIOPwmPin::getPinNumber() {
    return pinNumber;
}
