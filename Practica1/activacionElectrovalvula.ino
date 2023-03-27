void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, bool &estadoValvula){
    if(tactual - prev < T){
      digitalWrite(pin, HIGH);
    } else{
      digitalWrite(pin, LOW);
      estadoValvula = !estadoValvula;
    }
}