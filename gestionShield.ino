 void startShield(){
    DEBUG_PRINT("Starting Shield");
    pinMode(9, OUTPUT); 
    digitalWrite(9,LOW);	
    delay(1000);
    digitalWrite(9,HIGH);
    delay(2500);
    digitalWrite(9,LOW);
    delay(10000); //10 secondes de + pour allumer la shield, à enlever si démarrage auto


}

void sleepShield(){
    cell.print("AT+CFUN=0"); // mode sleep
    cell.write((byte)13);
    delay(20000);
}

void wakeShield(){
    cell.print("AT+CFUN=1"); // mode normal
    cell.write((byte)13);
    delay(20000);
}
