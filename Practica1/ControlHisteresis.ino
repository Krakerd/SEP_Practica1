/*Recibe el estado del sistema a controlar, ya sea caldera, colector o acmulador, aplicando un valor de 
histeresis se decide si se debe actuar sobre el sistema determinado

encenderCalefaccion = controlHisteresis(tA13, 1.0, 20.0, encenderCalefaccion);*/

/*Se pasa el valor de &resultado porque de esa manera si no ha habido cambios se retorna el mismo valor*/

bool controlHisteresis(float tObj, float hist, float valor, bool &resultado)
{
    if (valor > tObj + hist)
        resultado = LOW;
    if (valor < tObj - hist)
        resultado = HIGH;
    return resultado;
}