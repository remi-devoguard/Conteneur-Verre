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

void setTime(){
  cell.print("AT+CCLK=");
  cell.write((byte)34);
  cell.print("12/04/16,20:37:00+04"); //TODO yy/mm/dd, hh:mm:ss+04    changer ici pour mettre à l'heure la shield / arduino
  cell.write((byte)34);
  cell.write((byte)13);
}

