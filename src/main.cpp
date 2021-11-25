#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define byteExpiresTimer 1000000 // Define o timer em microsegundos
#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

bool stillWaitNextByte = true;

bool testButtonState = false;
int testButton = 7;

bool pkgButtonState = false;
int seePkgButton = 8;

bool sendTest = true;

struct CFP
{
  int operationCodeId; // Codigo de operaco do cfp
  int address;
  byte receivedByteIndex = 0;
  byte operationCode;
  byte pkgToSend[5];  // Define globalmente o pacote que irá ser enviado;
  byte pkgReceived[10]; // Define globalmente o pacote que irá ser recebido;
};

CFP cfpComponet; // Criado o componete cfp para controle;

/** ------------ Envio de pacotes -------------------- **/
// /**
//  * Função que recebe o identificador e retorna o byte hex codigo  de operaçao cfp
//  **/
// byte getCfpOperationCodeHex() {
//   byte cfpHexCode;
  
//   /** Switch que define codigo de operacao cfp **/
//   switch (cfpComponet.operationCodeId)
//   {
//     case 1: // id: 1 - Ler Modo de funcionamento
//       cfpHexCode = 0x01;
//       break;
//     case 2: // id: 2 - Ler Contagem
//       cfpHexCode = 0x02;
//       break;  
//     case 3: // id: 3 - Zerar contagem
//       cfpHexCode = 0x03;
//       break;  
//     case 4: // id: 4 - Zerar só as totalizações
//       cfpHexCode = 0x04;
//       break; 
//     case 6: // id: 6 - Ping
//       cfpHexCode = 0x0A;
//       break;  
//     default:
//       cfpHexCode = 0xFF; // Default, caso inválido
//       break;
//   }

//   return cfpHexCode;
// }

/**
 * Função que recebe o endereço do cfp e um id de codigo de operaco cfp 
**/
void makePkg() {
  byte checkSum;

  cfpComponet.pkgToSend[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  cfpComponet.pkgToSend[1] = cfpComponet.address; // byte de endereço do cfp;
  cfpComponet.pkgToSend[2] = 0x01; // tamanho, valor padrao para buffer que vamos mandar ao cfp;
  cfpComponet.pkgToSend[3] = cfpComponet.operationCode; // Uso da função auxiliar para pegar o codigo operacional em HEX
  
  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend) - 1; i++)
  { 
    checkSum = checkSum + cfpComponet.pkgToSend[i];  
  }
  
   cfpComponet.pkgToSend[4] = checkSum; //checksum
}

void clearSenderPackage() {
  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend); i++)
  {
    cfpComponet.pkgToSend[i] = 0x0;
  }
}

void  clearReceivedPackage() {
  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend); i++)
  {
    cfpComponet.pkgReceived[i] = 0x0;
  }

  cfpComponet.receivedByteIndex = 0;
}

/**
 * Funcão que envia o pacote ao cfp
**/
void sendPkg() {
  Serial.println("Send pkg.");
  digitalWrite(rs485Comunication, HIGH);
  
  // clearReceivedPackage();

  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend); i++)
  {
    Serial1.write(cfpComponet.pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(cfpComponet.pkgToSend[i], HEX);
  }

  Serial.println("---------------");
  Serial1.flush();
  digitalWrite(rs485Comunication, LOW);
   
 
  stillWaitNextByte = true;
  Timer1.restart();
};
/** --------------------  -------------------- **/


/** ------------ Recebimento de pacotes -------------------- **/
/**
 * Interrupção serial nas portas TX1 = 18, RX1 = 19 e recebe os bytes cfp
**/
void serialEvent1() {
  byte incomingByte;

  if(stillWaitNextByte) {
    Serial.println("receiving");
    while (Serial1.available())
    {
      incomingByte = Serial1.read();
      Serial.println("Received byte");
      Serial.println(incomingByte, HEX);
      
      cfpComponet.pkgReceived[cfpComponet.receivedByteIndex] = incomingByte; // Adiciona na posição atual o byte recebido
      cfpComponet.receivedByteIndex += 1; // Espera o próximo byte;
    
    }
    
    Timer1.restart();
  }  

}
/** ------------------------------ **/


/**
 * Função timerCallback
 * Executada a cada fim do tempo do timer, e monta o pacote recebido pelo cfp
 **/
void timerCallback()
{

  Timer1.stop();
  
  Serial.println("timer-------");
  Serial.println("Package received:");
  for (byte i = 0; i < sizeof(cfpComponet.pkgReceived); i++)
  {
    Serial.println(cfpComponet.pkgReceived[i], HEX);
  }
  Serial.println("-------------------");


  // cfpComponet.receivedByteIndex = 0; // Zera o index para novo pacote; 
  
}
/** ------------      -------------------- **/

/**
 * Arduino setup 
**/
void setup() {
  Timer1.initialize(1000000UL);
  
  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote 
  Timer1.stop();

  
  Serial.begin(9600);
  Serial1.begin(9600); 

  pinMode(rs485Comunication, OUTPUT); // Define a porta que manipula a recebimento e envio de msgs RS485
  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 

  pinMode(testButton, INPUT);
  pinMode(seePkgButton, INPUT);

}
/** ------------------------------ **/

/**
 * Arduino Loop 
**/
void loop() {

  
  testButtonState = digitalRead(testButton);

  pkgButtonState = digitalRead(seePkgButton);

  if (testButtonState || sendTest)
  {

    cfpComponet.operationCode = 2;                                          
    cfpComponet.address = 5;                                          
    makePkg();
    sendPkg();         

    sendTest = false;
  }
  
  
  if (pkgButtonState)
  {
    Serial.println("Package received -  button loop:");
    for (byte i = 0; i < sizeof(cfpComponet.pkgReceived); i++)
    {
      Serial.println(cfpComponet.pkgReceived[i], HEX);
    }
    Serial.println("-------------------");
  
  }

  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
}
/** ------------------------------ **/

