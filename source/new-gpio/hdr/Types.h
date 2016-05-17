#ifndef Types_H
#define Types_H

enum NodeType {
    nodeTypeOption,
    nodeTypeOperation
};

enum OperationType {
    opInvalid,
    
    opInfo,
    opSet,
    opRead,
    opSetInput,
    opSetOutput,
    opGetDirection,
    opPwm,
    opPwmStop,
    opIrq,
    opIrq2,
    opIrqStop,
    opExpled,
    opExpledStop,
    opTone,
    opToneStop,
    opShiftOut,
    opShiftIn,
    opPulseOut,
    opPulseIn,
    opFreq,
    opDelay,
    opExec,
    
    opAssign,
    opFileIn,
    opFileOut,
    opFileDelete,
    
    opWhile,
    opIf, 
    opBreak,
    opExit,

    opLast,
    
    opEndWhile,
    opElse,
    opEndIf
};

#endif
