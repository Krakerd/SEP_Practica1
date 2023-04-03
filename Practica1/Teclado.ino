char lecturaMatricial()
{
    char teclas[4][4] = {{'1', '2', '3', 'A'},
                         {'4', '5', '6', 'B'},
                         {'7', '8', '9', 'C'},
                         {'*', '0', '#', 'D'}};
    char tecla = 0;
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