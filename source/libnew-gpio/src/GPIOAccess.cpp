#include "GPIOAccess.h"

#define REGISTER_BLOCK_ADDR     0x18040000
#define REGISTER_BLOCK_SIZE	0x30

#define REGISTER_OE_OFFSET	0
#define REGISTER_IN_OFFSET	1
#define REGISTER_OUT_OFFSET	2
#define REGISTER_SET_OFFSET	3
#define REGISTER_CLEAR_OFFSET	4
#define REGISTER_IRQ_ENABLE     5
#define REGISTER_IRQ_TYPE       6
#define REGISTER_IRQ_POLARITY   7
#define REGISTER_IRQ_PENDING    8
#define REGISTER_IRQ_MASK       9

GPIOPwmPin * GPIOAccess::pwmPin[numPins];

bool GPIOAccess::pwmSetup = false;
bool GPIOAccess::pinUsable[numPins] = {
    true, // 0
    true, // 1
    false, // 2
    false, // 3
    false, // 4
    false, // 5
    true, // 6
    true, // 7
    true, // 8
    false, // 9
    false, // 10
    false, // 11
    true, // 12
    true, // 13
    true, // 14
    true, // 15
    true, // 16
    true, // 17
    true, // 18
    true, // 19
    false, // 20
    false, // 21
    false, // 22
    true, // 23
    false, // 24
    false, // 25
    true, // 26
};

GPIOIrqInf GPIOAccess::irqInf[numPins];
bool GPIOAccess::irqSetup = false;
pthread_t GPIOAccess::irqThread;
bool GPIOAccess::irqRunning = false;
sigset_t GPIOAccess::irqSigset;
siginfo_t GPIOAccess::irqSiginfo;
//#define SIG_GPIO_IRQ	(SIGRTMIN+10)	// SIGRTMIN is different in Kernel and User modes
#define SIG_GPIO_IRQ	42				// So we have to hardcode this value
bool GPIOAccess::irqIsEnabled = true;

GPIO_Result GPIOAccess::lastResult = GPIO_OK;

volatile unsigned long int *GPIOAccess::regAddress = NULL;

GPIO_Result GPIOAccess::checkPinAndAccess(int pin) {
    if (!isPinUsable(pin)) {
        return GPIO_INVALID_PIN;
    }
    
    return checkAndSetupAddress(REGISTER_BLOCK_ADDR, REGISTER_BLOCK_SIZE);
}

GPIO_Result GPIOAccess::checkAndSetupAddress(unsigned long int blockBaseAddr, unsigned long int blockSize)
{
    if (regAddress != NULL) {
        if (regAddress == MAP_FAILED)
        {
                return GPIO_BAD_ACCESS;	// maybe return -2
        }

        return GPIO_OK;	// regAddress is now populated
    }
    
    int  m_mfd;

    if ((m_mfd = open("/dev/mem", O_RDWR)) < 0)
    {
            return GPIO_BAD_ACCESS;	// maybe return -1
    }

    regAddress = (unsigned long*)mmap(NULL, 
                                    blockSize, 
                                    PROT_READ|PROT_WRITE, 
                                    MAP_SHARED, 
                                    m_mfd, 
                                    blockBaseAddr
                            );
    close(m_mfd);

    if (regAddress == MAP_FAILED)
    {
            return GPIO_BAD_ACCESS;	// maybe return -2
    }

    if (!pwmSetup) {
        int i;
        for (i = 0; i < numPins; i++) {
            if (isPinUsable(i)) {
                pwmPin[i] = new GPIOPwmPin(i);
            } else {
                pwmPin[i] = NULL;
            }
        }
        pwmSetup = true;
        for (i = 0; i < numPins; i++) {
            if (isPinUsable(i)) {
                pwmPin[i] = new GPIOPwmPin(i);
            } else {
                pwmPin[i] = NULL;
            }
        }
        pwmSetup = true;
    }
    
    if (!irqSetup) {
        int i;
        for (i = 0; i < numPins; i++) {
            irqInf[i].type = GPIO_IRQ_NONE;
            irqInf[i].handlerFunc = NULL;
            irqInf[i].handlerObj = NULL;
            irqInf[i].enabled = false;
            irqInf[i].debounceMs = 0;
            irqInf[i].lastTimeMs = 0;
            irqInf[i].lastVal = 0;
        }
        setupIrqHandler();
        irqSetup = true;
    }
    
    return GPIO_OK;	// regAddress is now populated
}

void * GPIOAccess::irqThreadRunner(void * unused) {
    return runIRQ();
}

void * GPIOAccess::runIRQ() {
    irqRunning = true;
    while (irqRunning)
    {
        sigwaitinfo(&irqSigset, &irqSiginfo);
    }
    return NULL;
}

void GPIOAccess::irqHandler(int n, siginfo_t *info, void *unused)
{
    // Ignore if irq not enabled
    if (!irqIsEnabled) {
        return;
    }

    // Get type of change and pin it applies to
    int pin = -1;
    int val = 0;
    GPIO_Irq_Type type = GPIO_IRQ_NONE;
    pin = (info->si_int >> 24) & 0xff;
    val = info->si_int & 0xff;
    if (val == 0) {
        type = GPIO_IRQ_FALLING;
    } else {
        type = GPIO_IRQ_RISING;
        val = 1;
    }

    // Only process if type matches that for pin and irq is enabled for pin
    if (((irqInf[pin].type == type) || (irqInf[pin].type == GPIO_IRQ_BOTH)) && irqInf[pin].enabled) {
        bool wantIrq = true;
        
        // Check for change in pin value in GPIO_IRQ_BOTH case
        if (irqInf[pin].type == GPIO_IRQ_BOTH) {
            wantIrq = (val != irqInf[pin].lastVal);
        }
        
        // Check for any debounce to be applied
        if (wantIrq && (irqInf[pin].debounceMs > 0)) {
            timeval timeNow;
            gettimeofday(&timeNow, NULL);
            long int nowMs = (timeNow.tv_sec * 1000L) + (timeNow.tv_usec / 1000L);        

            long int timeDiff = nowMs - irqInf[pin].lastTimeMs;

            // Don't want it if insufficient time has elapsed for debounce
            if (timeDiff < irqInf[pin].debounceMs) {
                wantIrq = false;
            }

            // Preserve last value change time
            irqInf[pin].lastTimeMs = nowMs;
        }
        
        if (wantIrq) {
            // Need to handle irq
            // Call whichever function or object is registered
            if (irqInf[pin].handlerFunc != NULL) {
                irqInf[pin].handlerFunc(pin, type);
            } else if (irqInf[pin].handlerObj != NULL) {
                irqInf[pin].handlerObj->handleIrq(pin, type);
            }

            // Preserve last applied value for GPIO_IRQ_BOTH case
            if (irqInf[pin].type == GPIO_IRQ_BOTH) {
                irqInf[pin].lastVal = val;
            }
        }
    }
}

void GPIOAccess::setupIrqHandler() {
    if (!irqRunning) {
        struct sigaction sig;
        sig.sa_sigaction = irqHandler;

        sig.sa_flags = SA_SIGINFO | SA_NODEFER;
        sigaction(SIG_GPIO_IRQ, &sig, NULL);
        
        sigemptyset(&irqSigset);
        sigaddset(&irqSigset, SIGINT);	//	Ctrl+C

        sigprocmask(SIG_BLOCK, &irqSigset, NULL);

        int pres = pthread_create(&irqThread, NULL, &irqThreadRunner, NULL);
    }
}

void GPIOAccess::enableIrq() {
    enableIrq(true);
}

void GPIOAccess::disableIrq() {
    enableIrq(false);
}

void GPIOAccess::enableIrq(bool enable) {
    irqIsEnabled = enable;
    lastResult = GPIO_OK;
}

bool GPIOAccess::irqEnabled() {
    return irqIsEnabled;
}

void GPIOAccess::enableIrq(int pinNum) {
    enableIrq(pinNum, true);
}
void GPIOAccess::disableIrq(int pinNum) {
    enableIrq(pinNum, false);
}
void GPIOAccess::enableIrq(int pinNum, bool enable) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            irqInf[pinNum].enabled = enable;
        }
    }

    lastResult = res;
}

bool GPIOAccess::irqEnabled(int pinNum) {
    bool ret = false;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            ret = irqInf[pinNum].enabled;
        }
    }

    lastResult = res;

    return ret;
}

GPIO_Irq_Type GPIOAccess::getIrqType(int pinNum) {
    GPIO_Irq_Type type = GPIO_IRQ_NONE;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            type = irqInf[pinNum].type;
        }
    }

    lastResult = res;
    
    return type;
}

GPIO_Irq_Handler_Func GPIOAccess::getIrqHandler(int pinNum) {
    GPIO_Irq_Handler_Func handler = NULL;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            handler = irqInf[pinNum].handlerFunc;
        }
    }

    lastResult = res;
    
    return handler;
}

GPIO_Irq_Handler_Object * GPIOAccess::getIrqHandlerObj(int pinNum) {
    GPIO_Irq_Handler_Object * handlerObj = NULL;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            handlerObj = irqInf[pinNum].handlerObj;
        }
    }

    lastResult = res;
    
    return handlerObj;
}

void GPIOAccess::writeReg(unsigned long int registerOffset, unsigned long int value)
{
    *(regAddress + registerOffset) = value;
}

unsigned long int GPIOAccess::readReg(unsigned long int registerOffset)
{
    unsigned long int 	value = 0x0;

    // read the value 
    value = *(regAddress + registerOffset);

    return(value);
}

// change the value of a single bit
void GPIOAccess::setBit(unsigned long int &regVal, int bitNum, int value)
{
	if (value == 1) {
		regVal |= (1 << bitNum);
	}
	else {
		regVal &= ~(1 << bitNum);
	}

	// try this out
	// regVal ^= (-value ^ regVal) & (1 << bitNum);
}

// find the value of a single bit
int GPIOAccess::getBit(unsigned long int regVal, int bitNum)
{
	int value;

	// isolate the specific bit
	value = ((regVal >> bitNum) & 0x1);

	return (value);
}

void GPIOAccess::setDirection(int pinNum, GPIO_Direction dir) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        unsigned long int regVal;

        // read the current input and output settings
        regVal = readReg(REGISTER_OE_OFFSET);

        // set the OE for this pin
        setBit(regVal, pinNum, dir);

        // write the new register value
        writeReg(REGISTER_OE_OFFSET, regVal);
    }
    
    lastResult = res;
}

GPIO_Direction GPIOAccess::getDirection(int pinNum) {
    GPIO_Direction dir = GPIO_INPUT;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        unsigned long int regVal;

        // find the OE for this pin
        dir = getDirectionImpl(pinNum);
    }
    
    lastResult = res;
    
    return dir;
}

GPIO_Direction GPIOAccess::getDirectionImpl(int pinNum) {
    unsigned long int regVal;

    // read the current input and output settings
    regVal = readReg(REGISTER_OE_OFFSET);

    // find the OE for this pin
    return getBit(regVal, pinNum) ? GPIO_OUTPUT : GPIO_INPUT;
}

void GPIOAccess::set(int pinNum, int value) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            unsigned long int 	regAddr;
            unsigned long int 	regVal;

            if (value == 0 )	{
                    // write to the clear register
                    regAddr 	= REGISTER_CLEAR_OFFSET;
            }
            else {
                    // write to the set register
                    regAddr 	= REGISTER_SET_OFFSET;
            }

            // put the desired pin value into the register 
            regVal = (0x1 << pinNum);

            // write to the register
            writeReg (regAddr, regVal);
        }
    }
    
    lastResult = res;
}

int GPIOAccess::get(int pinNum) {
    int value = 0;
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            unsigned long int 	regVal;

            // read the current value of all pins
            regVal 	= readReg (REGISTER_IN_OFFSET);

            // find the value of the specified pin
            value 	= getBit(regVal, pinNum);
        }
    }
    
    lastResult = res;
    
    return value;
}

void GPIOAccess::setIrq(int pinNum, GPIO_Irq_Type type, GPIO_Irq_Handler_Func handler, long int debounceMs) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
    
            lastResult = res;
        } else {
            resetIrq(pinNum);
            if (type != GPIO_IRQ_NONE) {
                int fd;
                char buf[100];

                fd=open("/sys/kernel/debug/gpio-irq", O_WRONLY);
                if(fd < 0)
                {
                    perror("open");
                    res = GPIO_INVALID_OP;
                } else {
                    sprintf(buf, "+ %d %i", pinNum, getpid());

                    if(write(fd, buf, strlen(buf) + 1) < 0)
                    {
                        perror("write");
                        res = GPIO_INVALID_OP;
                    } else {
                        irqInf[pinNum].type = type;
                        irqInf[pinNum].handlerFunc = handler;
                        irqInf[pinNum].handlerObj = NULL;
                        irqInf[pinNum].enabled = true;
                        irqInf[pinNum].debounceMs = debounceMs;
                        irqInf[pinNum].lastTimeMs = 0;
                        irqInf[pinNum].lastVal = 0;
                        if (!irqRunning) {
                            setupIrqHandler();
                        }
                    }

                    close(fd);
                }
            }
        }
    
        lastResult = res;
    }
}

void GPIOAccess::setIrq(int pinNum, GPIO_Irq_Type type, GPIO_Irq_Handler_Object * handlerObj, long int debounceMs) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
    
            lastResult = res;
        } else {
            resetIrq(pinNum);
            if (type != GPIO_IRQ_NONE) {
                int fd;
                char buf[100];

                fd=open("/sys/kernel/debug/gpio-irq", O_WRONLY);
                if(fd < 0)
                {
                    perror("open");
                    res = GPIO_INVALID_OP;
                } else {
                    sprintf(buf, "+ %d %i", pinNum, getpid());

                    if(write(fd, buf, strlen(buf) + 1) < 0)
                    {
                        perror("write");
                        res = GPIO_INVALID_OP;
                    } else {
                        irqInf[pinNum].type = type;
                        irqInf[pinNum].handlerFunc = NULL;
                        irqInf[pinNum].handlerObj = handlerObj;
                        irqInf[pinNum].enabled = true;
                        irqInf[pinNum].debounceMs = debounceMs;
                        irqInf[pinNum].lastTimeMs = 0;
                        irqInf[pinNum].lastVal = 0;
                        if (!irqRunning) {
                            setupIrqHandler();
                        }
                    }

                    close(fd);
                }
            }
        }
    
        lastResult = res;
    }
}

void GPIOAccess::resetIrq(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_INPUT) {
            res = GPIO_INVALID_OP;
        } else {
            int fd;
            char buf[100];

            fd=open("/sys/kernel/debug/gpio-irq", O_WRONLY);
            if(fd < 0)
            {
                perror("open");
                res = GPIO_INVALID_OP;
            } else {
                sprintf(buf, "- %d %i", pinNum, getpid());

                if(write(fd, buf, strlen(buf) + 1) < 0)
                {
                    perror("write");
                    res = GPIO_INVALID_OP;
                } else {
                    irqInf[pinNum].type = GPIO_IRQ_NONE;
                    irqInf[pinNum].handlerFunc = NULL;
                    irqInf[pinNum].handlerObj = NULL;
                    irqInf[pinNum].enabled = false;
                    irqInf[pinNum].debounceMs = 0;
                    irqInf[pinNum].lastTimeMs = 0;
                    irqInf[pinNum].lastVal = 0;
                }

                close(fd);
            }
        }
    }
    
    lastResult = res;
}

bool GPIOAccess::isPinUsable(int pin) {
    if ((pin < 0) || (pin >= numPins)) {
        return false;
    }
    
    return pinUsable[pin];
}

bool GPIOAccess::isAccessOk() {
    return checkAndSetupAddress(REGISTER_BLOCK_ADDR, REGISTER_BLOCK_SIZE) == GPIO_OK;
}

void GPIOAccess::setPWM(int pinNum, int freq, int duty) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            res = pwmPin[pinNum]->setPWM(freq, duty);

            if (res == GPIO_OK) {
                // Wait till thread has actually started PWM
                while (!pwmPin[pinNum]->isPWMRunning()) {
                }
            }
        }
    }

    lastResult = res;
}

void GPIOAccess::startPWM(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            res = pwmPin[pinNum]->startPWM();
        }
    }
    
    lastResult = res;
}

void GPIOAccess::stopPWM(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            res = pwmPin[pinNum]->stopPWM();
        }
    }

    lastResult = res;
}

int GPIOAccess::getPWMFreq(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    int freq = 0;
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            freq = pwmPin[pinNum]->getPWMFreq();
        }
    }
    
    lastResult = res;
    return freq;
}

int GPIOAccess::getPWMDuty(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    int duty = 0;
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            duty = pwmPin[pinNum]->getPWMDuty();
        }
    }
    
    lastResult = res;
    return duty;
}

bool GPIOAccess::isPWMRunning(int pinNum) {
    GPIO_Result res = checkPinAndAccess(pinNum);
    bool running = false;
    if (res == GPIO_OK) {
        if (getDirectionImpl(pinNum) != GPIO_OUTPUT) {
            res = GPIO_INVALID_OP;
        } else {
            running = pwmPin[pinNum]->isPWMRunning();
        }
    }
    
    lastResult = res;
    return running;
}

GPIO_Result GPIOAccess::getLastResult() {
    return lastResult;
}
