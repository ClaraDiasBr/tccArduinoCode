
#ifndef Cfp_h
#define Cfp_h

#include <Arduino.h>
#include <HardwareSerial.h>

class Cfp
{

private:
    /* Variables*/ 
    int byteExpiresTimer;
    int rs485ComunicationPin;
    int address; 

    /*Methods*/
    
public:
    Cfp(int rs485Comunication);

    void sendPkg(byte address, byte operationCode);
    bool pkgValidator(byte pkg[], byte sizeOf);
};

#endif