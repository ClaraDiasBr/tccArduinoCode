#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define byteExpiresTimer 8020 // Define o timer para 8,020 milisegundos o dobro do tempo de tramissão rs485
#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

bool test = true;

struct CFP
{
  int operationCodeId; // Codigo de operaco do cfp
  int receivedPkgSize; // Tamanho do pacote recebido, ele varia de acordo com codigo de operacao
  int address;
  byte pkgToSend[5];  // Define globalmente o pacote que irá ser enviado;
};

CFP cfpComponet; // Criado o componete cfp para controle;


/** ------------ Envio de pacotes -------------------- **/
/**
 * Função que recebe o identificador e retorna o byte hex codigo  de operaçao cfp
 **/
byte getCfpOperationCodeHex() {
  byte cfpHexCode;
  /** Switch que define codigo de operacao cfp **/
  switch (cfpComponet.operationCodeId)
  {
    case 1: // id: 1 - Ler Modo de funcionamento
      cfpHexCode = 0x01;
      break;
    case 2: // id: 2 - Ler Contagem
      cfpHexCode = 0x02;
      break;  
    case 3: // id: 3 - Zerar contagem
      cfpHexCode = 0x03;
      break;  
    case 4: // id: 4 - Zerar só as totalizações
      cfpHexCode = 0x04;
      break; 
    case 6: // id: 6 - Ping
      cfpHexCode = 0x0A;
      break;  
    default:
      cfpHexCode = 0xFF; // Default, caso inválido
      break;
  }

  return cfpHexCode;
}

/**
 * Função que recebe o endereço do cfp e um id de codigo de operaco cfp 
**/
void makePkg() {
  byte checkSum;

  cfpComponet.pkgToSend[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  cfpComponet.pkgToSend[1] = cfpComponet.address; // byte de endereço do cfp;
  cfpComponet.pkgToSend[2] = 0x01; // tamanho, valor padrao para buffer que vamos mandar ao cfp;
  cfpComponet.pkgToSend[3] = getCfpOperationCodeHex(); // Uso da função auxiliar para pegar o codigo operacional em HEX
  
  for (byte i = 0; i < sizeof( cfpComponet.pkgToSend); i++)
  { 
    checkSum = checkSum +  cfpComponet.pkgToSend[i];  
  }
  
   cfpComponet.pkgToSend[4] = checkSum; //checksum
}

/**
 * Funcão que envia o pacote ao cfp
**/
void sendPkg() {
  Serial.println("Start send pkg.");
  digitalWrite(rs485Comunication, HIGH);
  
  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend); i++)
  {
    Serial1.write(cfpComponet.pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(cfpComponet.pkgToSend[i], HEX);
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
    cfpComponet.operationCodeId = 3;                                          
    cfpComponet.address = 9;                                          
    makePkg();
    sendPkg();
  }
  
  test = false;
  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
}
/** ------------------------------ **/

