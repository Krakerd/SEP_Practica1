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