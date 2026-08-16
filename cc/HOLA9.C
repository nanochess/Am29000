/*
** Prueba de estructuras
*/

struct sopita {
  int a;
  int b;
  int c;
};

struct sopita a;
struct sopita b;

struct sopita *c;
struct sopita *d;

struct sopita leer();
void hola();

main()
{
  a = b;
  b = a;
  c = &a;
  d = &b;
  hola(5, a, 7);
  b = leer(a, b, 1);
}

hola(num1, estructura, num2)
  int num1;
  struct sopita estructura;
  int num2;
{
  b = estructura;
  num1 += num2;
}

struct sopita leer(a, b, c)
  struct sopita a;
  struct sopita b;
  int c;
{
  if (c)
    return a;
  else
    return b;
}

