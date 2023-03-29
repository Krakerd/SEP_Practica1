#include <Arduino.h>
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
char lecturaMatricial();
bool BotonApagado(char letra, char comparar, char *letraPrevia, unsigned long t, unsigned long &tPrevio, unsigned long periodo);
bool controlHisteresis(float tObj, float hist, float valor, bool &resultado);
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, bool &estadoValvula, bool &pulso);

enum estadosAlimentacion{ALIMENTACION_OK, ALIMENTACION_SOBRE, ALIMENTACION_DESC};
estadosAlimentacion estadoUPS(float voltaje, float limSup, float limInf);

enum estadosValvula{Cerrado = 0, Abierto = 1, Cambiando = 2};
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, estadosValvula &valvula, estadosValvula &estadoPrev);