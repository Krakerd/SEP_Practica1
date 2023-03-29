#include "Prototipos.h"

estadosAlimentacion estadoUPS(float voltaje, float limSup, float limInf){
    static estadosAlimentacion resultado = estadosAlimentacion::ALIMENTACION_OK;
    if(voltaje > limSup){resultado = estadosAlimentacion::ALIMENTACION_SOBRE;}
    if(voltaje < limInf){resultado = estadosAlimentacion::ALIMENTACION_DESC;}
    if(voltaje < limSup && voltaje > limInf){resultado = estadosAlimentacion::ALIMENTACION_OK;}
    return resultado;
}