#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "FileInOperation.h"
#include "Utilities.h"

using namespace std;

FileInOperation::FileInOperation()
    : Operation(opFileIn) {
    assignName = "";
    fileName = "";
}

bool FileInOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No file name specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    fileName = **paramIter;

    (*paramIter)++;
    
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No variable specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    
    bool isOk = true;

    char c = (*paramIter)->at(0);
    
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
        int i;
        assignName = "";
        for (i = 0; (i < (*paramIter)->length()) && isOk; i++) {
            c = (*paramIter)->at(i);
            if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9'))) {
                assignName = assignName + c;
            } else {
                isOk = false;
            }
        }
    } else {
        isOk = false;
    }
    
    if (!isOk) {
        appInfo->prtError(opType, "Invalid variable name for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    
    (*paramIter)++;

    return true;
}

string FileInOperation::toString() {
    return Operation::toString()
            + " FileName:" + fileName
            + " Variable:" + assignName;
}

bool FileInOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    string s = "";
    s = s + "Reading from file:" + fileName + " to variable:"+ assignName;
    appInfo->prtReport(s);
    
    long int fileVal;
    if (readFromFile(fileName, fileVal)) {
        appInfo->setVar(assignName, fileVal);
        appInfo->prtOutput(to_string(fileVal));
        appInfo->result = fileVal;
    } else {
        appInfo->prtError(opType, "Error reading value from file:" + fileName);
        return false;
    }
    
    return true;
}

string FileInOperation::help() {
    string hStr;
    hStr = "filein <file-name> <variable-name>";
    hStr = hStr + "\n\tReads integer value from first line of file to variable";
    hStr = hStr + "\n\tAlong with 'fileout', and 'filedelete' operations and";
    hStr = hStr + "\n\t$[<filename>] expression item, provides a rudimentary";
    hStr = hStr + "\n\tmechanism for communicating with other programs.";
    hStr = hStr + "\n\t<file-name> is the name of the file to read from.";
    hStr = hStr + "\n\tMust be enclosed in \" characters if it contains spaces";
    hStr = hStr + "\n\tor other special character.";
    hStr = hStr + "\n\t<variable-name> is the user variable to which to assign";
    hStr = hStr + "\n\tthe value read.";
    hStr = hStr + "\n\tAny sequence of letters and digits starting with a letter.";
    hStr = hStr + "\n\tNames are case sensitive";
    
    return hStr;
}

bool FileInOperation::readFromFile(string fileName, long int &fileVal) {
    ifstream myfile;
    char line[255];

    myfile.open(fileName);

    // read the file
    if (myfile.is_open()) {
        myfile.getline(line, 255);
        fileVal = atol(line);

        myfile.close();
        return true;
    }
    
    return false;
}