#include <SoftwareSerial.h>
#include <stdio.h>
#include <string.h>
#include <EEPROM.h>

#include "DS1302.h"
#include "SimpleTimer.h"
#include "arduino_CUGN.h"

//Nb Bouteilles
unsigned int Nb_bouteilles;

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT_CHAR(x) Serial.print (x)
  #define DEBUG_PRINT(x) Serial.println (x)
  #define DEBUG_PRINTDEC(x) Serial.println(x, DEC)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
#endif 


//Création de l'objet Timer
SimpleTimer timer;

char buf[50];

//Flag pour état btn1
boolean btn1_state = false;

//Création de l'objet pour gestion DS1302
//DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN);

// Serial:
char incoming_char=0;      //Will hold the incoming character from the Serial Port.
SoftwareSerial cell(7,8);  //Create a 'fake' serial port. Pin 8 is the Rx pin, pin 9 is the Tx pin.

// Conteneur:
char baseNumber[]="+33604049221"; //TODO numéro auquel les ping/update seront envoyes
//char baseNumber[]="+33631424719";
char containerID=1; // TODO l'id du conteneur courant, à modifier à chaque fois
char update1Sent=0;
char update2Sent=0;
char hourUpdate1=12; // Info: premiere update entre 12h et 14h
char hourUpdate2=19; // Info: deuxieme update entre 19h et 21h
char hourReset=0;
char retMomentToString[20];
SimpleTimer timerUpdates;
Moment currentTime;
int sCheckUpdate=60;


void momentToString(Moment mom){ //Remarque: cette construction est affreuse mais fonctionne, à refaire (mais tester)
  sprintf(retMomentToString, "%02d/%02d/%04d %02d:%02d:%02d\0", mom.date, mom.month, mom.year, mom.hour, mom.minute, mom.second);
}

void printTime(){
  momentToString(currentTime);
  Serial.println(retMomentToString);
}

Moment newMoment(int year, int month, int date, int hour, int minute, int second)
{
  Moment mom;
  mom.year = year;
  mom.month = month;
  mom.date = date;
  mom.hour = hour;
  mom.minute = minute;
  mom.second = second;
  return mom;
}

//Fonction Lecture VCC
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void getTime(int delay_s){
  DEBUG_PRINT("getTime");

   // empties the buffer
  while (cell.available() > 0){
    cell.read();
  }

  while (Serial.available() > 0){
    Serial.read();
  }
   
  cell.println("AT+CCLK?");
  //cell.write(34);
  delay(500);
  int taille=20;
  char str[taille];
   
  //Serial.println("debut");
  for (int i=0; i<=19; i++){
	cell.read();
    //Serial.write(cell.read());
  }
  //Serial.println("fin1");
   
  if (cell.available() >= taille){
    for (int i = 0; i <= taille-1; i++) {
      str[i] = cell.read();
      //Serial.write(str[i]);
    }
    //Serial.println("fin2");

    char Syear[3];
    Syear[0]=str[0];
    Syear[1]=str[1];
	Syear[2]='\0';
    int year=atoi(Syear);    

    char Smonth[3];
    Smonth[0]=str[3];
    Smonth[1]=str[4];
	Smonth[2]='\0';
    int month=atoi(Smonth);    

    char Sdate[3];
    Sdate[0]=str[6];
    Sdate[1]=str[7];
	Sdate[2]='\0';
    int date=atoi(Sdate);    

    char Shour[3];
    Shour[0]=str[9];
    Shour[1]=str[10];
	Shour[2]='\0';
    int hour=atoi(Shour);   

    char Sminute[3];
    Sminute[0]=str[12];
    Sminute[1]=str[13];
	Sminute[2]='\0';
    int minute=atoi(Sminute);   

    char Ssecond[3];
    Ssecond[0]=str[15];
    Ssecond[1]=str[16];
	Ssecond[2]='\0';
    int second=atoi(Ssecond);   

    Moment mom;
    mom.year = 2000+year;
    mom.month = month;
    mom.date = date;
    mom.hour = hour;
    mom.minute = minute;
    mom.second = second;

	if (year>=12 && year <=20 && month >=1 && month <= 12 && date >= 1 && date <= 31 && hour >= 0 && hour <= 24 && minute >= 0 && minute <= 60 && second >= 0 && second <= 60){
		currentTime.year=mom.year;
		currentTime.month=mom.month;
		currentTime.date=mom.date;
		currentTime.hour=mom.hour;
		currentTime.minute=mom.minute;
		currentTime.second=mom.second;

		momentToString(currentTime);
		Serial.println(retMomentToString);
	}
	else {
		Serial.println("Return is not a moment : no change done to currentTime");

		/*Serial.println(Syear);
		Serial.println(Smonth);
		Serial.println(Sdate);
		Serial.println(Shour);
		Serial.println(Sminute);

		Serial.println(year);
		Serial.println(month);
		Serial.println(date);
		Serial.println(hour);
		Serial.println(minute);*/
	}
  }
  else {
	Serial.println("moins de 20");
  }
}


//interrupt quand btn1 appuyé
void isr_btn1()
{ 
  if(!btn1_state)
  {
    btn1_state=true;
    //DEBUG_PRINT("Pushed");
    //on met en place le timeout
    timer.setTimeout(1000, clear_btn1);
  }
}


//interrupt quand btn2 appuyé
void isr_btn2()
{
  if(btn1_state)
  {
    btn1_state=false;
    //DEBUG_PRINT("Pushed 2");
    //DEBUG_PRINT("Bouteille");
    Nb_bouteilles++; 
    EEPROM.write(43, Nb_bouteilles >> 8);
    EEPROM.write(42, Nb_bouteilles & 0x00FF);
  }  
}


//On enleve le flag quand le timeout du 1er btn est atteint
void clear_btn1()
{
  if(btn1_state)
  {
    btn1_state=false;
    //DEBUG_PRINT("Btn1 TimeOut");  
  }
}

// Pour envoyer le premier SMS au démarrage (id, date, nbBouteilles, batterie)
void sendPing(){
	DEBUG_PRINT("sendingPing");
	getTime(sCheckUpdate);
	getTime(sCheckUpdate);
	getTime(sCheckUpdate);
	long indBattery=readVcc();
	char str[49]; // 10 + 2 + 1 + 2 + 1 + 2 + 1 + 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 5 + 1 + 10 (long, on sait jamais) = 49
	sprintf(str, "NYBI;PING;%02d;%02d-%02d-%04d-%02d:%02d:%02d;%05d;%04d", containerID, currentTime.date, currentTime.month, currentTime.year, currentTime.hour, currentTime.minute, currentTime.second, Nb_bouteilles, indBattery);
	sendSMS(str);
}

// Pour envoyer les SMS d'update (id, date, nbBouteilles, batterie)
void sendUpdatedCounter(){
	long indBattery=readVcc();
	char str[49]; // 2 + 49 = 51
	sprintf(str, "NYBI;UPDA;%02d;%02d-%02d-%04d-%02d:%02d:%02d;%05d;%04d", containerID, currentTime.date, currentTime.month, currentTime.year, currentTime.hour, currentTime.minute, currentTime.second, Nb_bouteilles, indBattery);
	sendSMS(str);
}

// Envoie un SMS
void sendSMS(char* str){
    cell.print("AT+CFUN=1"); // mode normal
	cell.write((byte)13);
	DEBUG_PRINT("sendingSMS");
    delay(60000);
        
	//cell.println("AT+CMGF=1"); // set SMS mode to text
	cell.print("AT+CMGS=");  // now send message...
	cell.write((byte)34); // ASCII equivalent of "
	cell.print(baseNumber);
	cell.write((byte)34);  // ASCII equivalent of "
	cell.write((byte)13);  // ASCII equivalent of Carriage Return
	delay(500); // give the module some thinking time
    cell.print(str);
	//Serial.println(str);
	cell.write((byte)26);  // ASCII equivalent of Ctrl-Z}

	DEBUG_PRINT("SMS sent");
    cell.print("AT+CFUN=0"); // mode minimal (mais rtc fonctionnelle)
	cell.write((byte)13);
}

// Regarde si on doit envoyer une update ou remettre les compteurs d'update a zero
void checkUpdates(){
	//Time t = rtc.time(); // recuperer le temps
	getTime(sCheckUpdate);
	DEBUG_PRINT("Checking Time");
	printTime();

	if (true && (!update1Sent && currentTime.hour >= hourUpdate1 && currentTime.hour <= hourUpdate1+1)){
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

void setTime(){
  cell.print("AT+CCLK=");
  cell.write((byte)34);
  cell.print("12/04/04,22:46:00+04"); //TODO yy/mm/dd, hh:mm:ss+04    changer ici pour mettre à l'heure la shield / arduino
  cell.write((byte)34);
  cell.write((byte)13);
}

//Setup
void setup() {
    pinMode(2, INPUT);
    pinMode(3, INPUT);
  
    //Activation Pull-up
    digitalWrite(2, HIGH);  
    digitalWrite(3, HIGH);
    
    // 0 = pin 2 ; 1 = pin 3
    attachInterrupt(0, isr_btn1, FALLING);
    attachInterrupt(1, isr_btn2, FALLING);   
    
	//Allumage de la SIM900
	pinMode(9, OUTPUT); 
 	digitalWrite(9,LOW);	
	delay(1000);
	digitalWrite(9,HIGH);
	delay(2500);
 	digitalWrite(9,LOW);
 	delay(3500);

    Serial.begin(9600);
    
    Nb_bouteilles = (EEPROM.read(43) << 8) | EEPROM.read(42);
  
	DEBUG_PRINT("Nombre de bouteilles : ");
    DEBUG_PRINTDEC(Nb_bouteilles);
	
  	//EEPROM.write(43,0); // TODO pour le setup uniquement
  	//EEPROM.write(42, 0); // idem

	delay(1000); //10 secondes de + pour allumer la shield, à enlever si démarrage auto

    cell.begin(9600);//9600 19200
	DEBUG_PRINT("Starting SIM900 Communication...");//SM5100B
	delay(3000);
	//cell.print("ATE1\r"); //local echo
    //setTime(); //TODO pour le setup uniquement
	DEBUG_PRINT("Setup done...");//SM5100B
	currentTime=newMoment(2012, 3, 31, 0, 0, 1);
	timerUpdates.setInterval(600000, checkUpdates);  //TODO mettre 600000 pour checker l'heure toutes les 10 minutes

	sendPing();
}

//Boucle principale
void loop() {
	if (cell.available()>0){ //If a character comes in from the cellular module...
		incoming_char=cell.read();    //Get the character from the cellular serial port.
		DEBUG_PRINT_CHAR(incoming_char);  //Print the incoming character to the terminal.
	}
	if (Serial.available())
      cell.write(Serial.read()); 


  //cell.println("AT+CCLK?");
  //cell.write(34);
	/*Moment mom;
    mom.year = 2012;
    mom.month = 3;
    mom.date = 31;
    mom.hour = 12;
    mom.minute = 42;
    mom.second = 15;
	char str[19]; // 2 + 49 = 51
  	sprintf(str, "%02d/%02d/%04d %02d:%02d:%02d", mom.date, 3, 2012, 12, 40, 25);
	//Serial.println(str);	
	char* ret;
	momentToString(mom);
	Serial.println(retMomentToString);*/

    timer.run();
    timerUpdates.run();
}


