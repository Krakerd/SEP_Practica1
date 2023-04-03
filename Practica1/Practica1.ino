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
uint8_t PIN_ZONA = A3;
float histeresis = 1.0;

// variables electrovalvula
estadosValvula valvulaZona = estadosValvula::Cerrado;
estadosValvula valvulaPrincipal = estadosValvula::Cerrado;
estadosValvula valvulaZonaAnterior = estadosValvula::Cerrado;
estadosValvula valvulaPrincipalAnterior = estadosValvula::Cerrado;
unsigned long tPrev_valvulaZona = 0;
unsigned long tPrev_valvulaPrincipal = 0;
unsigned long PeriodoConmutacion = 1000;

// Activacion de caldera
uint8_t PIN_ACUMULADOR = A1;
bool caldera = false;
uint8_t pinCaldera = 13;
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
    pinMode(pinZonaValvula, OUTPUT);
    pinMode(pinPrincipalValvula, OUTPUT);
    pinMode(pinCaldera, OUTPUT);
}

void loop()
{
    char letra = lecturaMatricial();
    unsigned long t_actual = millis();
    switch (sistema)
    {
    case estado::apagado:
        // Apagado de Válvulas al apagar el sistema
        if (valvulaZona != estadosValvula::Cerrado)
        {
            activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
            Serial.println("CERRANDO VALVULA ZONA");
        }
        if (valvulaPrincipal != estadosValvula::Cerrado)
        {
            activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
            Serial.println("CERRANDO VALVULA PRINCIPAL");
        }
        // Antirrebote con comprobado de Pin
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
        Serial.println(temperatura);
        encenderCalefaccion = controlHisteresis(temperaturaDeseada, histeresis, temperatura, encenderCalefaccion);
        if (encenderCalefaccion)
        {
            Serial.println("ENCENDER CALEFACCION");
            // Abrir las valvulas.
            if (valvulaZona != estadosValvula::Abierto)
            {
                activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
                Serial.println("ACTIVANDO VALVULA ZONA");
            }
            if (valvulaPrincipal != estadosValvula::Abierto)
            {
                activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
                Serial.println("ACTIVANDO VALVULA PRINCIPAL");
            }
            float temperaturaAcumulador = mapFloat(analogRead(PIN_ACUMULADOR), 0.0, 1023.0, -5.0, 80.0);
            // Se considera que el disparo de la caldera no tiene una histeresis por lo que procedemos a realizar la activacion
            if(temperaturaAcumulador < 45.0){
                caldera = true;
                digitalWrite(pinCaldera, caldera);
            } else{
                caldera = false;
            }
        }

        break;
    }
}