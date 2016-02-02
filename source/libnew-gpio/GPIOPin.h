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
    
    void setPWM(int freq, int duty);
    void startPWM();
    void stopPWM();
    
    int getPWMFreq();
    int getPWMDuty();
    bool isPWMRunning();

    int getPinNumber();
    
    GPIO_Result getLastResult();
private:
    int pinNumber;
    GPIO_Result lastResult;
};

#endif
