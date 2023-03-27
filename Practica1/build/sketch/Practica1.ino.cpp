#include <Arduino.h>
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Practica1.ino"
// prototipos de funciones y defines
#include "Prototipos.h"
#define pinZonaValvula 13
#define pinPrincipalValvula 10

// Variables teclado
byte pinesFilas[] = {9, 8, 7, 6};
byte pinesColumnas[] = {5, 4, 3, 2};
char teclas[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}};

// Variables Clave de acceso
char letraAneterior = 0;
char pin[] = "1234";
char pinIntroducido[5];
int indicepin = 0;

// Variables Apagado
unsigned long prevMillisApagado = 0;
enum estado
{
    apagado,
    encendido
};
estado sistema = apagado;

// variales temperatura
float temperaturaDeseada = 20.0;
bool encenderCalefaccion = false;
uint8_t PIN_ZONA = A5;
float histeresis = 1.0;

// variables electrovalvula
bool electroValvulaZona = false;
bool electroValvulaPrincipal = false;
unsigned long tPrev_valvulaZona = 0;
unsigned long tPrev_valvulaPrincipal = 0;
unsigned long PeriodoConmutacion = 1000;

#line 42 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Practica1.ino"
void setup();
#line 56 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Practica1.ino"
void loop();
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\BlinkSinDelays.ino"
void blinkSinDelays(uint8_t pinLed, unsigned long tiempo, unsigned long T_ON, unsigned long T_OFF, unsigned long *tiempoPrev, bool *estadoLED);
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\BotonApagado.ino"
bool BotonApagado(char letra, char comparar, char *letraPrevia, unsigned long t, unsigned long &tPrevio, unsigned long periodo);
#line 8 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\ControlHisteresis.ino"
bool controlHisteresis(float tObj, float hist, float valor, bool &resultado);
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Teclado.ino"
char lecturaMatricial();
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\activacionElectrovalvula.ino"
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, bool &estadoValvula);
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\mapFloat.ino"
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
#line 42 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Practica1.ino"
void setup()
{
    Serial.begin(9600);
    for (int nL = 0; nL <= 3; nL++)
    {
        pinMode(pinesFilas[nL], OUTPUT);
        digitalWrite(pinesFilas[nL], HIGH);
    }
    for (int nC = 0; nC <= 3; nC++)
    {
        pinMode(pinesColumnas[nC], INPUT_PULLUP);
    }
}

void loop()
{
    char letra;
    unsigned long t_actual;
    switch (sistema)
    {
    case estado::apagado:
        letra = lecturaMatricial();
        t_actual = millis();
        if(electroValvulaPrincipal == true)
            activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, electroValvulaPrincipal);
        if(electroValvulaZona == true)
            activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, electroValvulaZona);
        if (letra != letraAneterior)
        {
            letraAneterior = letra;
            if (letra != 0)
            {
                pinIntroducido[indicepin] = letra;
                indicepin++;
                if (indicepin > 3)
                {
                    pinIntroducido[indicepin] = '\0';
                    if (strcmp(pin, pinIntroducido) == 0)
                    {
                        sistema = estado::encendido;
                    }
                    else
                    {
                        sistema = estado::apagado;
                    }
                    indicepin = 0;
                }
            }
        }
        break;
    case estado::encendido:
        letra = lecturaMatricial();
        t_actual = millis();
        bool permutar = BotonApagado(letra, 'A', &letraAneterior, t_actual, prevMillisApagado, 2000);
        if (permutar)
        {
            permutar = false;
            sistema = estado::apagado;
            prevMillisApagado = t_actual;
            letraAneterior = letra;
        }

        // Control temperatura
        float temperatura = mapFloat(analogRead(PIN_ZONA), 0.0, 1023.0, -5.0, 80.0); // t en C
        encenderCalefaccion = controlHisteresis(temperaturaDeseada, histeresis, temperatura, encenderCalefaccion);
        if (encenderCalefaccion)
        {
            // codigo de encendido del sistema.
            if (electroValvulaPrincipal == false)
            {
                activacionElectrovalvula(electroValvulaPrincipal, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, electroValvulaPrincipal);
            }
            if (electroValvulaZona == false)
            {
                activacionElectrovalvula(electroValvulaZona, t_actual, tPrev_valvulaZona, PeriodoConmutacion, electroValvulaZona);
            }
        }

        break;
    }
}
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\BlinkSinDelays.ino"
void blinkSinDelays(uint8_t pinLed, unsigned long tiempo, unsigned long T_ON, unsigned long T_OFF, unsigned long *tiempoPrev, bool *estadoLED)
{

    // conmutacion de estados
    if (tiempo - *tiempoPrev >= T_ON)
    {
        if (*estadoLED == HIGH)
        {
            *estadoLED = LOW;
            *tiempoPrev = tiempo;
        }
    }
    if (tiempo - *tiempoPrev >= T_OFF)
    {
        if (*estadoLED == LOW)
        {
            *estadoLED = HIGH;
            *tiempoPrev = tiempo;
        }
    }

    // escribir estado al led
    Serial.println(*estadoLED);
    digitalWrite(pinLed, *estadoLED);
}
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\BotonApagado.ino"
bool BotonApagado(char letra, char comparar, char *letraPrevia, unsigned long t, unsigned long &tPrevio, unsigned long periodo)
{
    bool res = false;
    if (letra != comparar)
    {
        tPrevio = t;
    }
    if (t - tPrevio > periodo)
    {
        res = true;
        tPrevio = t;
        *letraPrevia = letra;
    }
    return res;
}
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\ControlHisteresis.ino"
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
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\Teclado.ino"
char lecturaMatricial()
{
    char tecla = 0;
    // perparar los pines (se pone por razones de encapsulado)
    for (int nL = 0; nL <= 3; nL++)
    {
        pinMode(pinesFilas[nL], OUTPUT);
        digitalWrite(pinesFilas[nL], HIGH);
    }
    for (int nC = 0; nC <= 3; nC++)
    {
        pinMode(pinesColumnas[nC], INPUT_PULLUP);
    }
    // lectura de matriz
    for (int Linea = 0; Linea <= 3; Linea++)
    {
        digitalWrite(pinesFilas[Linea], LOW);
        for (int Columna = 0; Columna <= 3; Columna++)
        {
            if (digitalRead(pinesColumnas[Columna]) == LOW)
            {
                tecla = teclas[Linea][Columna];
                break;
            }
        }
        digitalWrite(pinesFilas[Linea], HIGH);
    }
    return tecla;
}
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\activacionElectrovalvula.ino"
void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long &prev, unsigned long T, bool &estadoValvula){
    if(tactual - prev < T){
      digitalWrite(pin, HIGH);
    } else{
      digitalWrite(pin, LOW);
      estadoValvula = !estadoValvula;
    }
}
#line 1 "D:\\Documentos\\UNI\\Semestre 6\\Sistemas Electrónicos Programables\\Practica1\\mapFloat.ino"
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
