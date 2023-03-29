#include "Prototipos.h"

void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, estadosValvula &valvula, estadosValvula &estadoPrev){

    switch (valvula){
    case estadosValvula::Cerrado:
      digitalWrite(pin, HIGH);
      prev = tactual;
      estadoPrev = valvula;
      valvula = estadosValvula::Cambiando;
      break;
    case estadosValvula::Cambiando:
      if(tactual-prev > T){
        digitalWrite(pin, LOW);
        if(estadoPrev == estadosValvula::Abierto){
          valvula = estadosValvula::Cerrado;
        } else if(estadoPrev == estadosValvula::Cerrado){
          valvula = estadosValvula::Abierto;
        }
      }
      break;
    case estadosValvula::Abierto:
      digitalWrite(pin,HIGH);
      prev = tactual;
      estadoPrev = valvula;
      valvula = estadosValvula::Cambiando;
      break;
    }
}