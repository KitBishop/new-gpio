#include "GPIOPin.h"
#include "GPIOAccess.h"

GPIOPin::GPIOPin(int pinNum)
{
    pinNumber = pinNum;
    lastResult = GPIO_OK;
}

GPIOPin::~GPIOPin(void)
{
    stopPWM();
}

void GPIOPin::setDirection(GPIO_Direction dir)
{
    GPIOAccess::setDirection(pinNumber, dir);
    lastResult = GPIOAccess::getLastResult();
}

GPIO_Direction GPIOPin::getDirection()
{
    GPIO_Direction dir = GPIOAccess::getDirection(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return dir;
}

void GPIOPin::set(int value)
{
    if (GPIOAccess::isPWMRunning(pinNumber)) {
        lastResult = GPIO_INVALID_OP;
    } else {
        GPIOAccess::set(pinNumber, value);
        lastResult = GPIOAccess::getLastResult();
    }
}

int GPIOPin::get()
{
    int value = GPIOAccess::get(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    
    return value;
}

void GPIOPin::setIrq(GPIO_Irq_Type type, GPIO_Irq_Handler_Func handler, long int debounceMs) {
    GPIOAccess::setIrq(pinNumber, type, handler, debounceMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::setIrq(GPIO_Irq_Type type, GPIO_Irq_Handler_Object * handlerObj, long int debounceMs) {
    GPIOAccess::setIrq(pinNumber, type, handlerObj, debounceMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::resetIrq() {
    GPIOAccess::resetIrq(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::enableIrq() {
    enableIrq(true);
}

void GPIOPin::disableIrq() {
    enableIrq(false);
}

void GPIOPin::enableIrq(bool enable) {
    GPIOAccess::enableIrq(pinNumber, enable);
    lastResult = GPIOAccess::getLastResult();
}

bool GPIOPin::irqEnabled() {
    bool ret = GPIOAccess::irqEnabled(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return ret;
}

GPIO_Irq_Type GPIOPin::getIrqType(){
    GPIO_Irq_Type type = GPIOAccess::getIrqType(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return type;
}

GPIO_Irq_Handler_Func GPIOPin::getIrqHandler() {
    GPIO_Irq_Handler_Func handler = GPIOAccess::getIrqHandler(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return handler;
}

GPIO_Irq_Handler_Object * GPIOPin::getIrqHandlerObj() {
    GPIO_Irq_Handler_Object * handlerObj = GPIOAccess::getIrqHandlerObj(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return handlerObj;
}

void GPIOPin::setPWM(long int freq, int duty, int durationMs) {
    GPIOAccess::setPWM(pinNumber, freq, duty, durationMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::startPWM(int durationMs) {
    GPIOAccess::startPWM(pinNumber, durationMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::stopPWM() {
    GPIOAccess::stopPWM(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

long int GPIOPin::getPWMFreq() {
    long int freq = GPIOAccess::getPWMFreq(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return freq;
}

int GPIOPin::getPWMDuty() {
    int duty = GPIOAccess::getPWMDuty(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return duty;
}

int GPIOPin::getPWMDuration() {
    int duration = GPIOAccess::getPWMDuration(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return duration;
}

bool GPIOPin::isPWMRunning() {
    bool running = GPIOAccess::isPWMRunning(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return running;
}

int GPIOPin::getPinNumber() {
    return pinNumber;
}

GPIO_Result GPIOPin::getLastResult() {
    return lastResult;
}

void GPIOPin::setTone(long int freq, int durationMs) {
    GPIOAccess::setTone(pinNumber, freq, durationMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::startTone(int durationMs) {
    GPIOAccess::startTone(pinNumber, durationMs);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::stopTone() {
    GPIOAccess::stopTone(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

long int GPIOPin::getToneFreq() {
    int freq = GPIOAccess::getToneFreq(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return freq;
}

int GPIOPin::getToneDuration() {
    int duration = GPIOAccess::getToneDuration(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return duration;
}

bool GPIOPin::isToneRunning() {
    bool running = GPIOAccess::isToneRunning(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return running;
}

void GPIOPin::pulseOut(long int pulseLenUS, int pulseLevel) {
    GPIOAccess::pulseOut(pinNumber, pulseLenUS, pulseLevel);
    lastResult = GPIOAccess::getLastResult();
}

long int GPIOPin::pulseIn(int pulseLevel, long int timeoutUS) {
    long int val = GPIOAccess::pulseIn(pinNumber, pulseLevel, timeoutUS);
    lastResult = GPIOAccess::getLastResult();
    return val;
}

void GPIOPin::pulseIn(GPIO_PulseIn_Handler_Func handler, int pulseLevel, long int timeoutUS) {
    GPIOAccess::pulseIn(pinNumber, handler, pulseLevel, timeoutUS);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::pulseIn(GPIO_PulseIn_Handler_Object * handlerObj, int pulseLevel, long int timeoutUS) {
    GPIOAccess::pulseIn(pinNumber, handlerObj, pulseLevel, timeoutUS);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::stopPulseIn() {
    GPIOAccess::stopPulseIn(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

bool GPIOPin::isPulseInRunning() {
    bool running = GPIOAccess::isPulseInRunning(pinNumber);
    lastResult = GPIOAccess::getLastResult();
    return running;
}

long int GPIOPin::getFrequency(long int sampleTimeMS) {
    long int freq = GPIOAccess::getFrequency(pinNumber, sampleTimeMS);
    lastResult = GPIOAccess::getLastResult();
    return freq;
}
