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
#include "GPIOPulseInPin.h"
#include "TimeHelper.h"

#define numPins 27
#define libraryVersion  "1.4.0"

class GPIOAccess {
    friend GPIOPulseInPin;
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
    
    static void setPWM(int pinNum, long int freq, int duty, int durationMs = 0);
    static void startPWM(int pinNum, int durationMs = 0);
    static void stopPWM(int pinNum);
    
    static long int getPWMFreq(int pinNum);
    static int getPWMDuty(int pinNum);
    static int getPWMDuration(int pinNum);
    static bool isPWMRunning(int pinNum);

    static void setTone(int pinNum, long int freq, int durationMs = 0);
    static void startTone(int pinNum, int durationMs = 0);
    static void stopTone(int pinNum);
    static long int getToneFreq(int pinNum);
    static int getToneDuration(int pinNum);
    static bool isToneRunning(int pinNum);
    
    static void shiftOut(int dataPinNum, int clockPinNum, int val, long int clockPeriodNS, GPIO_Bit_Order bitOrder = GPIO_MSB_FIRST);
    static int shiftIn(int dataPinNum, int clockPinNum, long int clockPeriodNS, GPIO_Bit_Order bitOrder = GPIO_MSB_FIRST);
    
    static void pulseOut(int pinNum, long int pulseLenUS, int pulseLevel = 1);

    static long int pulseIn(int pinNum, int pulseLevel = 1, long int timeoutUS = 0);
    static void pulseIn(int pinNum, GPIO_PulseIn_Handler_Func handler, int pulseLevel = 1, long int timeoutUS = 0);
    static void pulseIn(int pinNum, GPIO_PulseIn_Handler_Object * handlerObj, int pulseLevel = 1, long int timeoutUS = 0);
    static void stopPulseIn(int pinNum);
    static bool isPulseInRunning(int pinNum);
    
    static long int getFrequency(int pinNum, long int sampleTimeMS);
    
    static bool isPinUsable(int pinNum);
    static bool isAccessOk();
    
    static GPIO_Result getLastResult();

    static void rawSet(int pinNum, int value);
    static int rawGet(int pinNum);
    
    static char * getLibVersion();
private:
    static GPIO_Result checkAndSetupAddress(unsigned long int blockBaseAddr, unsigned long int blockSize);
    static GPIO_Result checkPinAndAccessDir(int pin, GPIO_Direction dir);
    static GPIO_Result checkPinAndAccess(int pin);
    
    static void createPwmPinIfNeeded(int pin);
    static void createIrqInfIfNeeded(int pin);
    static void createPulseInPinIfNeeded(int pin);
    
    static void setupIrqHandler();
    static void irqHandler(int n, siginfo_t *info, void *unused);

    static void setPWMInternal(int pinNum, long int freq, int duty, bool isTone, int durationMs);
    static void startPWMInternal(int pinNum, bool isTone, int durationMs);
    
    static GPIO_Direction getDirectionImpl(int pinNum);
    
    static void setBit(unsigned long int &regVal, int bitNum, int value);
    static int getBit(unsigned long int regVal, int bitNum);

    static volatile unsigned long int *regAddress;
    
    static long int pinUsable;
    
    static GPIOPwmPin * pwmPin[numPins];
    static bool pwmSetup;
    
    static GPIOPulseInPin * pulseInPin[numPins];
    static bool pulseInSetup;
    
    static GPIOIrqInf * irqInf[numPins];
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
