//interrupt quand btn1 appuyé
void isr_btn1(){ 
  if(!btn1_state){
    btn1_state=true;
    //DEBUG_PRINT("Pushed");
    //on met en place le timeout
    timer.setTimeout(1000, clear_btn1);
  }
}


//interrupt quand btn2 appuyé
void isr_btn2(){
  if(btn1_state){
    btn1_state=false;
    //DEBUG_PRINT("Pushed 2");
    //DEBUG_PRINT("Bouteille");
    Nb_bouteilles++; 
    EEPROM.write(43, Nb_bouteilles >> 8);
    EEPROM.write(42, Nb_bouteilles & 0x00FF);
  }  
}


//On enleve le flag quand le timeout du 1er btn est atteint
void clear_btn1(){
  if(btn1_state){
    btn1_state=false;
    //DEBUG_PRINT("Btn1 TimeOut");  
  }
} 
