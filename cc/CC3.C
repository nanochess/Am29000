/*
** Compilador de C para G11
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 26 de junio de 1995.
** Revisión: 27 de julio de 1995. Agrego comillas a los nombres, gracias a la
**                                nueva ampliación del compilador.
** Revisión: 23 de agosto de 1995. Incluyo el camino al directorio /c/.
** Revisión: 22 de noviembre de 1995. Incluyo el camino a la unidad c:
** Revisión: 25 de mayo de 1998. Modificaciones para compilarse con Fénix 1.
*/

#define FILE int
#define FENIX
#define color(a);

#include "D:/CCvars.c"    /* Variables y definiciones.           */
#include "D:/CCinter.c"   /* Interfaz con el usuario.            */
#include "D:/CCanasin.c"  /* Análisis sintáctico de alto nivel.  */
#include "D:/CCvarios.c"  /* Funciones de soporte.               */
#include "D:/CCexpr.c"    /* Análisis sintáctico de expresiones. */
#include "D:/CCgencod.c"  /* Generador de codigo.                */
