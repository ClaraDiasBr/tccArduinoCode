#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define packageTimer 8020 // Define o timer para 8,020 milisegundos o dobro do tempo de tramissão 
#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

bool test = true;
byte msgToSend[5];

/**
 * Funcão que envia o pacote ao cfp
**/
void sendMsg() {
  Serial.println("Start send msg.");
  digitalWrite(rs485Comunication, HIGH);
  
  for (byte i = 0; i < sizeof(msgToSend); i++)
  {
    Serial1.write(msgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(msgToSend[i], HEX);
  }

  Serial1.flush();
};
/** ------------------------------ **/

/**
 * Função que recebe o endereço do cfp e um id de msg e manda a msg ao cpf
**/
void makeMsg(int cfpAddress, int msgId) {
  msgToSend[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  msgToSend[1] = 0x03; // byte de endereço do cfp
  msgToSend[2] = 0x01; // tamanho, variavel padrao para as msgs que vamos mandar ao cfp
  msgToSend[4] = 0x0D;

  /** Switch que define codigo de operacao cfp **/
  switch (msgId)
  {
  case 1: // id: 1 - Ler Modo de funcionamento
     msgToSend[3] = 0x01;
    break;
  default:
    break;
  }

  for (byte i = 0; i < sizeof(msgToSend); i++)
  {
    Serial.println(msgToSend[i], HEX);
  }
  
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

/**
 * Arduino setup 
 * **/

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); 

  pinMode(rs485Comunication, OUTPUT); // Define a porta que manipula a recebimento e envio de msgs RS485
 
  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote
}
/** ------------------------------ **/

/**
 * Arduino Loop 
**/
void loop() {
  if (test)
  {
    digitalWrite(rs485Comunication, HIGH);                                                         
    makeMsg(1, 1);
    sendMsg();
  }
  
  test = false;
  digitalWrite(rs485Comunication, LOW); // Espera receber mensagem
}
/** ------------------------------ **/
