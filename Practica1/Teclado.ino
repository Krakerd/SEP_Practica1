char lecturaMatricial()
{
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