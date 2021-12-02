
#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

// Programa: Comunicação RS485  Arduino com CFP-100
// Autor: Adalbery Castro, João Luz, Maria do Prado
// Data: 11/30/2021
// Versão: 0.0.1 

#define byteExpiresTimer 16080 // Define o timer para 8,040 milisegundos 2x do tempo de tramissão

#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

// button configs -----
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int testButton = 7;
int buttonState;           
int lastButtonState = LOW; 
/**--------------- **/


bool stillWaitNextBit = false; // Variavel de controle de espera do proximo bit

struct CFP
{
  int address; // Define globalmente o endereço do cpf
  int sendRetries = 0; // Define globalmente o numero de retentativa de erros
  byte receivedByteIndex = 0; // Define globalmente o index do pacote recebido
  byte operationCode; // Define globalmente o codigo de operação baseado na documentaça do cfp, hoje temos suporte para: 
    // 0x01 - Ler modo de funcionamento
    // 0x02 - Ler contagem
    // 0x03 - Zerar contagem
    // 0x04 - Zerar só totalizações
    // 0x0A - Ping 

  byte pkgToSend[5];  // Define globalmente o pacote que irá ser enviado;
  byte pkgReceived[25]; // Define globalmente o pacote que irá ser recebido;
  
};

CFP cfpComponent; // Criado o componete cfp para controle;


/**
 * Função que "limpa" o pacote que iria ser enviado
**/
void clearSenderPackage() {
  for (byte i = 0; i < sizeof(cfpComponent.pkgToSend); i++)
  {
    cfpComponent.pkgToSend[i] = 0x0;
  }
}

/**
 * Função que monta o pacote
**/

void makePkg() {
  byte checkSum;

  cfpComponent.pkgToSend[0] = 0x80; // byte inicial, padrão no inicio do pacote;
  cfpComponent.pkgToSend[1] = cfpComponent.address; // byte de endereço do cfp;
  cfpComponent.pkgToSend[2] = 0x01; // tamanho, valor padrao para buffer que vamos mandar ao cfp;
  cfpComponent.pkgToSend[3] = cfpComponent.operationCode; // Uso da função auxiliar para pegar o codigo operacional em HEX
  
  for (byte i = 0; i < sizeof(cfpComponent.pkgToSend) - 1; i++)
  { 
    checkSum = checkSum + cfpComponent.pkgToSend[i];  
  }
  
   cfpComponent.pkgToSend[4] = checkSum; //checksum
}



/**
 * Função que "limpa" o pacote recebido
**/
void  clearReceivedPackage() {
  for (byte i = 0; i < sizeof(cfpComponent.pkgToSend); i++)
  {
    cfpComponent.pkgReceived[i] = 0x0;
  }

  cfpComponent.receivedByteIndex = 0;
}


/**
 * Funcão que envia o pacote montado ao cfp, 
 * Importante:
 *  Antes de rodar essa função, deve-se definir o address, o operationCode e rodar o método makePkg()
**/
void sendPkg() {
  Serial.println("Send pkg.");
  
  clearReceivedPackage();
  digitalWrite(rs485Comunication, HIGH);
  
  for (byte i = 0; i < sizeof(cfpComponent.pkgToSend); i++)
  {
    Serial1.write(cfpComponent.pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(cfpComponent.pkgToSend[i], HEX);
  }

  Serial.println("---------------");
  Serial1.flush();
  digitalWrite(rs485Comunication, LOW);
  
  stillWaitNextBit = true;
  Timer1.start();
};
/** --------------------  -------------------- **/


/** ------------ Recebimento de pacotes -------------------- **/
/**
 * Interrupção serial nas portas TX1 = 18, RX1 = 19 e recebe os bytes cfp
**/
void serialEvent1() {
  byte incomingByte;

  if(stillWaitNextBit) {
    Serial.println("receiving");

    while (Serial1.available())
    {
      incomingByte = Serial1.read();
      Serial.println("Received byte");
      Serial.println(incomingByte, HEX);
      
      cfpComponent.pkgReceived[cfpComponent.receivedByteIndex] = incomingByte; // Adiciona na posição atual o byte recebido
      cfpComponent.receivedByteIndex += 1; // Espera o próximo byte;
    
    }
    Timer1.start();
  }  
}
/** ------------------------------ **/

/**
 * Funções responsável por fazer 3 tentiavas de envio
 * */
void retrySendPkgToCfp() {
  if (cfpComponent.sendRetries < 3)
  {   
    sendPkg();
    cfpComponent.sendRetries++;
  } 
  else
  {
    Serial.println("Max attempts errors");
    cfpComponent.sendRetries = 0;
    stillWaitNextBit = false;
    /* void() do something after 3 errors  */
  }
  
}

/**
 * Função timerCallback
 * Executada a cada fim do tempo do timer, e monta o pacote recebido pelo cfp
 **/
void timerCallback()
{
  Timer1.stop();
  Serial.println("timer-------");
  Serial.println("Package received:");
 
  byte auxReceivedBytesSum = 0;
  byte lastValidReceivedBit = 0;
 
  for (byte i = 0; i < sizeof cfpComponent.pkgReceived; i++)
  {
    Serial.println(cfpComponent.pkgReceived[i], HEX);
    auxReceivedBytesSum = auxReceivedBytesSum + cfpComponent.pkgReceived[i];

    if (cfpComponent.pkgReceived[i] != 0x00)
    {
     lastValidReceivedBit = cfpComponent.pkgReceived[i];
    }
  }

  Serial.println("-------------------");
  
  auxReceivedBytesSum = auxReceivedBytesSum -lastValidReceivedBit; // Subtrai o ultimo valor do pacote da soma de todos os bits, já que o ultimo valor é checkSum.

  if (auxReceivedBytesSum ==lastValidReceivedBit) // Se o checkSum for valido, o pacote recebido faz sentido e está habito a ser utilizado
  {
    Serial.println("valid");
    
    stillWaitNextBit = false;
    cfpComponent.receivedByteIndex = 0; // Zera o index para novo pacote; 
  
    /** void translate response **/
  } else {
    Serial.println("invalid");
    retrySendPkgToCfp();
  }
}
/** ------------      -------------------- **/

/**
 * Arduino setup 
**/
void setup() {
  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote 
  Timer1.initialize(1000000UL);
  Timer1.stop();

  
  Serial.begin(9600);
  Serial1.begin(9600); 

  pinMode(rs485Comunication, OUTPUT); // Define a porta que manipula a recebimento e envio de msgs RS485
  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 

  pinMode(testButton, INPUT);
}
/** ------------------------------ **/

/**
 * Arduino Loop 
**/
void loop() {
  int reading = digitalRead(testButton);  
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
   if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        cfpComponent.address = 4;
        cfpComponent.operationCode = 0X02;   
                                                  
        makePkg();
        sendPkg();         
      }
    }
  }

  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
  lastButtonState = reading;
}
/** ------------------------------ **/

