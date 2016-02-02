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

void GPIOPin::setPWM(int freq, int duty) {
    GPIOAccess::setPWM(pinNumber, freq, duty);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::startPWM() {
    GPIOAccess::startPWM(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

void GPIOPin::stopPWM() {
    GPIOAccess::stopPWM(pinNumber);
    lastResult = GPIOAccess::getLastResult();
}

int GPIOPin::getPWMFreq() {
    return GPIOAccess::getPWMFreq(pinNumber);
}

int GPIOPin::getPWMDuty() {
    return GPIOAccess::getPWMDuty(pinNumber);
}

bool GPIOPin::isPWMRunning() {
    return GPIOAccess::isPWMRunning(pinNumber);
}

int GPIOPin::getPinNumber() {
    return pinNumber;
}

GPIO_Result GPIOPin::getLastResult() {
    return lastResult;
}
