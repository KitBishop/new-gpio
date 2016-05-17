#include "Option.h"
#include "Utilities.h"
#include "InfoOperation.h"
#include "SetOperation.h"
#include "ReadOperation.h"
#include "SetInputOperation.h"
#include "SetOutputOperation.h"
#include "GetDirectionOperation.h"
#include "PwmOperation.h"
#include "PwmStopOperation.h"
#include "IrqOperation.h"
#include "Irq2Operation.h"
#include "IrqStopOperation.h"
#include "ExpledOperation.h"
#include "ExpledStopOperation.h"
#include "ToneOperation.h"
#include "ToneStopOperation.h"
#include "ShiftOutOperation.h"
#include "ShiftInOperation.h"
#include "PulseOutOperation.h"
#include "PulseInOperation.h"
#include "FreqOperation.h"
#include "DelayOperation.h"
#include "AssignOperation.h"
#include "FileInOperation.h"
#include "FileOutOperation.h"
#include "FileDeleteOperation.h"
#include "ExecOperation.h"
#include "WhileOperation.h"
#include "IfOperation.h"
#include "BreakOperation.h"
#include "ExitOperation.h"

#include "GPIOAccess.h"

Option::Option(char optC, OptOper op, string * param)
: Node(nodeTypeOption) {
    optChar = optC;
    optParam = param;
    oper = op;
}

string Option::toString() {
    string s = "Option: OptionChar:";
    s = s + optChar + " OptionOperation:";
    if (oper == on) {
        s = s + "On";
    } else if (oper == off) {
        s = s + "Off";
    } else {
        s = s + "Param OptionParameter:";
        if (optParam == NULL) {
            s = s + "NULL";
        } else {
            s = s + *optParam;
        }
    }
    return s;
}

Option * Option::create(AppInfo * appInfo, string optStr) { //char optC, string * param) {
    Option * opt = NULL;
    char optC;
    OptOper oper;
    string * optParam = NULL;
    bool optOk = true;

    if (optStr.length() >= 2) {
        optC = optStr.at(1);
        switch (optC) {
            case 'v':
            case 'q':
            case 'o':
            case 'r':
            case 'e':
            case 'i':
            case 's':
                if (optStr.length() == 2) {
                    oper = on;
                } else if (optStr.length() == 3) {
                    if (optStr.at(2) == '+') {
                        oper = on;
                    } else if (optStr.at(2) == '-') {
                        oper = off;
                    } else {
                        optOk = false;
                    }
                } else {
                    optOk = false;
                }
                break;

            case '?':
                appInfo->haveOp = true;
                if (optStr.length() == 2) {
                    oper = on;
                } else {
                    optOk = false;
                }
                break;

            case 'h':
                appInfo->haveOp = true;
                if (optStr.length() == 2) {
                    oper = on;
                } else if (optStr.length() > 3) {
                    if (optStr.at(2) == ':') {
                        oper = param;
                        optParam = new string(optStr.substr(3, string::npos));

                        if (optParam->at(0) == '-') {
                            if (optParam->length() != 2) {
                                optOk = false;
                            } else {
                                char hC = optParam->at(1);
                                switch (hC) {
                                    case 'v':
                                    case 'q':
                                    case 'o':
                                    case 'r':
                                    case 'e':
                                    case 'i':
                                    case 's':
                                    case 'd':
                                    case 'h':
                                    case '?':
                                        break;

                                    default:
                                        optOk = false;
                                }
                            }
                        }
                    } else {
                        optOk = false;
                    }
                } else {
                    optOk = false;
                }
                break;

            default:
                optOk = false;
                break;
        }
    } else {
        optOk = false;
    }

    if (optOk) {
        opt = new Option(optC, oper, optParam);
    } else {
        prterr("Invalid option:" + optStr);
    }

    return opt;
}

bool Option::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    bool setting;
    switch (optChar) {
        case 'v':
            setting = oper == on;
            appInfo->output = setting;
            appInfo->report = setting;
            appInfo->errors = setting;
            break;

        case 'q':
            setting = oper == off;
            appInfo->output = setting;
            appInfo->report = setting;
            appInfo->errors = setting;
            break;

        case 'o':
            setting = oper == on;
            appInfo->output = setting;
            break;

        case 'r':
            setting = oper == on;
            appInfo->report = setting;
            break;

        case 'i':
            setting = oper == on;
            appInfo->ignore = setting;
            break;

        case 'e':
            setting = oper == on;
            appInfo->errors = setting;
            break;

        case 's':
            setting = oper == on;
            appInfo->setdir = setting;
            break;

        case '?':
        {
            string bStr = basicHelp(appInfo);
            bool apbRep = appInfo->report;
            appInfo->report = true;
            appInfo->prtReport(bStr);
            appInfo->report = apbRep;
        }
            break;

        case 'h':
        {
            bool apRep = appInfo->report;
            appInfo->report = true;

            string hStr;
            if ((optParam == NULL) || optParam->empty() || (optParam->compare("all") == 0)) {
                hStr = progInfoHelp();
                hStr = hStr + "Usage:\n" + appInfo->appName + " [any length sequence of spaces separated <input-element>s]";
                hStr = hStr + "\nAn <input-element> is one of: <option> <file-input> <operation>\n\n";
                string optHstr;
                optHstr = optionHelp('v') + '\n'
                        + optionHelp('q') + '\n'
                        + optionHelp('o') + '\n'
                        + optionHelp('r') + '\n'
                        + optionHelp('e') + '\n'
                        + optionHelp('i') + '\n'
                        + optionHelp('s') + '\n'
                        + optionHelp('d') + '\n'
                        + optionHelp('h') + '\n'
                        + optionHelp('?');
                replaceStringInPlace(optHstr, "\n", "\n\t");
                optHstr = "An <option> is one of:\n\t" + optHstr + "\n";

                string atHstr;
                atHstr = atHelp();
                replaceStringInPlace(atHstr, "\n", "\n\t");
                atHstr = "A <file-input> is:\n\t" + atHstr + "\n";

                string opHstr = "";
                int otI;
                for (otI = 1; otI != opLast; otI++) {
                    opHstr = opHstr + operationHelp((OperationType) otI) + "\n";
                }
                replaceStringInPlace(opHstr, "\n", "\n\t");
                opHstr = "An <operation> is one of:\n\t" + opHstr;

                hStr = hStr + optHstr + "\n" + atHstr + "\n" + opHstr;
                hStr = hStr + "\n"+ sourcesHelp();
            } else if (optParam->at(0) == '-') {
                if (optParam->length() != 2) {
                    appInfo->prtError(opInvalid, "Invalid -h parameter:" + *optParam);
                    return false;
                }
                char hC = optParam->at(1);
                switch (hC) {
                    case 'v':
                    case 'q':
                    case 'o':
                    case 'r':
                    case 'i':
                    case 'e':
                    case 's':
                    case 'h':
                    case 'd':
                    case '?':
                        hStr = optionHelp(hC);
                        break;

                    default:
                        appInfo->prtError(opInvalid, "Invalid -h parameter:" + *optParam);
                        return false;
                }
            } else if (optParam->at(0) == '@') {
                if (optParam->length() != 1) {
                    appInfo->prtError(opInvalid, "Invalid -h parameter:" + *optParam);
                    return false;
                }
                hStr = atHelp();
            } else {
                OperationType opType = Operation::mapOpType(*optParam);
                switch (opType) {
                    case opInfo:
                    case opSet:
                    case opRead:
                    case opSetInput:
                    case opSetOutput:
                    case opGetDirection:
                    case opPwm:
                    case opPwmStop:
                    case opIrq:
                    case opIrq2:
                    case opIrqStop:
                    case opExpled:
                    case opExpledStop:
                    case opTone:
                    case opToneStop:
                    case opShiftOut:
                    case opShiftIn:
                    case opPulseOut:
                    case opPulseIn:
                    case opFreq:
                    case opDelay:
                    case opAssign:
                    case opFileIn:
                    case opFileOut:
                    case opFileDelete:
                    case opExec:
                    case opWhile:
                    case opEndWhile:
                    case opIf:
                    case opElse:
                    case opEndIf:
                    case opBreak:
                    case opExit:
                        hStr = operationHelp(opType);
                        break;

                    default:
                        appInfo->prtError(opType, "Invalid -h parameter:" + *optParam);
                        return false;
                }
            }
            appInfo->prtReport(hStr);
            appInfo->report = apRep;
        }
            break;
    }

    return true;
}

string Option::operationHelp(OperationType opType) {
    string hStr;
    switch (opType) {
        case opInfo:
            hStr = InfoOperation::help();
            break;

        case opSet:
            hStr = SetOperation::help();
            break;

        case opRead:
            hStr = ReadOperation::help();
            break;

        case opSetInput:
            hStr = SetInputOperation::help();
            break;

        case opSetOutput:
            hStr = SetOutputOperation::help();
            break;

        case opGetDirection:
            hStr = GetDirectionOperation::help();
            break;

        case opPwm:
            hStr = PwmOperation::help();
            break;

        case opPwmStop:
            hStr = PwmStopOperation::help();
            break;

        case opIrq:
            hStr = IrqOperation::help();
            break;

        case opIrq2:
            hStr = Irq2Operation::help();
            break;

        case opIrqStop:
            hStr = IrqStopOperation::help();
            break;

        case opExpled:
            hStr = ExpledOperation::help();
            break;

        case opExpledStop:
            hStr = ExpledStopOperation::help();
            break;

        case opTone:
            hStr = ToneOperation::help();
            break;

        case opToneStop:
            hStr = ToneStopOperation::help();
            break;

        case opShiftOut:
            hStr = ShiftOutOperation::help();
            break;

        case opShiftIn:
            hStr = ShiftInOperation::help();
            break;

        case opPulseOut:
            hStr = PulseOutOperation::help();
            break;

        case opPulseIn:
            hStr = PulseInOperation::help();
            break;

        case opFreq:
            hStr = FreqOperation::help();
            break;

        case opDelay:
            hStr = DelayOperation::help();
            break;

        case opAssign:
            hStr = AssignOperation::help();
            break;

        case opFileIn:
            hStr = FileInOperation::help();
            break;

        case opFileOut:
            hStr = FileOutOperation::help();
            break;

        case opFileDelete:
            hStr = FileDeleteOperation::help();
            break;

        case opExec:
            hStr = ExecOperation::help();
            break;

        case opWhile:
        case opEndWhile:
            hStr = WhileOperation::help();
            break;

        case opIf:
        case opElse:
        case opEndIf:
            hStr = IfOperation::help();
            break;

        case opBreak:
            hStr = BreakOperation::help();
            break;

        case opExit:
            hStr = ExitOperation::help();
            break;
    }

    return hStr;

}

string Option::optionHelp(char optC) {
    string hStr;
    switch (optC) {
        case 'v':
            hStr = "-v and -v+ - sets verbose mode; equivalent to -o -r -e\n";
            hStr = hStr + "-v- - resets verbose mode; equivalent to -q\n";
            hStr = hStr + "\tDefaults are: -o+ -r- -e+";
            break;

        case 'q':
            hStr = "-q and -q+ - sets quiet mode; equivalent to -o- -r- -e-\n";
            hStr = hStr + "-q- - resets quiet mode; equivalent to -v";
            break;

        case 'o':
            hStr = "-o and -o+ - enables output to stdout of any results of operation\n";
            hStr = hStr + "-o- - disables output to stdout of any results of operation\n";
            hStr = hStr + "\tDefault is: -o+";
            break;

        case 'r':
            hStr = "-r and -r+ - enables output to stderr of report on actions taken\n";
            hStr = hStr + "-r- - disables output to stderr of report on actions taken\n";
            hStr = hStr + "\tDefault is: -r-";
            break;

        case 'i':
            hStr = "-i and -i+ - enables ignoring of any errors during processing\n";
            hStr = hStr + "-i- - disables ignoring of any errors during processing\n";
            hStr = hStr + "\tDefault is: -i-";
            break;

        case 'e':
            hStr = "-e and -e+ - enables output to stderr of any errors during processing\n";
            hStr = hStr + "-e- - disables output to stderr of any errors during processing\n";
            hStr = hStr + "\tDefault is: -e+";
            break;

        case 's':
            hStr = "-s and -s+ - causes the program to ensure that the pin direction";
            hStr = hStr + "\n\tis set appropriately for each operation\n";
            hStr = hStr + "-s- - does not set the direction for each operation\n";
            hStr = hStr + "\tDefault is: -s-";
            break;

        case 'h':
            hStr = "-h or -h:all - displays all available help\n";
            hStr = hStr + "-h:-<option-letter> - displays help for the given option letter\n";
            hStr = hStr + "-h:<operation> - displays help for the given operation\n";
            hStr = hStr + "-h:@ - displays help for input from file";
            break;

        case 'd':
            hStr = "-d - enables debugging output on entered data\n";
            hStr = hStr + "\tWhen used anywhere in the input causes debugging output\n";
            hStr = hStr + "\tto be displayed on scanned input and processed operation data\n";
            hStr = hStr + "\tBy default, debugging output is disabled";
            break;

        case '?':
            hStr = "-? - displays basic usage help";
            break;
    }

    return hStr;
}

string Option::basicHelp(AppInfo * appInfo) {
    string hStr = progInfoHelp();
    hStr = hStr + "Basic Usage:\n" + appInfo->appName + " [any length sequence of spaces separated <input-element>s]\n";
    hStr = hStr + "An <input-element> is one of: <option> <file-input> <operation>\n";
    hStr = hStr + "An <option> is one of:\n";
    hStr = hStr + "\t-v - verbose output\n";
    hStr = hStr + "\t-q - quiet output\n";
    hStr = hStr + "\t-o - result output\n";
    hStr = hStr + "\t-r - report output\n";
    hStr = hStr + "\t-i - ignore errors\n";
    hStr = hStr + "\t-e - error output\n";
    hStr = hStr + "\t-s - automatic setting of pin direction\n";
    hStr = hStr + "\t-h - various levels of help output\n";
    hStr = hStr + "\t-? - this basic usage output\n";
    hStr = hStr + "A <file-input> is a:\n";
    hStr = hStr + "\t@<file-name> - input commands from file\n";
    hStr = hStr + "An <operation> is of the form:\n";
    hStr = hStr + "\t<operation-name> <operation parameters>\n";
    hStr = hStr + "\t\tAn <operation-name> is one of:\n";
    int otI;
    for (otI = 1; otI != opLast; otI++) {
        hStr = hStr + "\t\t\t" + Operation::mapFromOpType((OperationType) otI) + "\n";
        if ((OperationType) otI == opWhile) {
            hStr = hStr + "\t\t\t" + Operation::mapFromOpType(opEndWhile) + "\n";
        }
        if ((OperationType) otI == opIf) {
            hStr = hStr + "\t\t\t" + Operation::mapFromOpType(opElse) + "\n";
            hStr = hStr + "\t\t\t" + Operation::mapFromOpType(opEndIf) + "\n";
        }
    }
    hStr = hStr + "\t\tThe <operation-parameters> depend on the specific operation\n\n";
    hStr = hStr + "More information can be displayed by using one of:\n";
    hStr = hStr + "\t-h or -h:all - for all help\n";
    hStr = hStr + "\t-h:-<option-letter> - for help on the option\n";
    hStr = hStr + "\t-h:@ - for help on file input\n";
    hStr = hStr + "\t-h:<operation-name> - for help on the operation and its parameters\n\n";
    hStr = hStr + sourcesHelp();

    return hStr;
}

string Option::atHelp() {
    string hStr;
    hStr = "@<file-name> - causes input to be taken from the given file";
    hStr = hStr + "\n\tInput is free form sequence of space separated standard";
    hStr = hStr + "\n\t\tinput elements: <option> <file-input> <operation>";
    hStr = hStr + "\n\tAny line with first non-blank character of # is ignored";
    return hStr;
}

string Option::progInfoHelp() {
    string hStr = "A C++ program to control and interact with GPIO pins via scripted operations";
    hStr = hStr + "\n\tProgram version:" + applicationVersion;
    hStr = hStr + "\n\tGPIO Library version:" + GPIOAccess::getLibVersion() + "\n";
    return hStr;
}

string Option::sourcesHelp() {
    string hStr = "Sources available at: https://github.com/KitBishop/new-gpio\n";
    return hStr;
}
