#include <Arduino.h>
#include "TimerOne.h" // Usada biblio

#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485
#define packageTimer 8020 // Define o timer para 8,020 milisegundos o dobro do tempo de tramissão 


byte incomingByte;

void sendMsg(int cfpAddress, int msgId) {
  
  switch (msgId)
  {
  case 1: // Ler Modo de funcionamento
    byte msgToSend[] = {0x80, 0x05, 0x01, 0x02, 0x88};
    break;
  default:
    break;
  }
  
  Serial.println("Start send msg.");
  byte msgRedCont[] = {0x80, 0x05, 0x01, 0x02, 0x88};

  for (byte i = 0; i < sizeof(msgRedCont); i++)
  {
    Serial.write(msgRedCont[i]); // Envio de cada bit da mensagem
    Serial.println(msgRedCont[i], HEX);
  }
}

/**
 * Função timerCallback
 * Executada a cada fim do tempo do timer
 **/
void timerCallback()
{
  Timer1.stop();
   
  Serial.println("End package:");
  Serial.println("-------------------");
}

/**
 * Arduino setup 
 * **/

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); 

  pinMode(rs485Comunication, OUTPUT); // Define a porta que manipula a recebimento e envio de msgs RS485
  digitalWrite(rs485Comunication, LOW); // Espera receber mensagem

  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote
}

/**
 * Arduino Loop 
**/
void loop() {
  // digitalWrite(rs485Comunication, HIGH);
  
  // sendMsg(); // Envia msg
  // digitalWrite(rs485Comunication, LOW); // Espera receber mensagem

}

/**
 * interrupção serial nas portas TX1 18, RX1 19
**/
void serialEvent1() {
  incomingByte = Serial1.read();
  Serial.println("New byte:");
  Serial.println(incomingByte, HEX);
  
  Timer1.initialize(packageTimer); // Inicia o timer que aguardar o pacote inteiro
}