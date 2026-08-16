/*
** Compilador de C para G11
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 26-jun-1995.
** Revisión: 27-jul-1995. Agrego comillas a los nombres, gracias a la
**                        nueva ampliación del compilador.
** Revisión: 23-ago-1995. Incluyo el camino al directorio /c/.
** Revisión: 22-nov-1995. Incluyo el camino a la unidad c:
** Revisión: 25-may-1998. Modificaciones para compilarse con Fénix 1.
** Revisión: 01-jul-1998. Deja de ser una aplicación de modo texto y se
**                        convierte en una aplicación del Sistema Fénix.
** Revisión: 02-jul-1998. Gracias a la selección de nombres de archivo con un
**                        click en el sistema de ventanas, se ponen nombres
**                        más descriptivos para los archivos que componen el
**                        compilador de C.
** Revisión: 09-oct-1998. No es necesario indicar el camino a los archivos,
**                        gracias a nuevas mejoras del compilador de C.
** Revisión: 14-oct-1998. El compilador paso a través de un riguroso proceso
**                        de ANSIficación, ahora todas las funciones tienen
**                        prototipo y no hay ningún problema de tipos.
*/

#define FILE int

typedef unsigned short wchar_t;

         /* Variables y definiciones */
#include "Compilador de C (variables).c"

         /* Interfaz con el usuario */
#include "Compilador de C (interfaz).c"

         /* Análisis sintáctico de alto nivel */
#include "Compilador de C (análisis).c"

         /* Rutinas de apoyo */
#include "Compilador de C (varios).c"

         /* Análisis de expresiones */
#include "Compilador de C (expresiones).c"

         /* Generador de codigo */
#include "Compilador de C (generador).c"
