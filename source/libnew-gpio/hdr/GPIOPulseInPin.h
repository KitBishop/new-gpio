#ifndef GPIOPulseInPin_H
#define GPIOPulseInPin_H

#include <pthread.h>
#include "GPIOTypes.h"

class GPIOPulseInPin {
public:
    GPIOPulseInPin(int pinNum);
    ~GPIOPulseInPin(void);

    long int pulseIn(int pulseLevel = 1, long int timeoutUS = 0);
    void pulseIn(GPIO_PulseIn_Handler_Func handler, int pulseLevel = 1, long int timeoutUS = 0);
    void pulseIn(GPIO_PulseIn_Handler_Object * handlerObj, int pulseLevel = 1, long int timeoutUS = 0);
    void stopPulseIn();
    bool isPulseInRunning();

    int getPinNumber();
    
    GPIO_Result getLastResult();
private:
    static void * pulseInPinThreadRunner(void * pvPin);

    void * runPulseIn();

    int pinNumber;
    pthread_t pulseInThread;
    int piPulseLevel;
    long int piTimeoutUS;
    bool pulseInRunning;
    GPIO_PulseIn_Handler_Func handlerFunc;
    GPIO_PulseIn_Handler_Object * handlerObj;

    GPIO_Result lastResult;
};



#endif
