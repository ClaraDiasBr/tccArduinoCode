#include <Arduino.h>
#include "TimerOne.h" // Biblioteca usada para o timer

#define byteExpiresTimer 8040 // Define o timer para 8,040 milisegundos 2x do tempo de tramissão

#define rs485Comunication 3 // Define porta de que define o estado da comunicação rs485

// button configs -----
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int testButton = 7;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW; 
/**--------------- **/


bool stillWaitNextByte = true;

struct CFP
{
  int operationCodeId; // Codigo de operaco do cfp
  int address;
  int sendRetries = 0;
  byte receivedByteIndex = 0;
  byte operationCode;
  byte pkgToSend[5];  // Define globalmente o pacote que irá ser enviado;
  byte pkgReceived[10]; // Define globalmente o pacote que irá ser recebido;
};

CFP cfpComponet; // Criado o componete cfp para controle;


/**
 * Função que monta o pacote
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
  
  clearReceivedPackage();
  digitalWrite(rs485Comunication, HIGH);
  
  for (byte i = 0; i < sizeof(cfpComponet.pkgToSend); i++)
  {
    Serial1.write(cfpComponet.pkgToSend[i]); // Envio de cada bit da mensagem
    Serial.println(cfpComponet.pkgToSend[i], HEX);
  }

  Serial.println("---------------");
  Serial1.flush();
  digitalWrite(rs485Comunication, LOW);
  
  stillWaitNextByte = true;
  Timer1.start();
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
    
    Timer1.start();
  }  

}
/** ------------------------------ **/

/**
 * Funções responsável por fazer 3 tentiavas de envio, se não fazer algo
 * */
void retrySendPkgToCfp() {
  if (cfpComponet.sendRetries < 3)
  {   
    sendPkg();
    digitalWrite(rs485Comunication, LOW);
    cfpComponet.sendRetries++;
  } 
  else
  {
    Serial.println("Max attempts errors");
    cfpComponet.sendRetries = 0;
    /* do something if 3 errors  */
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
  byte lastValidReceivedByte;

  byte i = 0;
  while (cfpComponet.pkgReceived[i] != 0x00)
  {
    Serial.println(cfpComponet.pkgReceived[i], HEX);
    auxReceivedBytesSum = auxReceivedBytesSum + cfpComponet.pkgReceived[i];

    lastValidReceivedByte = cfpComponet.pkgReceived[i];
    
    i++;
  }
 
  Serial.println("-------------------");
  
  auxReceivedBytesSum = auxReceivedBytesSum - lastValidReceivedByte; //

  if (auxReceivedBytesSum == lastValidReceivedByte)
  {
    Serial.println("valid");
    
    stillWaitNextByte = false;
    cfpComponet.receivedByteIndex = 0; // Zera o index para novo pacote; 
    
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
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        cfpComponet.operationCode = 2;                                          
        cfpComponet.address = 5;                                          
        makePkg();
        sendPkg();         
      }
    }
  }

  digitalWrite(rs485Comunication, LOW); // Define por padrão esperar pacotes 
  lastButtonState = reading;
}
/** ------------------------------ **/

