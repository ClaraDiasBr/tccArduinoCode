#include"Cfp.h"
#include <Arduino.h>
#include <HardwareSerial.h>


Cfp::Cfp(int rs485Comunication) {
  pinMode(rs485Comunication, OUTPUT);
  rs485ComunicationPin = rs485Comunication;
}

/*
Função que valida  o pacote recebido do cfp
*/
bool Cfp::pkgValidator(byte pkg[], byte sizeOfPkg, byte operationCode) {
  bool valid = false;
 
  byte auxReceivedBytesSum = 0;
  byte lastValidReceivedBit = 0;
 
  for (byte i = 0; i < sizeOfPkg; i++)
  {
    auxReceivedBytesSum = auxReceivedBytesSum + pkg[i];
    if (pkg[i] != 0x00) lastValidReceivedBit = pkg[i];
  }
  
  auxReceivedBytesSum = auxReceivedBytesSum -lastValidReceivedBit; // Subtrai o ultimo valor do pacote da soma de todos os bits, já que o ultimo valor é checkSum.
 
  if ((auxReceivedBytesSum == lastValidReceivedBit) && (auxReceivedBytesSum != 0x00)) valid = true; 
  
  switch (operationCode)
  {
  case 0x02: // Checa se a contagem tem falha ou não
    if ((pkg[4] == 0x01 ) || (pkg[8] == 0x01) || (pkg[12] == 0x01 ) || (pkg[16] == 0x01)) valid = false;
    break;
  case 0x03: // Checa se o comando de zerar contagem falhou ou não
  case 0x04: // Checa se o comando de zerar contagem falhou ou não
    if (pkg[3] != 0x55) valid = false;
    break;  
  default:
    break;
  }
  
  return valid;
}

/*
Função que recebe o endereço cfp, o código de operação, e então manda o pacote
*/
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
  Serial1.flush();
  digitalWrite(rs485ComunicationPin, LOW);
  Serial.println("---------------");
}

/*
Função auxiliar para contar os valores do pacote recebido
*/
void Cfp::getValuesFromCpfResponse(byte pkg[], byte sizeOfPkg) {
  actualIn = 0;
  actualOut = 0;
  totalIn = 0;
  totalOut = 0;

  for (byte i = 0; i < sizeOfPkg; i++)
  {
    if ((i >= 4) && (i <=7)) actualIn = actualIn + pkg[i];
    
    if ((i >= 8) && (i <=11)) actualOut = actualOut + pkg[i];
    
    if ((i>=12) && (i<=15)) totalIn = totalIn + pkg[i];

    if  ((i>=16) && (i <=19)) totalOut = totalOut + pkg[i];
  }
}
