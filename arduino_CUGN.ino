
#include <stdio.h>
#include <string.h>
#include "DS1302.h"
#include "SimpleTimer.h"

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.println (x)
  #define DEBUG_PRINTDEC(x) Serial.println(x, DEC)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
#endif 


//Création de l'objet Timer
SimpleTimer timer;

//PINs pour DS21302
uint8_t CE_PIN   = 5;
uint8_t IO_PIN   = 6;
uint8_t SCLK_PIN = 7;

char buf[50];

//Flag pour état btn1
boolean btn1_state = false;

//Création de l'objet pour gestion DS1302
DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN);


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


void print_time()
{
  /* Récupération du temps */
  Time t = rtc.time();

  snprintf(buf, sizeof(buf), "%01d %04d-%02d-%02d %02d:%02d:%02d",
           t.day,
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);
  DEBUG_PRINT(buf);

}


//interrupt quand btn1 appuyé
void isr_btn1()
{ 
  if(!btn1_state)
  {
    btn1_state=true;
    DEBUG_PRINT("Pushed");
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
    DEBUG_PRINT("Pushed 2");
    DEBUG_PRINT("Bouteille"); 
  }  
}


//On enleve le flag quand le timeout du 1er btn est atteint
void clear_btn1()
{
  if(btn1_state)
  {
    DEBUG_PRINT("Btn1 TimeOut");  
    btn1_state=false;
  }
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
    
    Serial.begin(9600);
  
    rtc.write_protect(false);
    rtc.halt(false);

    /*Init A/M/J h */
    Time t(2012, 1, 28, 21, 37, 37, 3);

    /* Chargement de l'heure */
    rtc.time(t);
  
    
}

//Boucle principale
void loop() {
    //DEBUG_PRINT(readVcc());
    //print_time();
     timer.run();
}


