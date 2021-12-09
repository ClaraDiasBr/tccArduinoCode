#include"Cfp.h"
#include <Arduino.h>
#include <HardwareSerial.h>

byte receivedPkg[25];
byte receivedByteIndex = 0x00;

Cfp::Cfp(int rs485Comunication) {
  pinMode(rs485Comunication, OUTPUT);

  rs485ComunicationPin = rs485Comunication;
}

bool Cfp::pkgValidator(byte pkg[], byte sizeOf) {
  bool valid = false;
 
  byte auxReceivedBytesSum = 0;
  byte lastValidReceivedBit = 0;
 
  for (byte i = 0; i < sizeOf; i++)
  {
    Serial.println(pkg[i], HEX);
    auxReceivedBytesSum = auxReceivedBytesSum + pkg[i];

    if (pkg[i] != 0x00)
    {
     lastValidReceivedBit = pkg[i];
    }
  }
  
  auxReceivedBytesSum = auxReceivedBytesSum -lastValidReceivedBit; // Subtrai o ultimo valor do pacote da soma de todos os bits, já que o ultimo valor é checkSum.

  if (auxReceivedBytesSum ==lastValidReceivedBit) valid = true; 
  return valid;
}

void Cfp::sendPkg(byte address, byte operationCode){
  Serial.println("Making pkg");

  byte pkgToSend[5] = { 0x80, address, 0x01, operationCode, 0x00 };

  byte checkSum;

  for (byte i = 0; i < sizeof(pkgToSend) - 1; i++)
  { 
    checkSum = checkSum + pkgToSend[i];  
  }
  
  pkgToSend[4] = checkSum; //checksum
  Serial.println("Send pkg.");
  
  digitalWrite(rs485ComunicationPin, HIGH);
  
  for (byte i = 0; i < sizeof(pkgToSend); i++)
  {
    Serial1.write(pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(pkgToSend[i], HEX);
  }

  Serial.println("---------------");
  Serial1.flush();
  digitalWrite(rs485ComunicationPin, LOW);
  receivedByteIndex = 0x00;
}
