// prototipos de funciones y defines
#include "Prototipos.h"
#define pinZonaValvula 11
#define pinPrincipalValvula 10
#define pinERROR 12
#define pinColector 18
#define pinCaldera 13
#define PIN_ZONA A3
#define LECTOR_COLECTOR A2
#define PIN_ACUMULADOR A1
#define LECTOR_UPS A0

// Variables teclado
byte pinesFilas[] = {9, 8, 7, 6};
byte pinesColumnas[] = {5, 4, 3, 2};

// Variables Clave de acceso
char letraAneterior = 0;
const char pin[] = "1234";
char pinIntroducido[5];
int indicepin = 0;

// Variables Apagado
unsigned long prevMillisApagado = 0;
estado sistema = apagado;

// variales temperatura
float temperaturaDeseada = 20.0;
bool encenderCalefaccion = false;
const float histeresis = 1.0;

// variables electrovalvula
estadosValvula valvulaZona = estadosValvula::Cerrado;
estadosValvula valvulaPrincipal = estadosValvula::Cerrado;
estadosValvula valvulaZonaAnterior = estadosValvula::Cerrado;
estadosValvula valvulaPrincipalAnterior = estadosValvula::Cerrado;
unsigned long tPrev_valvulaZona = 0;
unsigned long tPrev_valvulaPrincipal = 0;
const unsigned long PeriodoConmutacion = 1000;

// Activacion de caldera
bool caldera = false;

// Variables UPS
const float tension_deseada = 12.0;
unsigned long tPrev_ErrorUps = 0.0;
bool LEDErrorUps = false;

// Variables error SobreTemperatura
bool LEDErrorTemp = false;
const float temperaturaError = 75.0;
unsigned long tPrev_ErrorTemp = 0.0;

// Variable Colector
estadosValvula valvulaColector = estadosValvula::Cerrado;
estadosValvula valvulaColectorAnterior = estadosValvula::Cerrado;
unsigned long tPrev_valvulaColector = 0;
unsigned long tPrev_AperturaColector = 0;
const unsigned long PeriodoVaciadoColector = 2000;
int estadoColector = 0;

void setup()
{
    Serial.begin(115200);
    for (int nL = 0; nL <= 3; nL++) // inicializar los pines del teclado
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
    pinMode(pinColector, OUTPUT);
    pinMode(pinERROR, OUTPUT);
}

void loop()
{
    char letra = lecturaMatricial();
    unsigned long t_actual = millis();

    //Control de errores
    float voltajeAlimentacion = mapFloat(analogRead(LECTOR_UPS), 0.0, 1023.0, 0.0, 5.0);
    float voltajeReal = mapFloat(voltajeAlimentacion, 0.0, 4.0, 0.0, 12.0);
    ImprimirArduino("TensionUPS", voltajeReal);
    estadosAlimentacion UPS = estadoUPS(voltajeReal, tension_deseada + 0.1, tension_deseada);
    float temperaturaAcumulador = mapFloat(analogRead(PIN_ACUMULADOR), 0.0, 1023.0, -5.0, 80.0);
    ImprimirArduino("TemperaturaAcumulador", temperaturaAcumulador);
    
    if (UPS != estadosAlimentacion::ALIMENTACION_OK)
        blinkSinDelays(pinERROR, t_actual, 1000, 4000, tPrev_ErrorUps, LEDErrorUps);
    else if(temperaturaAcumulador >= temperaturaError) // prioridad al error de alimentacion
        blinkSinDelays(pinERROR, t_actual, 1000, 1000, tPrev_ErrorTemp, LEDErrorTemp);
    else
        digitalWrite(pinERROR, LOW);

    //Control del colector para vaciado del mismo
    float temperaturaColector = mapFloat(analogRead(LECTOR_COLECTOR), 0.0, 1023.0, -5.0, 80.0);
    ImprimirArduino("TemperaturaColector", temperaturaColector);
    switch (estadoColector)
    {
    case 0:
        if (temperaturaColector >= 70.0)
            estadoColector = 1;
        break;
    case 1:
        if (valvulaColector != estadosValvula::Abierto)
            activacionElectrovalvula(pinColector, t_actual, tPrev_valvulaColector, PeriodoConmutacion, valvulaColector, valvulaColectorAnterior);
        if (valvulaColector == estadosValvula::Abierto)
            estadoColector = 2;
        tPrev_AperturaColector = t_actual;
        break;

    case 2:
        if (t_actual - tPrev_AperturaColector >= PeriodoVaciadoColector)
        {
            if (valvulaColector != estadosValvula::Cerrado)
                activacionElectrovalvula(pinColector, t_actual, tPrev_valvulaColector, PeriodoConmutacion, valvulaColector, valvulaColectorAnterior);
        }
        if (valvulaColector == estadosValvula::Cerrado || temperaturaColector >= 70.0)
            estadoColector = 0;
        break;
    }

    //Lectura temperatura
    float temperatura = mapFloat(analogRead(PIN_ZONA), 0.0, 1023.0, -5.0, 80.0); // t en C
    ImprimirArduino("TemperaturaZona", temperatura);

    // maquina de estados de calefaccion
    switch (sistema)
    {
    case estado::apagado:
        // Apagado de Válvulas al apagar el sistema
        if (valvulaZona != estadosValvula::Cerrado)
            activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
        if (valvulaPrincipal != estadosValvula::Cerrado)
            activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
        // Apagado de caldera al apagar el sistema
        if (caldera)
        {
            caldera = false;
            digitalWrite(pinCaldera, caldera);
        }
        // Antirrebote con comprobado de Pin
        if (letra != letraAneterior)
        {
            letraAneterior = letra;
            if (letra != 0)
            {
                pinIntroducido[indicepin] = letra;
                indicepin++;
                Serial.println(pinIntroducido);
                if (indicepin > 3)
                {
                    pinIntroducido[indicepin] = '\0';
                    if (strcmp(pin, pinIntroducido) == 0)
                        sistema = estado::encendido;
                    indicepin = 0;
                }
            }
        }
        break;
    case estado::encendido:
        bool permutar = BotonApagado(letra, 'A', letraAneterior, t_actual, prevMillisApagado, 2000);
        if (permutar)
        {
            sistema = estado::apagado;
            letraAneterior = letra;
        }
        // Siempre que este encendido y sin error se abre el sistema de calefaccion
        if (UPS == estadosAlimentacion::ALIMENTACION_OK && temperaturaAcumulador < temperaturaError)
        {
            // Control calefaccion
            encenderCalefaccion = controlHisteresis(temperaturaDeseada, histeresis, temperatura, encenderCalefaccion);
            if (encenderCalefaccion)
            {
                // Abrir las valvulas.
                if (valvulaZona != estadosValvula::Abierto)
                    activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
                if (valvulaPrincipal != estadosValvula::Abierto)
                    activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
                // Se considera que el disparo de la caldera no tiene una histeresis por lo que procedemos a realizar la activacion
                if (temperaturaAcumulador < 45.0)
                    caldera = true;
                else
                    caldera = false;
                digitalWrite(pinCaldera, caldera);
            }
            else
            {
                if (valvulaZona != estadosValvula::Cerrado)
                    activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
                if (valvulaPrincipal != estadosValvula::Cerrado)
                    activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
                if (caldera)
                {
                    caldera = false;
                    digitalWrite(pinCaldera, caldera);
                }
            }
        }
        else
        {
            encenderCalefaccion = false; // Para que se respete la zona muerta cuando se recupera de un error
            if (valvulaZona != estadosValvula::Cerrado)
                activacionElectrovalvula(pinZonaValvula, t_actual, tPrev_valvulaZona, PeriodoConmutacion, valvulaZona, valvulaZonaAnterior);
            if (valvulaPrincipal != estadosValvula::Cerrado)
                activacionElectrovalvula(pinPrincipalValvula, t_actual, tPrev_valvulaPrincipal, PeriodoConmutacion, valvulaPrincipal, valvulaPrincipalAnterior);
            if (caldera)
            {
                caldera = false;
                digitalWrite(pinCaldera, caldera);
            }
        }

        break;
    }
    //imresiones
    ImprimirArduino("Sistema", sistema);
    ImprimirArduino("Caldera", caldera);
    ImprimirArduino("ValvulaZona", valvulaZona);
    ImprimirArduino("ValvulaPrincipal", valvulaPrincipal);
    ImprimirArduino("ValvulaColector", valvulaColector);
    float limiteSuperior = temperaturaDeseada + histeresis;
    ImprimirArduino("LimiteSuperior", limiteSuperior);
    float limiteInferior = temperaturaDeseada - histeresis;
    ImprimirArduino("LimiteInferior", limiteInferior);
    Serial.println("");
}
