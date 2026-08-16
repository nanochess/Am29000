/*
** Pruebas de && y ||
*/

prueba1(c)
  int c;
{
  return (c == 1) && (c == 2);
}

prueba2(c)
  int c;
{
  return (c == 1) || (c == 2);
}

prueba3(c)
  int c;
{
  return ((c == 1) || (c == 2)) && c == 3;
}

prueba4(c)
  int c;
{
  return ((c == 1) && (c == 2)) || c == 3;
}

prueba5(c)
  int c;
{
  return c == 1 && (c == 2 || c == 3);
}

prueba6(c)
  int c;
{
  return c == 1 || (c == 2 && c == 3);
}

/*
** Prueba si el caracter dado es una letra.
*/
letra(c)
  int c;
{
  c = c & 255;
  return (((c >= 'a') && (c <= 'z')) ||
          ((c >= 'A') && (c <= 'Z')) ||
           (c == '_'));
}
