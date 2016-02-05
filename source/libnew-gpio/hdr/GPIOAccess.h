#ifndef GPIOAccess_H
#define GPIOAccess_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "GPIOTypes.h"
#include "GPIOPwmPin.h"
#include "GPIOIrqInfo.h"

#define numPins 27

class GPIOAccess {
public:
    static void setDirection(int pinNum, GPIO_Direction dir);
    static GPIO_Direction getDirection(int pinNum);
    static void set(int pinNum, int value);
    static int get(int pinNum);
    
    static void setIrq(int pinNum, GPIO_Irq_Type type, GPIO_Irq_Handler_Func handler, long int debounceMs = 0);
    static void setIrq(int pinNum, GPIO_Irq_Type type, GPIO_Irq_Handler_Object * handlerObj, long int debounceMs = 0);
    static void resetIrq(int pinNum);
    static void enableIrq(int pinNum);
    static void disableIrq(int pinNum);
    static void enableIrq(int pinNum, bool enable);
    static bool irqEnabled(int pinNum);
    static GPIO_Irq_Type getIrqType(int pinNum);
    static GPIO_Irq_Handler_Func getIrqHandler(int pinNum);
    static GPIO_Irq_Handler_Object * getIrqHandlerObj(int pinNum);
    
    static void enableIrq();
    static void disableIrq();
    static void enableIrq(bool enable);
    static bool irqEnabled();
    
    static void setPWM(int pinNum, int freq, int duty);
    static void startPWM(int pinNum);
    static void stopPWM(int pinNum);
    
    static int getPWMFreq(int pinNum);
    static int getPWMDuty(int pinNum);
    static bool isPWMRunning(int pinNum);
    
    static bool isPinUsable(int pinNum);
    static bool isAccessOk();
    
    static GPIO_Result getLastResult();
private:
    static GPIO_Result checkAndSetupAddress(unsigned long int blockBaseAddr, unsigned long int blockSize);
    static GPIO_Result checkPinAndAccess(int pin);
    static void setupIrqHandler();
    static void irqHandler(int n, siginfo_t *info, void *unused);
    
    static GPIO_Direction getDirectionImpl(int pinNum);
    
    static void writeReg(unsigned long int registerOffset, unsigned long int value);
    static unsigned long int readReg(unsigned long int registerOffset);

    static void setBit(unsigned long int &regVal, int bitNum, int value);
    static int getBit(unsigned long int regVal, int bitNum);

    static volatile unsigned long int *regAddress;
    
    static bool pinUsable[numPins];
    
    static GPIOPwmPin * pwmPin[numPins];
    static bool pwmSetup;
    
    static GPIOIrqInf irqInf[numPins];
    static bool irqSetup;
    
    static void * irqThreadRunner(void * unused);
    static void * runIRQ();
    static pthread_t irqThread;
    static bool irqRunning;
    static sigset_t irqSigset;
    static siginfo_t irqSiginfo;
    static bool irqIsEnabled;
    
    static GPIO_Result lastResult;
};

#endif
