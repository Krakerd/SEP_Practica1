bool BotonApagado(char letra, char comparar, char *letraPrevia, unsigned long t, unsigned long *tPrevio, unsigned long periodo)
{
    bool res = false;
    if (letra != comparar)
    {
        *tPrevio = t;
    }
    if (t - *tPrevio > periodo)
    {
        res = true;
        *tPrevio = t;
        *letraPrevia = letra;
    }
    return res;
}