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
    wakeShield();
    printLine("sendingSMS");
    delay(60000);
        
    //cell.println("AT+CMGF=1"); // set SMS mode to text
    cell.print("AT+CMGF=1");
    cell.write((byte)13);
    delay(500);
    cell.print("AT+CMGS=");  // now send message...
    cell.write((byte)34); // ASCII equivalent of "
    cell.print(baseNumber);
    cell.write((byte)34);  // ASCII equivalent of "
    cell.write((byte)13);  // ASCII equivalent of Carriage Return
    delay(500); // give the module some thinking time
    cell.print(str);
    //Serial.println(str);
    cell.write(26);  // ASCII equivalent of Ctrl-Z}
    delay(20000);
    printLine("SMS sent");
    sleepShield();
}
 
