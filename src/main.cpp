#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485
#define packageTimer 8020 // Define o timer para 8,020 milisegundos o dobro do tempo de tramissão 

/**
 * Funcão que envia o pacote ao cfp
**/
void sendMsg(byte msg[]) {
  Serial.println("Start send msg.");

  for (byte i = 0; i < sizeof(msg); i++)
  {
    Serial.write(msg[i]); // Envio de cada bit da mensagem
    Serial.println(msg[i], HEX);
  }
};
/** ------------------------------ **/

/**
 * Função que recebe o endereço do cfp e um id de msg e manda a msg ao cpf
**/
byte* makeMsg(int cfpAddress, int msgId) {
  byte packageMsg[5];

  packageMsg[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  packageMsg[1] = 0x03; // byte de endereço do cfp
  packageMsg[2] = 0x01; // tamanho, variavel padrao para as msgs que vamos mandar ao cfp

  /**  **/
  switch (msgId)
  {
  case 1: // id: 1 - Ler Modo de funcionamento
     packageMsg[3] = 0x01;
    break;
  default:
    break;
  }

  return packageMsg;
}
/** ------------------------------ **/

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
/** ------------------------------ **/

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
/** ------------------------------ **/

/**
 * Arduino Loop 
**/
void loop() {
  // digitalWrite(rs485Comunication, HIGH);
  // sendMsg(); // Envia msg
  // digitalWrite(rs485Comunication, LOW); // Espera receber mensagem
}
/** ------------------------------ **/

/**
 * interrupção serial nas portas TX1 = 18, RX1 = 19
**/
void serialEvent1() {
  byte incomingByte;
  incomingByte = Serial1.read();
  
  Serial.println("New byte:");
  Serial.println(incomingByte, HEX);
  
  Timer1.initialize(packageTimer); // Inicia o timer que aguardar o pacote inteiro
}
/** ------------------------------ **/