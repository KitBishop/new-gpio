#include <unistd.h>
#include <stdio.h>

#include "GPIOPulseInPin.h"
#include "GPIOAccess.h"

GPIOPulseInPin::GPIOPulseInPin(int pinNum)
{
    pinNumber = pinNum;
    pulseInRunning = false;

    piPulseLevel = 1;
    piTimeoutUS = 0;

    handlerFunc = NULL;
    handlerObj = NULL;
    lastResult = GPIO_OK;
}

GPIOPulseInPin::~GPIOPulseInPin(void)
{
    stopPulseIn();
}

long int GPIOPulseInPin::pulseIn(int pulseLevel, long int timeoutUS) {
    long int val = -1;
    
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        lastResult = res;
        return val;
    }
    if (dir != GPIO_INPUT) {
        lastResult = GPIO_INVALID_OP;
        return val;
    }
    int offV;
    int onV;
    long int elapsedUS = 0;
    long int startUS = 0;
    bool timedOut = false;
    if (pulseLevel == 0) {
        offV = 1;
        onV = 0;
    } else {
        offV = 0;
        onV = 1;
    }

    pulseInRunning = true;
    
    // wait until offV
    startUS = timeNowUS();
    while ((GPIOAccess::rawGet(pinNumber) != offV) && pulseInRunning && !timedOut) {
        sleepMicro(1LL);
        if (timeoutUS != 0) {
            elapsedUS = timeNowUS() - startUS;
            if (elapsedUS > timeoutUS) {
                timedOut = true;
            }
        }
    }

    // wait until onV
    startUS = timeNowUS();
    while ((GPIOAccess::rawGet(pinNumber) != onV) && pulseInRunning && !timedOut) {
        sleepMicro(1LL);
        if (timeoutUS != 0) {
            elapsedUS = timeNowUS() - startUS;
            if (elapsedUS > timeoutUS) {
                timedOut = true;
            }
        }
    }

    // wait until offV counting micro secs
    val = 0;
    startUS = timeNowUS();
    while ((GPIOAccess::rawGet(pinNumber) != offV) && pulseInRunning && !timedOut) {
        sleepMicro(1LL);
        val++;
        if (timeoutUS != 0) {
            elapsedUS = timeNowUS() - startUS;
            if (elapsedUS > timeoutUS) {
                timedOut = true;
            }
        }
    }
    if (pulseInRunning && !timedOut) {
        val = timeNowUS() - startUS;
    }

    if (timedOut) {
        res = GPIO_TIME_OUT;
    }
    if (res != GPIO_OK) {
        val = -1L;
    }
    
    if (!pulseInRunning) {
        val = -1;
    }
    
    if (pulseInRunning && (res == GPIO_OK)) {
        if (handlerFunc != NULL) {
            handlerFunc(pinNumber, val);
        } else if (handlerObj != NULL) {
            handlerObj->handlePulseIn(pinNumber, val);
        }
    }
    
    pulseInRunning = false;
    
    lastResult = res;
    return val;
}

void GPIOPulseInPin::pulseIn(GPIO_PulseIn_Handler_Func handler, int pulseLevel, long int timeoutUS) {
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        lastResult = res;
        return;
    }
    if (dir != GPIO_INPUT) {
        lastResult = GPIO_INVALID_OP;
        return;
    }
    
    handlerFunc = handler;
    handlerObj = NULL;
    piPulseLevel = pulseLevel;
    piTimeoutUS = timeoutUS;

    if (!pulseInRunning) {
        int pres = pthread_create(&pulseInThread, NULL, &GPIOPulseInPin::pulseInPinThreadRunner, this);
    }
    
    lastResult = GPIO_OK;
}

void GPIOPulseInPin::pulseIn(GPIO_PulseIn_Handler_Object * handlerObj, int pulseLevel, long int timeoutUS) {
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        lastResult = res;
        return;
    }
    if (dir != GPIO_INPUT) {
        lastResult = GPIO_INVALID_OP;
        return;
    }
    
    handlerFunc = NULL;
    handlerObj = handlerObj;
    piPulseLevel = pulseLevel;
    piTimeoutUS = timeoutUS;

    if (!pulseInRunning) {
        int pres = pthread_create(&pulseInThread, NULL, &GPIOPulseInPin::pulseInPinThreadRunner, this);
    }
    
    lastResult = GPIO_OK;
}

void * GPIOPulseInPin::runPulseIn() {
    pulseIn(piPulseLevel, piTimeoutUS);

    return NULL;
}

void GPIOPulseInPin::stopPulseIn() {
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    GPIO_Result res = GPIOAccess::getLastResult();
    if (res != GPIO_OK) {
        lastResult = res;
        return;
    }
    
    if ((dir != GPIO_INPUT) || !pulseInRunning) {
        lastResult = GPIO_INVALID_OP;
    }
    
    if (pulseInRunning) {
        pulseInRunning = false;
        pthread_join(pulseInThread, NULL);
    }

    lastResult = GPIO_OK;
}

void * GPIOPulseInPin::pulseInPinThreadRunner(void * pvPin) {
    GPIOPulseInPin * pPin = (GPIOPulseInPin *)pvPin;
    return pPin->runPulseIn();
}

bool GPIOPulseInPin::isPulseInRunning() {
    return pulseInRunning;
}

int GPIOPulseInPin::getPinNumber() {
    return pinNumber;
}

GPIO_Result GPIOPulseInPin::getLastResult() {
    return lastResult;
}
