#include <SoftwareSerial.h>
#include <stdio.h>
#include <string.h>
#include <EEPROM.h>

#include "SimpleTimer.h"
#include "arduino_CUGN.h"

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT_CHAR(x) Serial.print (x)
  #define DEBUG_PRINT(x) Serial.println (x)
  #define DEBUG_PRINTDEC(x) Serial.println(x, DEC)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
#endif 

//Nb Bouteilles
unsigned int Nb_bouteilles;

//Création de l'objet Timer
SimpleTimer timer;

//Flag pour état btn1
boolean btn1_state = false;

// Serial:
char incoming_char=0;      //Will hold the incoming character from the Serial Port.
SoftwareSerial cell(7,8);  //Create a 'fake' serial port. Pin 8 is the Rx pin, pin 9 is the Tx pin.

// Conteneur:
char baseNumber[]="+33679552469"; //TODO numéro auquel les ping/update seront envoyes
//char baseNumber[]="+33604049221"; //TODO numéro auquel les ping/update seront envoyes
//char baseNumber[]="+33631424719";
char containerID=99; // TODO l'id du conteneur courant, à modifier à chaque fois
char update1Sent=0;
char update2Sent=0;
char hourUpdate1=12; // Info: premiere update entre 12h et 14h
char hourUpdate2=19; // Info: deuxieme update entre 19h et 21h
char hourReset=0;
char retMomentToString[20];
SimpleTimer timerUpdates;
Moment currentTime;
int sCheckUpdate=60;

void printLine(char* str){
	Serial.print(str);
	Serial.write((byte)13);
}

void firstInit()
{
      EEPROM.write(43,0); // TODO pour le setup uniquement
      EEPROM.write(42, 0); // idem
      setTime();
}


// Regarde si on doit envoyer une update ou remettre les compteurs d'update a zero
void checkUpdates(){
  
    DEBUG_PRINT("Checking Time");
    getTime(sCheckUpdate);
	
    printTime();
    if (!update1Sent && currentTime.hour >= hourUpdate1 && currentTime.hour <= hourUpdate1+1){
	DEBUG_PRINT("Update1");
	sendUpdatedCounter();
	update1Sent=1;
    }
	//else if (false && (!update2Sent && currentTime.hour >= hourUpdate2 && currentTime.hour <= hourUpdate2+1)){ // TODO passer a false si une seule update
	//	DEBUG_PRINT("Update2");
	//	sendUpdatedCounter();
	//	update2Sent=1;
	//}

    if (currentTime.hour >= hourReset && currentTime.hour <= hourReset+1){
  	update1Sent=0;
	update2Sent=0;
	DEBUG_PRINT("Update Reset");
    }
}


//Setup
void setup() {
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    
    Serial.begin(9600);
  
    //Activation Pull-up
    digitalWrite(2, HIGH);  
    digitalWrite(3, HIGH);
    
    Nb_bouteilles = (EEPROM.read(43) << 8) | EEPROM.read(42);
    DEBUG_PRINT("Nombre de bouteilles : ");
    DEBUG_PRINTDEC(Nb_bouteilles);

    // 0 = pin 2 ; 1 = pin 3
    attachInterrupt(0, isr_btn1, FALLING);
    attachInterrupt(1, isr_btn2, FALLING);   

    startShield();
   
    cell.begin(9600);//9600 19200
    currentTime=newMoment(2012, 3, 31, 0, 0, 1);
    
    timerUpdates.setInterval(3000000, checkUpdates);  //TODO mettre 600000 pour checker l'heure toutes les 10 minutes

    sendPing();
}

//Boucle principale
void loop() {
	if (cell.available()>0){ //If a character comes in from the cellular module...
		incoming_char=cell.read();    //Get the character from the cellular serial port.
		DEBUG_PRINT_CHAR(incoming_char);  //Print the incoming character to the terminal.
	}
	if (Serial.available()) cell.write(Serial.read());

    timer.run();
    timerUpdates.run();
}


