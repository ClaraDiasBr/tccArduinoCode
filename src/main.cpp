#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define byteExpiresTimer 8020 // Define o timer para 8,020 milisegundos o dobro do tempo de tramissão rs485
#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

bool test = true;
byte pkgToSend[5];

/** ------------ Envio de pacotes -------------------- **/

/**
 * Função que recebe o endereço do cfp e um id de codigo de operaco cfp 
**/
void makePkg(int cfpAddress, int cfpOperationCodeId) {
  pkgToSend[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  pkgToSend[1] = cfpAddress; // byte de endereço do cfp;
  pkgToSend[2] = 0x01; // tamanho, valor padrao para buffer que vamos mandar ao cfp;

  /** Switch que define codigo de operacao cfp **/
  switch (cfpOperationCodeId)
  {
    case 1: // id: 1 - Ler Modo de funcionamento
      pkgToSend[3] = 0x80;
      break;
    default:
      break;
  }

  pkgToSend[4] = 0x0D; //checksum
}

/**
 * Funcão que envia o pacote ao cfp
**/
void sendPkg() {
  Serial.println("Start send pkg.");
  digitalWrite(rs485Comunication, HIGH);
  
  for (byte i = 0; i < sizeof(pkgToSend); i++)
  {
    Serial1.write(pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(pkgToSend[i], HEX);
  }

  Serial1.flush();
};
/** --------------------  -------------------- **/


/** ------------ Recebimento de pacotes -------------------- **/
/**
 * Interrupção serial nas portas TX1 = 18, RX1 = 19 e recebe os bytes cfp
**/
void serialEvent1() {
  byte incomingByte;
  incomingByte = Serial1.read();
  
  Serial.println("New byte:");
  Serial.println(incomingByte, HEX);
  
  Timer1.initialize(byteExpiresTimer); // Inicia o timer que aguarda o prox byte
}
/** ------------------------------ **/

/**
 * Função timerCallback
 * Executada a cada fim do tempo do timer, e monta o pacote recebido pelo cfp
 **/
void timerCallback()
{
  Timer1.stop();
   
  Serial.println("End package:");
  Serial.println("-------------------");
}

/** ------------      -------------------- **/


/**
 * Arduino setup 
**/
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); 

  pinMode(rs485Comunication, OUTPUT); // Define a porta que manipula a recebimento e envio de msgs RS485
  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
  
  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote
}
/** ------------------------------ **/

/**
 * Arduino Loop 
**/
void loop() {
  if (test)
  {                                                     
    makePkg(18, 1);
    sendPkg();
  }
  
  test = false;
   digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
}
/** ------------------------------ **/
