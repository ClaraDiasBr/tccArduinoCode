#include <Arduino.h>
#include <Cfp.h>
#include "TimerOne.h" // Biblioteca usada para o timer

// button configs -----
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int testButton = 7;
int buttonState;           
int lastButtonState = LOW; 
/**--------------- **/


/* Cfp configs*/
struct CFPStruct
{
  int address; // Define globalmente o endereço do cpf
  int maxAttemptsErrors = 0; // Define globalmente o numero de retentativa de erros
  byte receivedByteIndex = 0; // Define globalmente o index do pacote recebido
  byte operationCode; // Define globalmente o codigo de operação baseado na documentaça do cfp, hoje temos suporte para:   
  byte pkgToSend[5];  // Define globalmente o pacote que irá ser enviado;
  byte pkgReceived[25]; // Define globalmente o pacote que irá ser recebido;  
};
CFPStruct cfpComponent; // Criado o componete cfp para controle;
/* ----- */

/* Flux control variables*/
Cfp cfp(3); // Define o pino de comunicação RS485 e instancia a lib
bool stillWaitNextBit = false; // Variavel de controle de espera do proximo bit
int attempts = 0; // Variável de controle de tentativas de error
/* ----- */

/* Função auxiliar para limpar o pacote, tornando todos os valores = 0x00*/
void  clearReceivedPackage() {
  for (byte i = 0; i < sizeof(cfpComponent.pkgReceived); i++) cfpComponent.pkgReceived[i] = 0x0;
  cfpComponent.receivedByteIndex = 0;
}

/*Função auxiliar para definir as variáveis de controle padrões para envio de pacotes.*/
void setSendPkgParams(byte address, byte opertionCode, int maxAttemptsErrors = 0 ) {
  cfpComponent.address = address;
  cfpComponent.operationCode = opertionCode;
  if (!(maxAttemptsErrors == 0)) cfpComponent.maxAttemptsErrors = maxAttemptsErrors;
  
  clearReceivedPackage();                                                    
  stillWaitNextBit = true;

  cfp.sendPkg(cfpComponent.address, cfpComponent.operationCode);         
  Timer1.start();       
}

/* Função auxiliar para fazer retry em caso de erro */
void retrySendPkgToCfp() {
  if (attempts < cfpComponent.maxAttemptsErrors)
  {  
    setSendPkgParams(cfpComponent.address, cfpComponent.operationCode);
    attempts++;
  } 
  else
  {
    Serial.println("Max attempts errors");
    attempts = 0;
    stillWaitNextBit = false;
    /* void() do something after 3 errors  */
  }
  
}

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
      Serial.println(incomingByte, HEX);
      cfpComponent.pkgReceived[cfpComponent.receivedByteIndex] = incomingByte; // Adiciona na posição atual o byte recebido
      cfpComponent.receivedByteIndex += 1; // Espera o próximo byte;
    }
    Timer1.start();
  }  
}

/*
  Function responable por, após o estouro do timer, se valido, realizar o fluxo de entender o pacote
  se não chamar o fluxo de retantativas
 */
void timerCallback() {
  Timer1.stop();
  stillWaitNextBit = false;
  Serial.println("timer-------");
  Serial.println("Package received:");

  bool valid = cfp.pkgValidator(cfpComponent.pkgReceived, sizeof cfpComponent.pkgReceived);

  if (valid)
  {
    Serial.println("valid");
  } else
  {
    Serial.println("invalid");
    retrySendPkgToCfp();
  }
}

void setup() {
  Timer1.initialize(116080);
  Timer1.stop();
  Timer1.attachInterrupt(timerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote 

  Serial1.begin(9600);
  Serial.begin(9600);
  pinMode(testButton, INPUT);
}


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
        cfpComponent.address = 0x01;
        cfpComponent.operationCode = 0x03;

        setSendPkgParams(cfpComponent.address, cfpComponent.operationCode, 3);
      }
    }
  }
 
  lastButtonState = reading;
}