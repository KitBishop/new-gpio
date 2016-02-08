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

    printf("\t%s <ledhex>\n",nm);
    printf("\t\tStarts output to expansion led\n");
    printf("\t%s rgb <r> <g> <b>\n",nm);
    printf("\t\tStarts output to expansion led using decimal rgb values\n");
    printf("\t%s stop\n",nm);
    printf("\t\tTerminates output to expansion led\n");
    printf("\t%s help\n",nm);
    printf("\t\tDisplays this usage information\n");
    printf("Where:\n");
    printf("\t<ledhex> specifies the hex value to be output to expansion led\n");
    printf("\t\tMust be a six digit hex value with or without leading 0x\n");
    printf("\t\tThe order of the hex digits is: rrggbb\n");
    printf("\t<r> <g> <b> specify the decimal values for output to expansion led\n");
    printf("\t\tEach value is in the range 0..100\n");
    printf("\t\t0 = off, 100 = fully on\n");
}

#define opexpled 1
#define opexpledstop 2
int operation;
int ledRedValue;
int ledBlueValue;
int ledGreenValue;

bool processArgs(int argc, char** argv) {
    operation = -1;
    if (argc > 1) {
        if (strcmp(argv[1], "stop") == 0) {
            operation = opexpledstop;
        } else if (strcmp(argv[1], "help") == 0) {
            return false;
        } else {
            operation = opexpled;
        }
        
        if (operation == opexpled) {
            if (argc > 1) {
                
                if (strcmp("rgb", argv[1]) == 0) {
                    if (argc >= 5) {
                        if (strcmp("0", argv[2]) == 0) {
                            ledRedValue = 0;
                        } else {
                            ledRedValue = strtol(argv[2], NULL, 10);
                            if ((ledRedValue <= 0) || (ledRedValue > 100)) {
                                printf("**ERROR** Invalid <r> for expled rgb: %s\n", argv[2]);
                                return false;
                            }
                        }
                        if (strcmp("0", argv[3]) == 0) {
                            ledGreenValue = 0;
                        } else {
                            ledGreenValue = strtol(argv[3], NULL, 10);
                            if ((ledGreenValue <= 0) || (ledGreenValue > 100)) {
                                printf("**ERROR** Invalid <g> for expled rgb: %s\n", argv[3]);
                                return false;
                            }
                        }
                        if (strcmp("0", argv[4]) == 0) {
                            ledBlueValue = 0;
                        } else {
                            ledBlueValue = strtol(argv[4], NULL, 10);
                            if ((ledBlueValue <= 0) || (ledBlueValue > 100)) {
                                printf("**ERROR** Invalid <b> for expled rgb: %s\n", argv[4]);
                                return false;
                            }
                        }
                    } else {
                        printf("**ERROR** Insufficient data for expled rgb\n");
                        return false;
                    }
                } else {
                    char expled[16];

                    if ((strncmp("0X", argv[1], 2) == 0) || (strncmp("0x", argv[1], 2) == 0)) {
                        strcpy(expled, argv[1]+2);
                    } else {
                        strcpy(expled, argv[1]);
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
                }
            } else {
                printf("**ERROR** No <ledhex> specified for: expled\n");
                return false;
            }
        }
        else if (operation == opexpledstop) {
            return true;
        }
    } else {
        printf("**ERROR** No parameter supplied\n");
        return false;
    }
    
    return true;
}

#define PID_FILE_EXPLED (char *)"/tmp/expled_pid"

void stopFork(int pinnum, char * basePath) {
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

void stopExpLed() {
    stopFork(0, PID_FILE_EXPLED);
}

void startExpLed(int ledRedValue, int ledGreenValue, int ledBlueValue) {
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

int main(int argc, char** argv) {
    if (!processArgs(argc, argv)) {
        usage(argv[0]);
        return -1;
    }
    
    if (operation == opexpled) {
        startExpLed(ledRedValue, ledGreenValue, ledBlueValue);
        return 0;
    } else if (operation == opexpledstop) {
        stopExpLed();
        return 0;
    }

    return -1;
}

