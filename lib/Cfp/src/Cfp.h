
#ifndef Cfp_h
#define Cfp_h

#include <Arduino.h>
#include <HardwareSerial.h>

/*Classe para funções de controle para o CFP*/
class Cfp
{

private:
    /* Variables*/ 
    int byteExpiresTimer;
    int rs485ComunicationPin;
    int address; 

    /*Methods*/
    
public:
    /* Variables*/ 
    unsigned long actualIn;
    unsigned long actualOut;
    unsigned long totalIn;
    unsigned long totalOut;
    /*Methods*/
    Cfp(int rs485Comunication);
    void sendPkg(byte address, byte operationCode);
    bool pkgValidator(byte pkg[], byte sizeOfPkg, byte operationCode);
    void getValuesFromCpfResponse(byte pkg[], byte sizeOfPkg);
};

#endif