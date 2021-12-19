#include <Arduino.h>
#include <Cfp.h>
#include "TimerOne.h" // Biblioteca usada para o timer
#include "TimerThree.h" // Biblioteca usada para o timer


/* Cfp configs*/
struct CFPStruct
{
  int address; // Define globalmente o endereço do cpf
  int maxAttemptsErrors = 0; // Define globalmente o numero de retentativa de erros
  byte receivedByteIndex = 0; // Define globalmente o index do pacote recebido
  byte operationCode; // Define globalmente o codigo de operação baseado na documentaça do cfp
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
void sendPkgToCfp() {
  clearReceivedPackage();                                                    
  stillWaitNextBit = true;

  cfp.sendPkg(cfpComponent.address, cfpComponent.operationCode);         
  Timer1.start();       
}

/* Função auxiliar para fazer retry em caso de erro */
void retrySendPkgToCfp() {
  if (attempts < cfpComponent.maxAttemptsErrors)
  { 
    sendPkgToCfp();
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
 Função responsável por, após o estouro do timer, se valido, realizar o fluxo de entender o pacote
  se não chamar o fluxo de retantativas
 */
void receivedPkgTimerCallback() {
  Timer1.stop();
  stillWaitNextBit = false;
  Serial.println("timer-------");
  Serial.println("Package received:");

  bool valid = cfp.pkgValidator(cfpComponent.pkgReceived, sizeof cfpComponent.pkgReceived, cfpComponent.operationCode);

  if (valid)
  {
    Serial.println("valid");
    cfp.getValuesFromCpfResponse(cfpComponent.pkgReceived, sizeof cfpComponent.pkgReceived);

    Serial.println(cfp.actualIn);
    Serial.println(cfp.actualOut);
    Serial.println(cfp.totalIn);
    Serial.println(cfp.totalOut);
    
  } else
  {
    Serial.println("invalid");
    retrySendPkgToCfp();
  }
}

/**
 * Função responsável por, perguntar ao cfp numero de contagem
*/
void askTotalsToCfp() {
  cfpComponent.operationCode = 0x02;
  sendPkgToCfp();
}

/*
Função responsável por zerar contagem
*/
void resetCounts() {
  cfpComponent.operationCode = 0x03;
  sendPkgToCfp();
}

/*
Função responsável por zerar totais
*/
void resetTotals() {
  cfpComponent.operationCode = 0x04;
  sendPkgToCfp();
}

void setup() {
  cfpComponent.address = 0x01;
  cfpComponent.operationCode = 0x02;
  cfpComponent.maxAttemptsErrors = 3;

  Serial1.begin(9600);
  Serial.begin(9600);

  Timer1.attachInterrupt(receivedPkgTimerCallback);  // Define a função que irá ser executada a cada fim de espera do pacote 
  Timer1.initialize(232800);
  Timer1.stop();

  Timer3.attachInterrupt(askTotalsToCfp);  // Define a função que irá ser executada a cada fim de espera do pacote 
  Timer3.initialize(10000000); // periodo de envio de pacote, 10 segundos
}

void loop() {}