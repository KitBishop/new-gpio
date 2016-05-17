#ifndef GPIOPin_h
#define GPIOPin_h

#include <pthread.h>
#include "GPIOTypes.h"

class GPIOPin {
public:
    GPIOPin(int pinNum);
    ~GPIOPin(void);

    void setDirection(GPIO_Direction dir);
    GPIO_Direction getDirection();

    void set(int value);
    int get();

    void setIrq(GPIO_Irq_Type type, GPIO_Irq_Handler_Func handler, long int debounceMs =0);
    void setIrq(GPIO_Irq_Type type, GPIO_Irq_Handler_Object * handlerObj, long int debounceMs = 0);
    void resetIrq();
    void enableIrq();
    void disableIrq();
    void enableIrq(bool enable);
    bool irqEnabled();
    GPIO_Irq_Type getIrqType();
    GPIO_Irq_Handler_Func getIrqHandler();
    GPIO_Irq_Handler_Object * getIrqHandlerObj();
    
    void setPWM(long int freq, int duty, int durationMs = 0);
    void startPWM(int durationMs = 0);
    void stopPWM();
    
    long int getPWMFreq();
    int getPWMDuty();
    int getPWMDuration();
    bool isPWMRunning();

    void setTone(long int freq, int durationMs = 0);
    void startTone(int durationMs = 0);
    void stopTone();
    long int getToneFreq();
    int getToneDuration();
    bool isToneRunning();

    void pulseOut(long int pulseLenUS, int pulseLevel = 1);

    long int pulseIn(int pulseLevel = 1, long int timeoutUS = 0);
    void pulseIn(GPIO_PulseIn_Handler_Func handler, int pulseLevel = 1, long int timeoutUS = 0);
    void pulseIn(GPIO_PulseIn_Handler_Object * handlerObj, int pulseLevel = 1, long int timeoutUS = 0);
    void stopPulseIn();
    bool isPulseInRunning();
    
    long int getFrequency(long int sampleTimeMS);
    
    int getPinNumber();
    
    GPIO_Result getLastResult();
private:
    int pinNumber;
    GPIO_Result lastResult;
};

#endif
