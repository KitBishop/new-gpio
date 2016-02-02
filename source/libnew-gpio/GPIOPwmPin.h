#ifndef GPIOPwmPin_h
#define GPIOPwmPin_h

#include <pthread.h>
#include "GPIOTypes.h"

#define PWM_DEFAULT_FREQ 200
#define PWM_DEFAULT_DUTY 50

class GPIOPwmPin {
public:
    GPIOPwmPin(int pinNum);
    ~GPIOPwmPin(void);

    GPIO_Result setPWM(int freq, int duty);
    GPIO_Result startPWM();
    GPIO_Result stopPWM();
    
    int getPWMFreq();
    int getPWMDuty();
    bool isPWMRunning();
    int getPinNumber();
    
private:
    static void * pwmThreadRunner(void * pvPin);

    void * runPWM();

    int pinNumber;
    pthread_t pwmThread;
    int pwmFreq;
    int pwmDuty;
    int pwmMicroSecHigh;
    int pwmMicroSecLow;
    bool pwmRunning;
};

#endif
