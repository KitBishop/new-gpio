#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include "GPIOPin.h"
#include "RGBLED.h"
#include "GPIOAccess.h"

using namespace std;

#define numPins 27

void usage(char * nm) {
    printf("Usage\n");
    printf("Commands - one of:\n");

    printf("\t%s set-input <pin>\n",nm);
    printf("\t\tSets pin to be an input pin\n");
    printf("\t%s set-output <pin>\n",nm);
    printf("\t\tSets pin to be an output pin\n");
    printf("\t%s get-direction <pin>\n",nm);
    printf("\t\tGets and returns pin direction\n");
    printf("\t%s read <pin>\n",nm);
    printf("\t\tGets and returns input pin value\n");
    printf("\t%s set <pin> <val>\n",nm);
    printf("\t\tSets output pin value\n");


    printf("\t%s pwm <pin> <freq> <duty>\n",nm);
    printf("\t\tStarts PWM output on pin\n");
    printf("\t%s pwmstop <pin>\n",nm);
    printf("\t\tStops PWM output on pin\n");
    printf("\t%s irq <pin> <irqtype> <irqcmd> <debounce>\n",nm);
    printf("\t\tEnables IRQ handling on pin\n");
    printf("\t%s irqstop <pin>\n",nm);
    printf("\t\tTerminates IRQ handling on pin\n");

    printf("\t%s expled <ledhex>\n",nm);
    printf("\t\tStarts output to expansion led\n");
    printf("\t%s expledstop\n",nm);
    printf("\t\tTerminates output to expansion led\n");

    printf("\t%s info <pin>\n",nm);
    printf("\t\tDisplays information on pin(s)\n");
    printf("\t%s help\n",nm);
    printf("\t\tDisplays this help information\n");

    printf("Where:\n");
    printf("\t<pin> is one of\n\t\t0, 1, 6, 7, 8, 12, 13, 14, 15, 16, 17, 18, 19, 23, 26, all\n");
    printf("\t\tA <pin> of all can only be used for:\n");
    printf("\t\t\tinfo, set-input, set-output, set\n");
    printf("\t<val> is only required for set:\n");
    printf("\t\t<val> is 0 or 1\n");
    printf("\t<freq> is PWM frequency in Hz > 0\n");
    printf("\t<duty> is PWM duty cycle %% in range 0 to 100\n");
    printf("\t<irqtype> is the type for IRQ and is one of:\n");
    printf("\t\tfalling, rising, both\n");
    printf("\t<irqcmd> is the shell command to be executed when the IRQ occurs\n");
    printf("\t\tMust be enclosed in \" characters if it contains\n");
    printf("\t\tspaces or other special characters\n");
    printf("\t\tIf it starts with the string [debug],\n");
    printf("\t\tdebug output is displayed first\n");
    printf("\t<debounce> is optional debounce time for IRQ in milliseconds\n");
    printf("\t\tDefaults to 0 if not supplied\n");
    printf("\t<ledhex> specifies the hex value to be output to expansion led\n");
    printf("\t\tMust be a six digit hex value with or without leading 0x\n");
    printf("\t\tThe order of the hex digits is: rrggbb\n");
}

#define opinfo 0
#define opset 1
#define opget 2
#define opsetd 3
#define opgetd 4
#define oppwm 5
#define oppwmstop 6
#define opirq 7
#define opirqstop 8
#define opexpled 9
#define opexpledstop 10
int operation;
int pinnum;
int value;
int freq;
int duty;
GPIO_Direction direction;
GPIO_Irq_Type irqType;
char irqcmd[200];
long int debounce;
int ledRedValue;
int ledBlueValue;
int ledGreenValue;
#define ledRedPin 17
#define ledGreenPin 16
#define ledBluePin 15

class GPIO_Irq_Command_Handler_Object : public GPIO_Irq_Handler_Object {
public:
    GPIO_Irq_Command_Handler_Object(char * com) {
        strcpy(this->cmd, com);
    }

    void handleIrq(int pinNum, GPIO_Irq_Type type) {
        if (strstr(cmd, "[debug]") == cmd) {
            char dbgcmd[300];
            sprintf(dbgcmd, "echo 'GPIO Irq Debug: Pin=%d Type=", pinNum);
            if (type == GPIO_IRQ_RISING) {
                strcat(dbgcmd, "Rising' && ");
            } else {
                strcat(dbgcmd, "Falling' && ");
            }
            strcat(dbgcmd, cmd + 7);
            system(dbgcmd);
        } else {
            system(cmd);
        }
    }
    
private:
    char cmd[200];
};

bool processArgs(int argc, char** argv) {
    operation = -1;
    pinnum = -1;
    value = -1;
    if (argc > 1) {
        if (strcmp(argv[1], "info") == 0) {
            operation = opinfo;
        } else if (strcmp(argv[1], "set") == 0) {
            operation = opset;
        } else if (strcmp(argv[1], "read") == 0) {
            operation = opget;
        } else if (strcmp(argv[1], "set-input") == 0) {
            operation = opsetd;
            direction = GPIO_INPUT;
        } else if (strcmp(argv[1], "set-output") == 0) {
            operation = opsetd;
            direction = GPIO_OUTPUT;
        } else if (strcmp(argv[1], "get-direction") == 0) {
            operation = opgetd;
        } else if (strcmp(argv[1], "pwm") == 0) {
            operation = oppwm;
        } else if (strcmp(argv[1], "pwmstop") == 0) {
            operation = oppwmstop;
        } else if (strcmp(argv[1], "irq") == 0) {
            operation = opirq;
        } else if (strcmp(argv[1], "irqstop") == 0) {
            operation = opirqstop;
        } else if (strcmp(argv[1], "expled") == 0) {
            operation = opexpled;
        } else if (strcmp(argv[1], "expledstop") == 0) {
            operation = opexpledstop;
        } else if (strcmp(argv[1], "help") == 0) {
            return false;
        } else {
            printf("**ERROR** Invalid <op> : %s\n", argv[1]);
            return false;
        }
        
        if (operation == opexpled) {
            if (argc > 2) {
                char expled[16];
                
                if ((strncmp("0X", argv[2], 2) == 0) || (strncmp("0x", argv[2], 2) == 0)) {
                    strcpy(expled, argv[2]+2);
                } else {
                    strcpy(expled, argv[2]);
                }
                
                if ((strlen(expled) == 6) && (strspn(expled, "0123456789abcdefABCDEF") == 6)) {
                    char fullled[16];
                    strcpy(fullled, "0x");
                    strcat(fullled, expled);
                    int allled = strtol(fullled, NULL, 0);
                    
                    ledRedValue = (allled >> 16) & 0xff;
                    ledGreenValue = (allled >> 8) & 0xff;
                    ledBlueValue = allled & 0xff;
                    
                    ledRedValue = (ledRedValue * 100) / 255;
                    ledGreenValue = (ledGreenValue * 100) / 255;
                    ledBlueValue = (ledBlueValue * 100) / 255;
                } else {
                    printf("**ERROR** Invalid <ledhex> for expled : %s\n", argv[2]);
                    return false;
                }
            } else {
                printf("**ERROR** No <ledhex> specified for: expled\n");
                return false;
            }
        }
        else if (operation == opexpledstop) {
            return true;
        } else if (argc > 2) {
            if (strcmp(argv[2], "all") == 0) {
                if ((operation != opset) && 
                        (operation != opsetd) &&
                        (operation != opinfo)) {
                    printf("**ERROR** Invalid <pin> for operation: %s\n", argv[2]);
                    return false;
                }
                pinnum = -1;
            } else if (strcmp(argv[2], "0") == 0) {
                pinnum = 0;
                if (!GPIOAccess::isPinUsable(pinnum)) {
                    printf("**ERROR** Invalid <pin> : %s\n", argv[2]);
                    return false;
                }
            } else {
                pinnum = strtol(argv[2], NULL, 10);
                if (pinnum != 0) {
                    if (!GPIOAccess::isPinUsable(pinnum)) {
                        printf("**ERROR** Invalid <pin> : %s\n", argv[2]);
                        return false;
                    }
                } else {
                    printf("**ERROR** Invalid <pin> : %s\n", argv[2]);
                    return false;
                }
            }
            
            if (operation == opset) {
                if (argc > 3) {
                    if (strcmp(argv[3], "0") == 0) {
                        value = 0;
                    } else if (strcmp(argv[3], "1") == 0 ) {
                        value = 1;
                    } else{
                        printf("**ERROR** Invalid <val> for set: %s\n", argv[3]);
                        return false;
                    }
                } else {
                    printf("**ERROR** No <val> specified for set\n");
                    return false;
                }
            } else if (operation == opirq) {
                if (argc > 3) {
                    if (strcmp(argv[3], "falling") == 0 ) {
                        irqType = GPIO_IRQ_FALLING;
                    } else if (strcmp(argv[3], "rising") == 0 ) {
                        irqType = GPIO_IRQ_RISING;
                    } else if (strcmp(argv[3], "both") == 0 ) {
                        irqType = GPIO_IRQ_BOTH;
                    } else{
                        printf("**ERROR** Invalid <irqtype> for irq: %s\n", argv[3]);
                        return false;
                    }
                    if (argc > 4) {
                        strcpy(irqcmd, argv[4]);
                        if (argc > 5) {
                            if (strcmp(argv[5], "0") == 0) {
                                debounce = 0;
                            } else {
                                debounce = strtol(argv[5], NULL, 10);
                                if (debounce == 0) {
                                    printf("**ERROR** Invalid <debounce> for irq : %s\n", argv[5]);
                                    return false;
                                }
                            }                            
                        } else {
                            debounce = 0;
                        }
                    } else {
                        printf("**ERROR** No <irqcmd> specified for irq\n");
                        return false;
                    }
                } else {
                    printf("**ERROR** No <irqtype> specified for irq\n");
                    return false;
                }
            } else if (operation == oppwm) {
                if (argc > 3) {
                    freq = strtol(argv[3], NULL, 10);
                    if (freq > 0) {
                        if (argc > 4) {
                            duty = strtol(argv[4], NULL, 10);
                            if ((duty < 0) || (duty > 100)) {
                                printf("**ERROR** Invalid <duty> for pwm : %s\n", argv[4]);
                                return false;
                            }
                        } else {
                            printf("**ERROR** No <duty> specified for pwm\n");
                            return false;
                        }
                    } else {
                        printf("**ERROR** Invalid <freq> for pwm : %s\n", argv[3]);
                        return false;
                    }
                } else {
                    printf("**ERROR** No <freq> specified for pwm\n");
                    return false;
                }
            }
        } else {
            printf("**ERROR** No <pin> specified\n");
            return false;
        }
    } else {
        printf("**ERROR** No <op> specified\n");
        return false;
    }
    
    return true;
}

#define PID_FILE_PWM	(char *)"/tmp/pin%d_pwm_pid"
#define PID_FILE_IRQ	(char *)"/tmp/pin%d_irq_pid"
#define PID_FILE_EXPLED (char *)"/tmp/expled_pid"

void stopFork(int pinum, char * basePath) {
    char 	pathname[255];
    char	line[255];
    char	cmd[255];

    int 	pid;
    std::ifstream myfile;

    // determine the file name and open the file
    snprintf(pathname, sizeof(pathname), basePath, pinnum);
    myfile.open (pathname);

    // read the file
    if ( myfile.is_open() ) {
        // file exists, check for pid
        myfile.getline(line, 255);
        pid = atoi(line);

        // kill the process
        if (pid > 0)
        {
                sprintf(cmd, "kill %d >& /dev/null", pid);
                system(cmd);
        }

        sprintf(cmd, "rm %s >& /dev/null", pathname);
        system(cmd);

        myfile.close();
    }
}

void stopIRQ(int pinnum) {
    stopFork(pinnum, PID_FILE_IRQ);
}

void stopPWM(int pinnum) {
    stopFork(pinnum, PID_FILE_PWM);
}

bool getForkInf(int pinnum, char * basePath, char * inf) {
    char 	pathname[255];
    char	line[255];
    char	cmd[255];

    int 	pid;
    std::ifstream myfile;

    // determine the file name and open the file
    snprintf(pathname, sizeof(pathname), basePath, pinnum);
    myfile.open (pathname);

    // read the file
    if ( myfile.is_open() ) {
        bool ret = false;
        // file exists, check for pid
        myfile.getline(line, 255);
        pid = atoi(line);        
        if (kill(pid, 0) == 0)
        {
            // Process is running - get the info
            char infostr[128];
            sprintf (infostr, "\tProcess Id:%d\n\t", pid);
            myfile.getline(line, 255);
            strcat(infostr, line);
            strcpy(inf, infostr);
            ret = true;
        }        

        myfile.close();
        
        return ret;
    }
    
    return false;
}

bool getPWMInf(int pinnum, char * inf) {
    return getForkInf(pinnum, PID_FILE_PWM, inf);
}


bool getIRQInf(int pinnum, char * inf) {
    return getForkInf(pinnum, PID_FILE_IRQ, inf);
}

void noteForkChildPID(int pinnum, int pid, char * baseName, char * inf) {
    char 	pathname[255];
    std::ofstream myfile;

    // determine the file name and open the file
    snprintf(pathname, sizeof(pathname), baseName, pinnum);
    myfile.open (pathname);

    // write the pid to the file
    myfile << pid;
    myfile << "\n";
    
    char infa[100];
    sprintf(infa, "%s\n", inf);
    myfile << infa;

    myfile.close();
}

void noteChildPIDPWM(int pinnum, int pid, int freq, int duty) {
    char inf[100];
    sprintf(inf, "Frequency:%d, Duty:%d", freq, duty);
    noteForkChildPID(pinnum, pid, PID_FILE_PWM, inf);
}

void noteChildPIDIRQ(int pinnum, int pid, GPIO_Irq_Type irqType, char * irqcmd, int debounce) {
    char inf[100];
    char irqTypeStr[20];
    if (irqType == GPIO_IRQ_RISING) {
        strcpy(irqTypeStr, "Rising");
    } else if (irqType == GPIO_IRQ_FALLING) {
        strcpy(irqTypeStr, "Falling");
    } else {
        strcpy(irqTypeStr, "Both");
    }
    sprintf(inf, "Type:%s, Cmd:'%s', Debounce:%d", irqTypeStr, irqcmd, debounce);
    noteForkChildPID(pinnum, pid, PID_FILE_IRQ, inf);
}

void stopExpLed() {
    stopFork(0, PID_FILE_EXPLED);
}

void startPWM(int pinnum, int freq, int duty) {
    if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
        stopExpLed();
    }
    stopPWM(pinnum);
    stopIRQ(pinnum);

    // Continuous PWM output requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the pwm
        GPIOPin *pin = new GPIOPin(pinnum);
        pin->setDirection(GPIO_OUTPUT);
        pin->setPWM(freq, duty);
        GPIO_Result r = pin->getLastResult();
        if (r != GPIO_OK) {
            printf("**ERROR starting PWM on pin:%d to freq:%d, duty:%d err=%d\n", pinnum, freq, duty, r);
        }
        // Ensure child stays alive since PWM is running
        while (pin->isPWMRunning()) {
            usleep(1000000L);
        }
    }
    else {
        // parent process
        noteChildPIDPWM(pinnum, pid, freq, duty);
    }
}

void startIRQ(int pinnum, GPIO_Irq_Type irqType, char * irqcmd, int debounce) {
    if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
        stopExpLed();
    }
    stopPWM(pinnum);
    stopIRQ(pinnum);

    // IRQ handling requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the irq handler

        GPIO_Irq_Command_Handler_Object * irqHandlerObj = new GPIO_Irq_Command_Handler_Object(irqcmd);
        GPIOPin *pin = new GPIOPin(pinnum);
        pin->setDirection(GPIO_INPUT);
        pin->setIrq(irqType, irqHandlerObj, debounce);
        GPIO_Result r = pin->getLastResult();
        if (r != GPIO_OK) {
            printf("**ERROR starting IRQ on pin:%d with command:%s err=%d\n", pinnum, irqcmd, r);
        }
        // Ensure child stays alive since IRQ is running
        while (true) {
            usleep(1000000L);
        }
    }
    else {
        // parent process
        noteChildPIDIRQ(pinnum, pid, irqType, irqcmd, debounce);
    }
}

void startExpLed(int ledRedValue, int ledGreenValue, int ledBlueValue) {
    stopIRQ(ledRedPin);
    stopIRQ(ledBluePin);
    stopIRQ(ledGreenPin);
    stopPWM(ledRedPin);
    stopPWM(ledBluePin);
    stopPWM(ledGreenPin);
    stopExpLed();

    // EXPLED handling requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the expled

        RGBLED * expled = new RGBLED();

        expled->setColor(ledRedValue, ledGreenValue, ledBlueValue);
        
        // Ensure child stays alive since EXPLED is running
        while (true) {
            usleep(1000000L);
        }
    }
    else {
        // parent process
        char inf[100];
        sprintf(inf, "Red Value:%d, Green Value:%d, Blue Value:%d", ledRedValue, ledGreenValue, ledBlueValue);
        noteForkChildPID(0, pid, PID_FILE_EXPLED, inf);
    }
}

void setPin(int pinnum, int val) {
    if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
        stopExpLed();
    }
    stopPWM(pinnum);
    stopIRQ(pinnum);
    GPIOPin *pin = new GPIOPin(pinnum);
    pin->setDirection(GPIO_OUTPUT);
    pin->set(val);
    GPIO_Result r = pin->getLastResult();
    delete pin;
    if (r != GPIO_OK) {
        printf("**ERROR setting pin:%d to value:%d, err=%d\n", pinnum, val, r);
    }
}

void setPinDir(int pinnum, GPIO_Direction dir) {
    if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
        stopExpLed();
    }
    stopPWM(pinnum);
    stopIRQ(pinnum);
    GPIOPin *pin = new GPIOPin(pinnum);
    pin->setDirection(dir);
    GPIO_Result r = pin->getLastResult();
    delete pin;
    if (r != GPIO_OK) {
        printf("**ERROR setting pin direction:%d to direction:%d, err=%d\n", pinnum, dir, r);
    }
}

GPIO_Result getPin(int pinnum, int &val) {
    if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
        stopExpLed();
    }
    stopPWM(pinnum);
    GPIOPin *pin = new GPIOPin(pinnum);
    pin->setDirection(GPIO_INPUT);
    val = pin->get();
    GPIO_Result r = pin->getLastResult();
    delete pin;
    if (r != GPIO_OK) {
        printf("**ERROR getting pin:%d value, err=%d\n", pinnum, r);
    }
    return r;
}

GPIO_Result getPinDir(int pinnum, GPIO_Direction &dir) {
    GPIOPin *pin = new GPIOPin(pinnum);
    dir = pin->getDirection();
    GPIO_Result r = pin->getLastResult();
    delete pin;
    if (r != GPIO_OK) {
        printf("**ERROR getting pin:%d direction, err=%d\n", pinnum, r);
    }
    return r;
}


void reportOn(int pinnum) {
    printf("Pin:%d, ", pinnum);
    GPIOPin *pin = new GPIOPin(pinnum);
    GPIO_Direction dir = pin->getDirection();
    printf("Direction:");
    if (dir == GPIO_OUTPUT) {
        printf("OUTPUT");
        char inf[100];
        if ((pinnum == ledRedPin) || (pinnum == ledGreenPin) || (pinnum == ledBluePin)) {
            if (getForkInf(0, PID_FILE_EXPLED, inf)) {
                if (pinnum == ledRedPin) {
                    printf(", EXPLED(red)\n%s", inf);
                } else if (pinnum == ledGreenPin) {
                    printf(", EXPLED(green)\n%s", inf);
                } else if (pinnum == ledBluePin) {
                    printf(", EXPLED(blue)\n%s", inf);
                }
            }
        }
        if (getPWMInf(pinnum, inf)) {
            printf(", PWM\n%s", inf);
        }
    } else if (dir == GPIO_INPUT) {
        int val = pin->get();
        printf("INPUT, Value:%d", val);
        char inf[100];
        if (getIRQInf(pinnum, inf)) {
            printf(" IRQ\n:%s", inf);
        }
    }
    printf("\n");
    delete pin;
}

int main(int argc, char** argv) {
    if (!processArgs(argc, argv)) {
        usage(argv[0]);
        return -1;
    }
    
    if (operation == opinfo) {
        if (pinnum != -1) {
            reportOn(pinnum);
        } else {
            int i;
            for (i = 0; i < numPins; i++) {
                if (GPIOAccess::isPinUsable(i)) {
                    reportOn(i);
                }
            }
        }
        return 0;
    } else if (operation == opset) {
        if (pinnum != -1) {
            setPin(pinnum, value);
        } else {
            int i;
            for (i = 0; i < numPins; i++) {
                if (GPIOAccess::isPinUsable(i)) {
                    setPin(i, value);
                }
            }
        }
        return 0;
    } else if (operation == opget) {
        int val;
        GPIO_Result r = getPin(pinnum, val);
        if (r == GPIO_OK) {
            return val;
        } else {
            return r;
        }
    } else if (operation == opsetd) {
        if (pinnum != -1) {
            setPinDir(pinnum, direction);
        } else {
            int i;
            for (i = 0; i < numPins; i++) {
                if (GPIOAccess::isPinUsable(i)) {
                    setPinDir(i, direction);
                }
            }
        }
        return 0;
    } else if (operation == opgetd) {
        GPIO_Direction dir;
        GPIO_Result r = getPinDir(pinnum, dir);
        if (r == GPIO_OK) {
            return dir;
        } else {
            return r;
        }
    } else if (operation == oppwm) {
        startPWM(pinnum, freq, duty);
        return 0;
    } else if (operation == oppwmstop) {
        stopPWM(pinnum);
        return 0;
    } else if (operation == opirq) {
        startIRQ(pinnum, irqType, irqcmd, debounce);
        return 0;
    } else if (operation == opirqstop) {
        stopIRQ(pinnum);
        return 0;
    } else if (operation == opexpled) {
        startExpLed(ledRedValue, ledGreenValue, ledBlueValue);
        return 0;
    } else if (operation == opexpledstop) {
        stopExpLed();
        return 0;
    }

    return -1;
}

