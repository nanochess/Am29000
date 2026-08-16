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
/*
** Compilador de C para G11.
** Definiciones de variables.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 2 de junio de 1995.
** Revisión: 25 de julio de 1995. Se agregan N_ANDB y N_ORB, se comentan
**                                las operaciones.
** Revisión: 25 de julio de 1995. Se agrega N_TRI.
** Revisión: 25 de julio de 1995. Se elimina la variable modo_c.
** Revisión: 25 de julio de 1995. Se agrega las variables dentro_pp, nivel_if
**                                y evadir_nivel.
** Revisión: 25 de julio de 1995. Se agregan las macros de tipos para futura
**                                expansión del compilador.
** Revisión: 26 de julio de 1995. Se agregan las macros N_A??? para operadores
**                                de asignación, desaparecen N_GBYTE y N_GPAL.
** Revisión: 26 de julio de 1995. Se agrega la macro N_AIXP.
** Revisión: 26 de julio de 1995. Se elimina la macro E_ASM.
** Revisión: 27 de julio de 1995. Se agrega la macro N_COMA.
** Revisión: 10 de agosto de 1995. Se agregan las macros CHAR, SHORT, USHORT,
**                                 UINT, VOID, FLOAT, y DOUBLE.
** Revisión: 10 de agosto de 1995. Se agregan las variables sig_tipo,
**                                 tipo_basico, tipo_proc, t_char, t_short,
**                                 t_int, t_ushort, t_uint, t_float, t_double,
**                                 t_achar, t_void y la matriz tipos.
** Revisión: 11 de agosto de 1995. Se agrega la variable t_func.
** Revisión: 11 de agosto de 1995. Se agrega la variable multi.
** Revisión: 12 de agosto de 1995. Nueva variable. nivel_include. Desaparecen
**                                 las variables c_funcion_actual,
**                                 c_comienzo_funcion, c_linea_actual, y
**                                 c_dentro_funcion. Nueva matriz. incl.
** Revisión: 22 de agosto de 1995. Nueva macro STRUCT, nueva variable
**                                 lista_struct.
** Revisión: 23 de agosto de 1995. Se agrega la variable ultima_estruct,
**                                 las macros EST_QUE_ES, EST_ES_UNION,
**                                 EST_TAM, EST_LISTA, EST_SIG, EST_NOMBRE,
**                                 MIE_TIPO, MIE_POSICION, MIE_SIG, y
**                                 MIE_NOMBRE.
** Revisión: 24 de agosto de 1995. Se agregan las variables lista_enum y
**                                 ultimo_enum, las macros ENUM_VALOR,
**                                 ENUM_SIG y ENUM_NOMBRE.
** Revisión: 5 de septiembre de 1995. Se agrega N_COPIA y N_RESULTA.
** Revisión: 6 de septiembre de 1995. Se agrega la macro MAX_INIC, y las
**                                    variables nodo_inic y vars_inicializadas.
** Revisión: 12 de septiembre de 1995. Ahora MAX_INIC es por 3.
** Revisión: 12 de septiembre de 1995. Se añade N_CFLOAT, N_CDOUBLE, N_IGUALPF,
**                                     N_MAYORPF, N_SUMAPF, N_RESTAPF, N_MULPF,
**                                     N_DIVPF, N_CEROPF, N_NUMPF, N_ENTPF,
**                                     N_PFENT, N_PARF y N_IXF.
** Revisión: 12 de septiembre de 1995. Nueva matriz regsf[].
** Revisión: 22 de noviembre de 1995. Se añade MAX_CONST, TAM_DOUBLE, la matriz
**                                    constantes y la variable const_definidas.
** Revisión: 29 de noviembre de 1995. Se añaden las macros car_act() y
**                                    nueva_etiq(), también N_CONVFD.
** Revisión: 30 de noviembre de 1995. Se añaden las macros N_CDI y N_CFI para
**                                    optimización interna.
** Revisión: 1o. de diciembre de 1995. Se añaden las macros N_ENTF y N_CONVDF.
** Revisión: 1o. de enero de 1996. Mejoras mínimas.
** Revisión: 20 de junio de 1996. Se añade la macro NIVEL.
** Revisión: 6 de mayo de 1998. Se convierten los char a unsigned char,
**                              para maxima portabilidad.
** Revisión: 7 de mayo de 1998. Se crean las estructuras nodo y inic.
** Revisión: 8 de mayo de 1998. Se crea la estructura sentencia, nueva
**                              variable funcion.
** Revisión: 8 de mayo de 1998. Empieza la conversión a G11.
** Revisión: 9 de mayo de 1998. Nuevo arreglo virtuales[], nueva variable
**                              variables_virtuales.
** Revisión: 11 de mayo de 1998. Nuevo arreglo clase_argumento[].
** Revisión: 12 de mayo de 1998. Fin de conversión a G11. Muchos cambios.
*/
 
#define PROGRAMA     "Compilador de C para G11  (c) Oscar Toledo G.1995-1998"

#define NO           0
#define SI           1

#define NULL         0

/* Define parametros de la tabla de nombres */

#define TAM_SIM      28
#define TAM_TABLA    12600
#define NUM_GLBS     400
#define INICIO_GLB   tabla
#define FIN_GLB      (INICIO_GLB+NUM_GLBS*TAM_SIM)
#define INICIO_LOC   (FIN_GLB+TAM_SIM)
#define FIN_LOC      tabla+(TAM_TABLA-TAM_SIM)

/* Define formato de los nombres */

#define NOMBRE       0
#define IDENT        17  /* Tipo de identificador */
#define CLASE        18  /* Tipo de almacenamiento */
#define NIVEL        19  /* Nivel de declaración */
#define TIPO         20  /* Apuntador a cadena de tipo */
#define POSICION     24  /* Posición real (globales y locales asignadas), */
                         /* o número de variable virtual */

/* Tamaño máximo de los nombres */

#define TAM_NOMBRE   17
#define MAX_NOMBRE   16

/* Valores posibles para "IDENT" */

#define VARIABLE     1
#define ETIQUETA     2
/*#define FUNCION      12   Definido más abajo*/
#define TYPEDEF      4

/* Valores posibles para "CLASE" */

#define STATIC       1
#define AUTO         2
#define EXTERN       3

/* Valores posibles para "TIPO" */

#define CHAR         0
#define SHORT        1
#define INT          2
#define USHORT       3
#define UINT         4
#define FLOAT        5
#define DOUBLE       6
#define VOID         7  /* Esta es una caracteristica del C K&R añadida */
                        /* con el *NIX versión 7, así cómo la asignación */
                        /* y paso cómo parametros de estructuras, y enum. */
#define STRUCT       8  /* Se considera cómo un tipo básico */
#define ENUM         9

#define APUNTADOR   10
#define MATRIZ      11
#define FUNCION     12

#define FUNC_REF     0
#define FUNC_TIPO    1
#define FUNC_DEF     2

/* Define los desplazamientos en la cola de while's */

#define B_ANTERIOR   0
#define B_PILA       1
#define B_BUCLE      2
#define B_FIN        3

/* Define el almacenamiento de cadenas */

#define TAM_LITS     2000
#define MAX_LITS     (TAM_LITS-1)

/* Define la linea de entrada */

#define TAM_LINEA    512
#define MAX_LINEA    (TAM_LINEA-1)

/* Define el almacenamiento de macros */

#define TAM_MAC      10000
#define MAX_MAC      (TAM_MAC-1)

/* Define el espacio disponible para substitución de argumentos de macros */

#define TAM_AMAC     512
#define MAX_AMAC     (TAM_AMAC-1)

/* Define el espacio disponible para definiciones de tipos */

#define TAM_TIPOS    5000
#define MAX_TIPOS    (tipos+TAM_TIPOS)

/* Define los tipos de sentencias */

#define E_IF         1
#define E_WHILE      2
#define E_RETURN     3
#define E_BREAK      4
#define E_CONT       5
#define E_EXPR       6

/* Número máximo de cases * 2 */

#define MAX_CASOS    100

/* Número máximo de #include * 5 */

#define MAX_INCL     30

/* Definiciones de estructura */

#define EST_QUE_ES    0   /* char, indica si es un rótulo de struct o enum */
#define EST_ES_UNION  1   /* char, indica si es una unión o una estructura */
#define EST_TAM       2   /* int, tamaño total de la estructura/unión */
#define EST_LISTA     6   /* char*, lista de miembros */
#define EST_SIG      10   /* char*, siguiente rótulo */
#define EST_NOMBRE   14   /* char[], rótulo */

/* Definiciones de miembros */

#define MIE_TIPO      0   /* char*, tipo del miembro */
#define MIE_POSICION  4   /* int, posición dentro de la estructura */
#define MIE_SIG       8   /* char*, siguiente miembro */
#define MIE_NOMBRE   12   /* nombre del miembro */

/* Definiciones de enumeradores */

#define ENUM_VALOR    0   /* int, valor del enumerador */
#define ENUM_SIG      4   /* char*, siguiente enumerador */
#define ENUM_NOMBRE   8   /* char[], nombre del enumerador */

/* Reserva espacio para las variables */
#define MAX_VIRTUALES   120    /* Total variables virtuales x 3 */
#define MAX_ARGS         32
#define TAM_DOUBLE  8    /* Tamaño en enteros del tipo double */
#define MAX_INIC   16   /* Máximo número de inicializaciones de variables */
                        /* automáticas locales. */
#define MAX_BUFR  512 /* Buffer para instrucciones retrasadas (delay slot) */
#define MAX_LIN     8 /* Total máximo de instrucciones almacenadas,f */
                      /* debe ser un multiplo de 2. */

unsigned char *ap_glb,  /* Apuntadores a las sigs. entradas libres en */
              *ap_loc;  /* la tabla de nombres */

int *ultimo_bucle;      /* Apuntador al último bucle abierto */

int ap_lit;             /* Apuntador a la sig. entrada para las cadenas */

int ap_mac;             /* Indice en el buffer de macros */

int pos_linea,          /* Apuntadores a las lineas de análisis */
    pos_linea_m;

/* Almacenamiento miscelaneo */

int sig_etiq,           /* Siguiente etiqueta disponible */
    etiq_lit,           /* Etiqueta para el buffer de cadenas */
    pila,               /* Apuntador de pila del compilador (memoria) */
    pila_regs,          /* Apuntador de pila del compilador (registros) */
    total_regs,         /* Total de registros requeridos para argumentos */
    pila_args,          /* Pila de argumentos (funciones) */
    nivel,              /* No. de bloques abiertos */
    errores,            /* No. de errores detectados */
    pausa,              /* Indica si se detiene en caso de error */
    eof,                /* Indica el final del archivo de entrada */
    desvio_salida,      /* Indica desvio de la salida a la consola */
    comienzo_funcion,   /* Linea de comienzo de la funcion actual */
    linea_actual,       /* Linea en el archivo actual */
    dentro_funcion,     /* Indica si esta dentro de una funcion */
    dentro_pp,          /* Indica si esta dentro del preprocesador */
    nivel_if,           /* Nivel de anidamiento de #if... */
    nivel_incl,         /* Nivel de anidamiento de #include */
    ultima_sentencia,   /* Ultima sentencia procesada */
    evadir_nivel,       /* Nivel que esta evadiendo en el preprocesador */
    dentro_switch,      /* Indica si esta dentro de una sentencia switch */
    etiqueta_default,   /* Etiqueta para el default */
   *inicio_lista,       /* Inicio de la lista de cases */
   *sig_case,           /* Siguiente posición disponible para un case */
    casos[MAX_CASOS],   /* Hasta 100 cases */
    incl[MAX_INCL];     /* Almacenamiento de #include */

FILE *entrada,           /* Archivo de entrada */
     *salida,            /* Archivo de salida */
     *entrada2;          /* Archivo #include */

unsigned
char *funcion_actual,   /* Apuntador a la definicion de la función actual */
     *sig_tipo,         /* Sig. posición disponible en la tabla de tipos */
     *tipo_basico,      /* Tipo básico de la declaración actual */
     *tipo_proc,        /* Tipo procesado */
     *t_char,           /* Tipo char o unsigned char */
     *t_short,          /* Tipo short */
     *t_int,            /* Tipo int o long */
     *t_ushort,         /* Tipo unsigned short */
     *t_uint,           /* Tipo unsigned int o unsigned long */
     *t_float,          /* Tipo float */
     *t_double,         /* Tipo double */
     *t_void,           /* Tipo void */
     *t_achar,          /* Tipo apuntador a char */
     *t_func;           /* Función que retorna int */

unsigned
char *lista_estruct,    /* Lista de nombres de estructuras */
     *ultima_estruct,   /* Ultima estructura definida */
     *lista_enum,       /* Lista de constantes de enumeradores */
     *ultimo_enum;      /* Ultimo enumerador definido */

unsigned char *ap_c;    /* Apuntador de trabajo */
int *ap_e;              /* Apuntador de trabajo */
int pos_global;         /* Posición para variables estáticas */
int usa_expr;           /* Indica si se usa el resultado de la expr. */

int vars_inicializadas;  /* Variables inicializadas */

/*
** Nodo del árbol de expresiones
*/
struct nodo {
  struct nodo *izq;
  struct nodo *der;
  int oper;
  int esp;
  int regs;
};

/*
** Tipos de sentencias para la representación interna de la función
*/
enum tipo_sentencia {
  t_if, t_while, t_do, t_for, t_switch,
  t_case, t_default, t_goto, t_etiqueta,
  t_return, t_break, t_continue, t_expresion};

/*
** Cada función se representa como una lista de sentencias que a su
** vez pueden tener sublistas (recursivamente)
**
** Observese como se puede efectuar fácilmente optimización de las
** construcciones del tipo:
**
**    if (0)
**      xxxx;
*/
struct sentencia {
  struct sentencia *sig;       /* Siguiente sentencia en la lista */
  enum tipo_sentencia tipo;    /* Tipo de sentencia */
  union {
    struct {                     /* Esta unión es para los if */
      struct nodo *expresion;
      struct sentencia *lista1;
      struct sentencia *lista2;
    } t_if;
    struct {                      /* Esta unión sirve para while, do y switch */
      struct nodo *expresion;
      struct sentencia *lista;
      int etiqueta_break;
      int etiqueta_continue;     /* Un 0 indica que continue no es válido */
    } t_while;
    struct {                      /* Esta unión es para for */
      struct nodo *expresion1;
      struct nodo *expresion2;
      struct nodo *expresion3;
      struct sentencia *lista;
      int etiqueta_break;
      int etiqueta_continue;
    } t_for;
    struct {                 /* Al generar el codigo, se buscan todos los */
      int constante;         /* case en la sublista del switch */
      int etiqueta;
    } t_case;
    struct {                 /* Esta unión sirve para break, continue, goto */
      int etiqueta;          /* default y etiqueta de goto */
    } t_break;
    struct {                 /* Esta unión sirve para return */
      struct nodo *expresion;
      int informacion;
    } t_return;
    struct {                 /* Esta unión sirve para las expresiones */
      struct nodo *expresion;
    } t_expresion;
  } def;
};

int variables_virtuales;       /* Total de variables virtuales */
int pila_temporal;         /* Describe el espacio asignado temporalmente */
                           /* en la pila */
struct sentencia *funcion; /* La función actual esta aquí */
struct nodo *ultimo_nodo;  /* Ultimo nodo definido */
int multi;             /* Multiplicación para suma y resta con apuntadores */
int buffer_vacio;            /* Buffer recien vaciado */
char *ap_buf_retrasado;      /* Apuntador dentro del buffer retrasado */
int total_lineas;            /* Total de líneas almacenadas */
int estado_buf[MAX_LIN];     /* Estado de instrucciones retrasadas */
char *linea_inst[MAX_LIN];   /* Total máximo de lineas en buffer retrasado */
char buf_retrasado[MAX_BUFR];/* Buffer para llenar instrucciones retrasadas */

/*
** Los valores que se usan en estado_buf[]
**
**  0 = No optimizable.
**  1 = Lee gr96-gr111.
**  2 = Lee gr116-gr119.
**  3 = Lee lr0-lr127 (variable local).
**  4 = Lee lr0-lr127 (argumento función).
**  5 = Escribe gr116-gr119.
**  6 = Escribe lr0-lr127 (variable local).
**  7 = Escribe lr0-lr127 (argumento función).
**  8 = Nop.
**  9 = Reservado.
** 10 = Es un jmp.
** 11 = Etiqueta.
** 12 = Es un jmpf.
** 13 = Es un jmpt.
**
** Parte de la optimización se realiza en emite_car().
*/

int clase_argumento[MAX_ARGS]; /* 0 = Normal, 1 = Función */
int virtuales[MAX_VIRTUALES];  /* Máximo de variables virtuales */
int temporales[16];        /* Describe los 16 registros temporales */
                           /* gr96 - gr111 */
unsigned char
     linea[TAM_LINEA],  /* Buffer de análisis */
     linea_m[TAM_LINEA],/* Buffer para el preproceso */
     lits[TAM_LITS],    /* Almacenamiento de cadenas literales */
     macs[TAM_MAC],     /* Buffer de macros */
     amacs[TAM_AMAC],   /* Buffer para argumentos de macros */
     tipos[TAM_TIPOS],  /* Tabla de tipos */
     tabla[TAM_TABLA];  /* Tabla de nombres */

struct inic {
  struct nodo *raiz;
  unsigned char *var;
} nodo_inic[MAX_INIC]; /* Nodo correspondiente a la inicialización */

union {
  double valor;          /* Valor de la constante */
  unsigned
  char byte[TAM_DOUBLE]; /* Esto es dependiente del procesador destino */
} constan;

/* Tipos de operadores */

#define N_OR       1      /* OR binario */
#define N_XOR      2      /* XOR binario */
#define N_AND      3      /* AND binario */
#define N_IGUAL    4      /* Compara si es igual */
#define N_NOIGUAL  5      /* Compara si no es igual */
#define N_MAYOR    6      /* Compara si es mayor */
#define N_MAYORI   7      /* Compara si es mayor igual */
#define N_MENOR    8      /* Compara si es menor */
#define N_MENORI   9      /* Compara si es menor igual */
#define N_SMAYOR   11     /* Compara si es mayor sin signo */
#define N_SMAYORI  12     /* Compara si es mayor igual sin signo */
#define N_SMENOR   13     /* Compara si es menor sin signo */
#define N_SMENORI  14     /* Compara si es menor igual sin signo */
#define N_IGUALPF  15     /* Compara si es igual (punto flotante) */
#define N_MAYORPF  16     /* Compara si es mayor (punto flotante) */
#define N_MAYORIPF 17     /* Compara si es mayor igual (punto flotante) */
#define N_CD       18     /* Corrimiento a la derecha */
#define N_CI       19     /* Corrimiento a la izquierda */
#define N_SUMA     20     /* Suma */
#define N_RESTA    21     /* Resta */
#define N_AOR      22     /* |= */
#define N_AXOR     23     /* ^= */
#define N_AAND     24     /* &= */
#define N_ACI      25     /* <<= */
#define N_ACD      26     /* >>= */
#define N_ASUMA    27     /* += */
#define N_ARESTA   28     /* -= */
#define N_AMUL     29     /* *= */
#define N_ADIV     30     /* /= */
#define N_AMOD     31     /* %= */
#define N_ASIGNA   32     /* = */
#define N_MUL      33     /* Multiplicación */
#define N_DIV      34     /* División */
#define N_SDIV     35     /* División sin signo */
#define N_MOD      36     /* Resto */
#define N_SMOD     37     /* Resto sin signo */
#define N_NEG      38     /* Negación */
#define N_NEGPF    39     /* Negación (punto flotante) */
#define N_COM      40     /* Complemento binario */
#define N_INC      41     /* Preincremento */
#define N_PINC     42     /* Posincremento */
#define N_NOT      43     /* NOT boleano */
#define N_APFUNC   44     /* Obtiene un apuntador a una función */
#define N_CONST    45     /* Carga de una constante */
#define N_LIT      46     /* Carga la dirección de una cadena */
#define N_CBYTE    47     /* Lee un byte de la memoria */
#define N_CPAL     48     /* Lee una palabra de la memoria */
#define N_DIR      49     /* Dirección de una variable */
#define N_DIRG     50     /* Dirección de una variable global */
#define N_ANDB     51     /* AND boleano */
#define N_ORB      52     /* OR boleano */
#define N_TRI      53     /* Operador trinario ?: */
#define N_APRES    54     /* Obtiene apuntador a espacio de resultado */
#define N_COMA     55     /* Operador coma */
#define N_CSHORT   56     /* Obtiene una palabra corta indirecta */
#define N_CUSHORT  57     /* Obtiene una palabra corta sin signo indirecta */
#define N_COPIA    58     /* Copia un bloque de memoria */
#define N_CFLOAT   59     /* Lee un float de la memoria, convierte a double */
#define N_CDOUBLE  60     /* Lee un double de la memoria */
#define N_SUMAPF   61     /* Suma (punto flotante) */
#define N_RESTAPF  62     /* Resta (punto flotante) */
#define N_MULPF    63     /* Multiplicación (punto flotante) */
#define N_DIVPF    64     /* División (punto flotante) */
#define N_CEROPF   65     /* Carga cero en precisión doble */
#define N_NUMPF    66     /* Carga un número de punto flotante */
#define N_ENTPF    67     /* Convierte un entero a double */
#define N_PFENT    68     /* Convierte punto flotante a entero */
#define N_FUNC     69     /* Llamada a función */
#define N_FUNCI    70     /* Llamada a función indirecta */
#define N_PAR      71     /* Parametro de una función */
#define N_PARF     72     /* Parametro de una función (punto flotante) */
#define N_RESULTA  73     /* Reserva espacio para recibir una estructura */
#define N_RESTAI   74     /* Resta invertida */

/*
** Macros para acelerar el compilador.
*/

/*
** Caracter en la posición actual.
*/
#define car_act           linea[pos_linea]

/*
** Siguiente etiqueta interna disponible.
*/
#define nueva_etiq        (++sig_etiq)
/*
** Compilador de C para G11.
** Interfaz con el usuario.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 4 de junio de 1995.
** Revisión: 26 de julio de 1995. Ahora ap_mac se inicializa a 1.
** Revisión: 27 de julio de 1995. Se modifica p_include() para que sólo
**                                acepte la sintaxis estandard.
** Revisión: 10 de agosto de 1995. Predefine los tipos estandares.
** Revisión: 12 de agosto de 1995. Soporte para #include anidado.
** Revisión: 22 de agosto de 1995. Inicialización de lista_estruct.
** Revisión: 23 de agosto de 1995. Inicialización de ultima_estruct.
** Revisión: 24 de agosto de 1995. Inicialización de lista_enum y ultimo_enum.
** Revisión: 7 de septiembre de 1995. Nueva función. inicializa().
** Revisión: 22 de noviembre de 1995. Inicialización de const_definidas, y
**                                    inicialización de pos_globales a 3.
** Revisión: 8 de mayo de 1998. Inicialización de funcion a NULL.
*/

/*
** El compilador comienza su ejecución aquí.
*/
main()
{
  presentacion();           /* Presentacion */
  opciones();               /* Determina las opciones */
  abre_entrada();           /* Primer archivo a procesar */
  if (entrada != NULL) {
    inicializa();           /* Inicializa todo */
    abre_salida();          /* Prepara el archivo de salida */
    prologo();              /* Emite el prologo */
    analiza();              /* Hace la compilación */
    if (nivel)
      error("Falta llave de cierre");
    epilogo();              /* Emite el epilogo */
    cierra_salida();        /* Cierra la salida */
    reporta_errores();      /* Reporta errores detectados */
  }
  color(7);
}

/*
** Selecciona un color
*/
#ifndef FENIX
color(col)
  int col;
{
  putchar(0x1b);
  putchar(0x5b);
  putchar(0x30);
  putchar(0x3b);
  putchar(0x33);
  if (col >= 8) {
    putchar(0x30 + (col - 8));
    putchar(0x3b);
    putchar(0x31);
  } else
    putchar(0x30 + col);
  putchar(0x6d);
}
#endif

/*
** Inicializa todo.
*/
inicializa()
{
  ap_glb = INICIO_GLB;    /* Limpia la tabla global */
  ap_loc = INICIO_LOC;    /* Limpia la tabla local */
  ultimo_bucle = NULL;    /* Limpia la cola de bucles */
  pila =                  /* Apuntador de pila */
  errores =               /* No hay errores */
  eof =                   /* No se ha alcanzado el fin del archivo */
  desvio_salida =         /* No se ha desviado la salida */
  nivel =                 /* No hay bloques abiertos */
  comienzo_funcion =      /* La función actual empezó en la linea 0 */
  linea_actual =          /* No se han leido líneas del archivo */
  dentro_funcion =        /* No esta dentro de una función */
  sig_etiq =              /* Inicia números de etiquetas */
  nivel_if =              /* No esta dentro de un #if... */
  nivel_incl =            /* No esta dentro de un #include */
  evadir_nivel =          /* No esta evadiendo ningun texto de la entrada */
  ultimo_nodo = 0;        /* Ultimo nodo usado del arbol */
  dentro_pp = NO;         /* No esta dentro del preprocesador */
  ap_mac = 1;             /* Limpia la tabla de macros */
  pos_global = 3;         /* Reserva dos palabras para el limpiador, una */
                          /* palabra extra para apuntar a la tabla de punto */
                          /* flotante */
  sig_case = casos;       /* Ningún case aún */
  funcion_actual = NULL;  /* Ninguna función aún */
  funcion = NULL;         /* La función no ha sido compilada aún */
  lista_estruct = NULL;   /* Ninguna estructura definida */
  ultima_estruct = NULL;  /* No hay última estructura definida */
  lista_enum = NULL;      /* No hay lista de enumeradores */
  ultimo_enum = NULL;     /* No hay último enumerador definido */
  sig_tipo = tipos;       /* Ningún tipo aún */
                          /* Prepara los tipos predefinidos */
  t_achar = sig_tipo;     /* Apuntador a char */
  *sig_tipo++ = APUNTADOR;
  t_char = sig_tipo;      /* char */
  *sig_tipo++ = CHAR;
  t_short = sig_tipo;     /* short */
  *sig_tipo++ = SHORT;
  t_func = sig_tipo;      /* Función que retorna int */
  *sig_tipo++ = FUNCION;
  t_int = sig_tipo;       /* int */
  *sig_tipo++ = INT;
  t_ushort = sig_tipo;    /* unsigned short */
  *sig_tipo++ = USHORT;
  t_uint = sig_tipo;      /* unsigned int */
  *sig_tipo++ = UINT;
  t_float = sig_tipo;     /* float */
  *sig_tipo++ = FLOAT;
  t_double = sig_tipo;    /* double */
  *sig_tipo++ = DOUBLE;
  t_void = sig_tipo;      /* void */
  *sig_tipo++ = VOID;
}

/*
** Cancela la compilación.
*/
cancela()
{
  while (nivel_incl)
    fin_include();
  if (entrada != NULL)
    fclose(entrada);
  cierra_salida();
  hacia_consola();
  color(15);
  mensaje("Compilación cancelada.");
  emite_nueva_linea();
  exit(1);
}

/*
** Reporta los errores
*/
reporta_errores()
{
  emite_nueva_linea();
  color(11);
  emite_texto("Hubo ");
  emite_numero(errores);       /* No. total de errores */
  emite_texto(" errores en la compilación.");
  emite_nueva_linea();
}

/*
** Presentación.
*/
presentacion()
{
#ifndef FENIX
  color(15);
  mensaje(PROGRAMA);
  emite_nueva_linea();
#endif
}

/*
** Opciones de compilación.
*/
opciones()
{
  pausa = NO;
#ifndef FENIX
  color(10);
  mensaje("¿ Desea una pausa despues de un error (S/N) ? ");
  gets(linea);
  if ((car_act == 'S') || (car_act == 's'))
    pausa = SI;
#endif
}

/*
** Obtiene el nombre del archivo de salida.
*/
abre_salida()
{
  salida = 0;           /* Por defecto la salida a la consola */
  while (salida == 0) {
    descarta();
#ifndef FENIX
    color(10);
    mensaje("¿ Archivo de salida ? ");
    gets(linea);        /* Obtiene el nombre */
#else
    strcpy(linea, "d:/ejemplo.a");
#endif
    if (car_act == 0)
      cancela();        /* Ninguno, cancelar */
    if ((salida = fopen(linea, "w")) == NULL) {  /* Intenta crear */
      salida = 0;       /* No pudo crearse */
      error("No se pudo crear el archivo");
    }
  }
  hacia_consola();
  emite_nueva_linea();
  hacia_archivo();
  descarta();           /* Limpia la línea */
  ap_buf_retrasado = buf_retrasado;
  total_lineas = 0;
}

/*
** Obtiene el archivo de entrada
*/
abre_entrada()
{
  entrada = NULL;          /* Ninguno aún */
  while (entrada == NULL) {
    descarta();         /* Limpia la línea de entrada */
#ifndef FENIX
    color(10);
    mensaje("¿ Archivo de entrada ? ");
    gets(linea);        /* Obtiene un nombre */
#else
    strcpy(linea, "d:/ejemplo.c");
#endif
    if (car_act == 0)
      break;
    if ((entrada = fopen(linea, "r")) != NULL)
      nuevo_archivo();
    else {
      entrada = 0;      /* No se pudo leer */
      color(15);
      mensaje("No se pudo leer el archivo");
    }
  }
  descarta();           /* Limpia la línea */
}

/*
** Inicia el contador de líneas.
*/
nuevo_archivo()
{
  linea_actual = 0;     /* Ninguna línea leida */
  comienzo_funcion = 0; /* Ninguna función aún */
  funcion_actual = NULL;
  dentro_funcion = NO;
}

/*
** Procesa #include, abre el nuevo archivo.
*/
p_include()
{
  unsigned char *rastreo, *comienzo;
  int estatus;

  espacios();           /* Salta los espacios */

  hacia_consola();
  color(11);
  emite_texto("#include ");
  emite_texto(linea + pos_linea);
  emite_nueva_linea();
  hacia_archivo();

  rastreo = linea + pos_linea;
  if (*rastreo == '<') {
    ++rastreo;
    comienzo = rastreo;
    estatus = 1;
  } else if (*rastreo == '"') {
    ++rastreo;
    comienzo = rastreo;
    estatus = 2;
  } else {
    estatus = 0;
    comienzo = rastreo;
    error("Error de sintaxis");
  }
  while(*rastreo != '>' && *rastreo != '"' && *rastreo)
    ++rastreo;
  if(*rastreo == '>' && estatus == 1) *rastreo = 0;
  else if(*rastreo == '"' && estatus == 2) *rastreo = 0;
  else if(estatus != 0)
    error("Falta > o \" al final");
  if (nivel_incl == MAX_INCL)
    error("Demasiados #include");
  else if ((entrada2 = fopen(comienzo, "r")) == NULL)
    error("No se pudo leer el archivo");
  else {
    incl[nivel_incl++] = entrada;
    incl[nivel_incl++] = funcion_actual;
    incl[nivel_incl++] = comienzo_funcion;
    incl[nivel_incl++] = linea_actual;
    incl[nivel_incl++] = dentro_funcion;
    entrada = entrada2;
    nuevo_archivo();
  }
  descarta();           /* La siguiente entrada será del */
                        /* nuevo archivo. */
}

/*
** Fin de un archivo #include
*/
fin_include()
{
  hacia_consola();
  color(11);
  emite_texto("#fin include");
  emite_nueva_linea();
  hacia_archivo();

  fclose(entrada);
  dentro_funcion = incl[--nivel_incl];
  linea_actual = incl[--nivel_incl];
  comienzo_funcion = incl[--nivel_incl];
  funcion_actual = incl[--nivel_incl];
  entrada = incl[--nivel_incl];
}

/*
** Cierra el archivo de salida.
*/
cierra_salida()
{
  hacia_archivo();      /* Si esta desviado, volver al archivo */
  if (salida)
    fclose(salida);     /* Si esta abierto, cerrarlo */
  salida = 0;           /* Marcar como cerrado */
}
/*
** Compilador de C para G11.
** Análisis sintáctico de alto nivel.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 3 de junio de 1995.
** Revisión: 24 de julio de 1995. Soporte para switch, case y default.
** Revisión: 24 de julio de 1995. Soporte para variables locales anidadas.
** Revisión: 25 de julio de 1995. Soporte para goto.
** Revisión: 26 de julio de 1995. Ahora se aceptan declaraciones de
**                                matrices con expresiones constantes cómo
**                                limites.
** Revisión: 26 de julio de 1995. Ahora los case pueden usar expresiones
**                                constantes.
** Revisión: 26 de julio de 1995. Traslado #include y #define al preprocesador.
** Revisión: 26 de julio de 1995. Traslado #asm al preprocesador.
** Revisión: 10 de agosto de 1995. Soporte para tipos complejos. Se agregan
**                                 las funciones p_tipo_1, p_tipo_2, p_tipo_3,
**                                 copia_tipo, guarda_tipo, tam_tipo.
** Revisión: 12 de agosto de 1995. Se corrige la sintaxis que reconoce
**                                 p_tipo_3.
** Revisión: 12 de agosto de 1995. Soporte para static, register y auto.
** Revisión: 12 de agosto de 1995. Soporte para typedef.
** Revisión: 12 de agosto de 1995. Corrección de un defecto en tipos_args().
** Revisión: 22 de agosto de 1995. Soporte para struct y union.
** Revisión: 22 de agosto de 1995. Nueva función. p_estructura().
** Revisión: 24 de agosto de 1995. Soporte para enum.
** Revisión: 5 de septiembre de 1995. Soporte para paso de estructuras cómo
**                                    parametros y resultados.
** Revisión: 5 de septiembre de 1995. Nueva función. ordena_args().
** Revisión: 6 de septiembre de 1995. Soporte para inicialización de
**                                    variables automáticas, nueva función.
**                                    inic_loc().
** Revisión: 6 de septiembre de 1995. Corrección de un defecto en la
**                                    compilación de la sentencia for.
** Revisión: 12 de septiembre de 1995. Soporte para float y double.
** Revisión: 23 de septiembre de 1995. Corrección de un defecto en s_return().
** Revisión: 27 de septiembre de 1995. Corrección de una llamada mal hecha a
**                                     checa_entero en s_switch, corrección
**                                     de un problema de tipos en p_estructura.
** Revisión: 24 de noviembre de 1995. Corrección de varios defectos en
**                                    tipos_args().
** Revisión: 24 de noviembre de 1995. optimización de la salida de s_if().
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en el análisis
**                                    de la sintaxis de do-while.
** Revisión: 28 de diciembre de 1995. Ahora soporta declaraciones dentro de un
**                                    switch. (siempre que estén dentro de un
**                                    bloque)
** Revisión: 28 de diciembre de 1995. Corrección de errores en tam_tipo().
** Revisión: 25 de enero de 1996. Corrección de un defecto en la inicialización
**                                de variables, tronaba el sistema.
** Revisión: 3 de febrero de 1996. Aprovecha las hasta 2 palabras libres que
**                                 ocurren cuando se llama una función.
** Revisión: 30 de marzo de 1996. Corrección de un defecto tremendo en los
**                                switch cuando salta a default.
** Revisión: 9 de abril de 1996. Simplificación de ordena_args(), corrección
**                               de defectos.
** Revisión: 19 de abril de 1996. Corrección de un error en la declaración
**                                de funciones, no convertia float a double en
**                                algunos casos.
** Revisión: 20 de junio de 1996. Detección de variables locales redefinidas.
** Revisión: 12 de octubre de 1996. Optimiza estructuras que usan matrices
**                                  de caracteres.
** Revisión: 7 de mayo de 1998. Soporte para árboles dinámicos de expresiones.
** Revisión: 8 de mayo de 1998. Revisión total del análisis sintáctico para
**                              generar la representación intermedia de las
**                              funciones.
** Revisión: 8 de mayo de 1998. Se elimina la función inic_loc().
*/

/*
** Procesa todo el texto de entrada.
**
** En este nivel, sólo declaraciones estáticas,
** #define, #include, y definiciones de función
** son legales.
*/
analiza()
{
  while (eof == 0) {    /* Trabaja hasta que no haya más entrada */
    if (amatch("typedef", 7))
      decl_typedef(NO);
    else {
      if (amatch("static", 6))
        decl_glb(STATIC);
      else if (amatch("extern", 6))
        decl_glb(EXTERN);
      else {
        if (amatch("register", 8)
           || amatch("auto", 4))
          error("Solo se acepta static o extern");
        decl_glb(AUTO);
      }
    }
    espacios();         /* Rastrea fin de archivo */
  }
}

/*
** Declara un tipo.
*/
decl_typedef(local)
  int local;
{
  unsigned char nombre[TAM_NOMBRE];
  unsigned char *chequeo;

  p_tipo_1(SI);
  while (1) {
    if (fin_sentencia())
      break;
    if (p_tipo_2(nombre))
      pide(")");
    if (*tipo_proc == FUNCION)
      error("No se puede definir un tipo de función");
    if (local)
      chequeo = busca_loc(nombre);
    else
      chequeo = busca_glb(nombre);
    if (chequeo)
      redefinido(nombre);
    if (local)
      nueva_loc(nombre, TYPEDEF, AUTO, tipo_proc, 0);
    else
      nueva_glb(nombre, TYPEDEF, STATIC, tipo_proc, 0);
    if (match(",") == 0)
      break;
  }
  punto_y_coma();
}

/*
** Declara una variable global.
**
** Crea una entrada en la tabla, para que las
** referencias subsiguientes la llamen por nombre.
*/
decl_glb(clase)
  int clase;
{
  int p;
  unsigned char nombre[TAM_NOMBRE];
  unsigned char *chequeo;

  p_tipo_1(SI);
  while (1) {
    if (fin_sentencia())
      break;
    p = p_tipo_2(nombre);
    if (*tipo_proc == FUNCION) {
      espacios();
      if (*(tipo_proc + 1) == FLOAT)
        *(tipo_proc + 1) = DOUBLE;
      if (p == 0 && (car_act == ';' || car_act == ',')) {
        if (chequeo = busca_glb(nombre)) {
          if (chequeo[IDENT] != FUNCION)
            redefinido(nombre);
          else if (chequeo[POSICION] != FUNC_REF)
            redefinido(nombre);
          else {
            chequeo[POSICION] = FUNC_TIPO;
            escribe_entero(chequeo + TIPO, tipo_proc);
          }
        } else
          nueva_glb(nombre, FUNCION, STATIC, tipo_proc, FUNC_TIPO);
      } else {
        if (clase != STATIC)
          def_global(nombre);
        nueva_func(nombre, p);
        return;
      }
    } else {
      if (busca_glb(nombre)) {  /* ¿ Ya estaba en la tabla ? */
        if (clase != EXTERN)
          redefinido(nombre);
      } else {                  /* Agrega la nueva variable */
        if (clase == STATIC)
          nueva_glb(nombre, VARIABLE, STATIC, tipo_proc, p = nueva_etiq);
        else
          nueva_glb(nombre, VARIABLE, STATIC, tipo_proc, p = 0);
        if (clase != EXTERN) {
          if (p)
            emite_etiq(p);
          else {
            def_global(nombre);
            emite_nombre(nombre);
          }
          dos_puntos();
          emite_nueva_linea();
          def_espacio((tam_tipo(tipo_proc) + 3) & ~3);
        }
      }
    }
    if (match(",") == 0)
      break;
  }
  punto_y_coma();
}

/*
** Procesa declaraciones de variables locales.
*/
decl_loc()
{
  unsigned char nombre[TAM_NOMBRE];
  unsigned char *salva_tipo_basico, *salva_tipo_proc;
  unsigned char *tipo_expr, *chequeo, *ap;
  int p, sin_int, estaticas;
  struct nodo *nodo_expr;

  while (1) {
    sin_int = estaticas = NO;
    if (amatch("register", 8)
    || amatch("auto", 4)) sin_int = SI;
    else if (amatch("static", 6))
      sin_int = estaticas = SI;
    if (p_tipo_1(sin_int) == 0)
      break;
    if (dentro_switch)
      error("No se pueden hacer declaraciones dentro de un switch");
    while (1) {
      if (fin_sentencia())
        break;
      p = p_tipo_2(nombre);
      if ((chequeo = busca_loc(nombre)) && nivel == chequeo[NIVEL])
        redefinido(nombre);
      if (*tipo_proc == FUNCION) {
        if (p)
          pide(")");
        nueva_loc(nombre, FUNCION, AUTO, tipo_proc, FUNC_TIPO);
      } else if (estaticas) {
        nueva_loc(nombre, VARIABLE, STATIC, tipo_proc, pos_global);
        pos_global += (tam_tipo(tipo_proc) + 3) / 4;
      } else {
        /*
        ** Las estructuras y las matrices jamás caben en un registro.
        **
        ** Las variables normales las dejamos al libre albedrío del
        ** compilador.
        */
        if (*tipo_proc != STRUCT && *tipo_proc != MATRIZ) {
          ap = nueva_loc(nombre, VARIABLE, AUTO, tipo_proc,
                         var_virtual(0, *tipo_proc == DOUBLE));
        } else
          ap = nueva_loc(nombre, VARIABLE, AUTO, tipo_proc,
                 var_virtual((((tam_tipo(tipo_proc) + 3) & ~3) << 2) | 1, 0));
        if (*tipo_proc != STRUCT && *tipo_proc != MATRIZ && match("=")) {
          salva_tipo_basico = tipo_basico;
          salva_tipo_proc = tipo_proc;
          tipo_expr = almacena_expresion(NO);
          nodo_expr = ultimo_nodo;
          tipo_proc = salva_tipo_proc;
          tipo_basico = salva_tipo_basico;
          convierte_tipo(&nodo_expr, tipo_expr, tipo_proc);
          if (vars_inicializadas == MAX_INIC) {
            error("Demasiadas variables inicializadas");
          } else {
            nodo_inic[vars_inicializadas].raiz = nodo_expr;
            nodo_inic[vars_inicializadas].var = ap;
            vars_inicializadas++;
          }
        }
      }
      if (match(",") == 0)
        break;
    }
    punto_y_coma();
  }
}

/*
** Procesa la primera parte de un tipo, la variable tipo_basico
** contiene el tipo base del tipo completo.
*/
p_tipo_1(sin_int)
  int sin_int;
{
  int salva_posicion;
  unsigned char nombre[TAM_NOMBRE];
  unsigned char *chequeo;

  if (amatch("char", 4)) {
    tipo_basico = t_char;
    return 1;
  }
  if (amatch("int", 3)) {
    tipo_basico = t_int;
    return 1;
  }
  if (amatch("long", 4)) {
    if (amatch("int", 3)) ;
    else if (amatch("float", 5)) {
      tipo_basico = t_double;
      return 1;
    }
    tipo_basico = t_int;
    return 1;
  }
  if (amatch("short", 5)) {
    if (amatch("int", 3)) ;
    tipo_basico = t_short;
    return 1;
  }
  if (amatch("void", 4)) {
    tipo_basico = t_void;
    return 1;
  }
  if (amatch("float", 5)) {
    tipo_basico = t_float;
    return 1;
  }
  if (amatch("double", 6)) {
    tipo_basico = t_double;
    return 1;
  }
  if (amatch("struct", 6)) {
    p_estructura(NO);
    return 1;
  }
  if (amatch("union", 5)) {
    p_estructura(SI);
    return 1;
  }
  if (amatch("enum", 4)) {
    p_enumerador();
    return 1;
  }
  if (amatch("unsigned", 8)) {
    if (amatch("char", 4)) {
      tipo_basico = t_char;
      return 1;
    }
    if (amatch("short", 5)) {
      if (amatch("int", 3)) ;
      tipo_basico = t_ushort;
      return 1;
    }
    if (amatch("int", 3)) ;
    else if (amatch("long", 4)) {
      if (amatch("int", 3)) ;
    }
    tipo_basico = t_uint;
    return 1;
  }
  espacios();
  salva_posicion = pos_linea;
  if (nombre_legal(nombre)) {
    if ((chequeo = busca_loc(nombre))
    || (chequeo = busca_glb(nombre))) {
      if (chequeo[IDENT] == TYPEDEF) {
        tipo_basico = lee_entero(chequeo + TIPO);
        return 1;
      }
    }
    pos_linea = salva_posicion;
  }
  if (sin_int) {
    tipo_basico = t_int;
    return 1;
  }
  return 0;
}

/*
** Genera un tipo procesado.
*/
p_tipo_2(nombre)
  unsigned char *nombre;
{
  int p;
  int decoracion;
  tipo_proc = sig_tipo;
  decoracion = 0;
  p = p_tipo_3(nombre, &decoracion, 0);
  if (decoracion == 0) {
    tipo_proc = tipo_basico;
    return p;
  }
  copia_tipo(tipo_basico);
  return p;
}

/*
** Copia un tipo en la siguiente posición disponible.
*/
copia_tipo(tipo)
  unsigned char *tipo;
{
  while (*tipo >= APUNTADOR) {
    if (*tipo == MATRIZ) {
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
      guarda_tipo(*tipo++);
    } else
      guarda_tipo(*tipo++);
  }
  if (*tipo == STRUCT) {
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
    guarda_tipo(*tipo++);
  }
  guarda_tipo(*tipo++);
}

/*
** Almacena un byte de tipo.
*/
guarda_tipo(byte)
  int byte;
{
  if (sig_tipo >= MAX_TIPOS) {
    error("Tabla de tipos llena");
    cancela();
  }
  *sig_tipo++ = byte;
}

/*
** Procesa las decoraciones de tipo.
*/
p_tipo_3(nombre, decoracion, anidamiento)
  unsigned char *nombre;
  int *decoracion, anidamiento;
{
  int p, tam;

  if (match("*")) {
    p = p_tipo_3(nombre, decoracion, anidamiento);
    *decoracion = 1;
    guarda_tipo(APUNTADOR);
    return p;
  }
  if (match("(")) {
    if (nombre == NULL && match(")")) {
      *decoracion = 1;
      guarda_tipo(FUNCION);
      return 0;
    }
    p = p_tipo_3(nombre, decoracion, 1);
    pide(")");
  } else if (nombre != NULL) {
    if (nombre_legal(nombre) == 0)
      nombre_ilegal();
  }
  if (match("(")) {
    *decoracion = 1;
    p = 0;
    if (anidamiento == 0 && nombre != NULL) {
      if (match(")") == 0)
        p = 1;
    } else
      pide(")");
    guarda_tipo(FUNCION);
    return p;
  }
  while (match("[")) {
    *decoracion = 1;
    tam = subindice();
    guarda_tipo(MATRIZ);
    guarda_tipo(tam);
    guarda_tipo(tam >> 8);
    guarda_tipo(tam >> 16);
    guarda_tipo(tam >> 24);
  }
  return 0;
}

/*
** Procesa una declaración de estructura o unión.
*/
p_estructura(es_union)
  int es_union;
{
  unsigned char rotulo[TAM_NOMBRE];
  unsigned char nombre_miembro[TAM_NOMBRE];
  unsigned char *miembro;
  unsigned char *estructura;
  unsigned char *lista;
  int posicion, tam, numero_bits;
  unsigned char *tipo_optimo;

  if (nombre_legal(rotulo)) {
    if ((estructura = busca_estructura(rotulo)) != NULL) {
      if (estructura[EST_QUE_ES] == ENUM)
        redefinido(rotulo);
      if (es_union != estructura[EST_ES_UNION])
        if (es_union)
          error("Se uso union en lugar de struct");
        else
          error("Se uso struct en lugar de union");
      if (lee_entero(estructura + EST_TAM)) {
        tipo_basico = sig_tipo;
        guarda_tipo(STRUCT);
        posicion = estructura;
        guarda_tipo(posicion);
        guarda_tipo(posicion >> 8);
        guarda_tipo(posicion >> 16);
        guarda_tipo(posicion >> 24);
        return;
      }
    } else {
      estructura = nueva_estructura(rotulo);
      estructura[EST_QUE_ES] = STRUCT;
      estructura[EST_ES_UNION] = es_union;
    }
  } else {
    estructura = nueva_estructura("");
    estructura[EST_QUE_ES] = STRUCT;
    estructura[EST_ES_UNION] = es_union;
  }
  if (match("{") == 0) {
    tipo_basico = sig_tipo;
    guarda_tipo(STRUCT);
    posicion = estructura;
    guarda_tipo(posicion);
    guarda_tipo(posicion >> 8);
    guarda_tipo(posicion >> 16);
    guarda_tipo(posicion >> 24);
    return;
  }
  lista = NULL;
  posicion = tam = 0;
  while (p_tipo_1(NO)) {
    while (1) {
      if (es_union)
        posicion = 0;
      if (fin_sentencia())
        break;
      numero_bits = 0;
      if (match(":")) {
        *nombre_miembro = 0;
        numero_bits = expr_constante();
        if (tipo_basico != t_int && tipo_basico != t_uint)
          error("No es de tipo int o unsigned int");
        if (numero_bits > 32)
          error("Más de 32 bits en el miembro");
        if (numero_bits == 0) {
          posicion = ((posicion + 3) & ~3) + 4;
        } else if (numero_bits > 16) {
          posicion += 4;
        } else if (numero_bits > 8) {
          posicion += 2;
        } else {
          posicion++;
        }
      } else {
        if (p_tipo_2(nombre_miembro))
          pide(")");
        if (match(":")) {
          numero_bits = expr_constante();
          if (tipo_proc != t_int && tipo_proc != t_uint)
            error("No es de tipo int o unsigned int");
          if (numero_bits == 0)
            error("Miembro vacio");
          if (numero_bits > 32)
            error("Más de 32 bits en el miembro");
          if (numero_bits > 16) {
            posicion = (posicion + 3) & ~3;
          } else if (numero_bits > 8) {
            posicion = (posicion + 1) & ~1;
            if (tipo_proc == t_int)
              tipo_proc = t_short;
            else
              tipo_proc = t_ushort;
          } else {
            tipo_proc = t_char;
          }
          numero_bits = 0;
        } else {
          tipo_optimo = tipo_proc;
          while (*tipo_optimo == MATRIZ)
            tipo_optimo += 5;
          if (*tipo_optimo == *t_short || *tipo_optimo == *t_ushort)
            posicion = (posicion + 1) & ~1;
          else if (*tipo_optimo != *t_char)
            posicion = (posicion + 3) & ~3;
        }
      }
      if (*nombre_miembro) {
        if (miembro = busca_miembro(lista, nombre_miembro))
          redefinido(nombre_miembro);
        else
          miembro = nuevo_miembro(&lista, nombre_miembro);
        escribe_entero(miembro + MIE_TIPO, tipo_proc);
        escribe_entero(miembro + MIE_POSICION, posicion);
        posicion += tam_tipo(tipo_proc);
      }
      if (es_union)
        tam = (posicion > tam) ? posicion : tam;
      else
        tam = posicion;
      if (match(",") == 0) break;
    }
    punto_y_coma();
  }
  if (lista != NULL)
    escribe_entero(estructura + EST_LISTA, lista);
  escribe_entero(estructura + EST_TAM, tam);
  if (tam == 0)
    if (es_union) error("Unión vacia");
    else error("Estructura vacia");
  tipo_basico = sig_tipo;
  guarda_tipo(STRUCT);
  posicion = estructura;
  guarda_tipo(posicion);
  guarda_tipo(posicion >> 8);
  guarda_tipo(posicion >> 16);
  guarda_tipo(posicion >> 24);
  pide("}");
}

/*
** Procesa un enumerador.
*/
p_enumerador()
{
  unsigned char rotulo[TAM_NOMBRE];
  unsigned char nombre_miembro[TAM_NOMBRE];
  unsigned char *enumerador;
  int valor;

  if (nombre_legal(rotulo)) {
    if ((enumerador = busca_estructura(rotulo)) != NULL) {
      if (enumerador[EST_QUE_ES] != ENUM)
        redefinido(rotulo);
      if (lee_entero(enumerador + EST_TAM)) {
        tipo_basico = t_int;
        return;
      }
    } else {
      enumerador = nueva_estructura(rotulo);
      enumerador[EST_QUE_ES] = ENUM;
    }
  } else {
    enumerador = nueva_estructura("");
    enumerador[EST_QUE_ES] = ENUM;
  }
  if (match("{") == 0) {
    tipo_basico = t_int;
    return;
  }
  escribe_entero(enumerador + EST_TAM, 1);
  valor = 0;
  while (1) {
    if (nombre_legal(nombre_miembro) == 0)
      break;
    if (match("="))
      valor = expr_constante();
    if (busca_enum(nombre_miembro) != NULL)
      redefinido(nombre_miembro);
    else
      nuevo_enum(nombre_miembro, valor++);
    if (match(",") == 0)
      break;
  }
  pide("}");
  tipo_basico = t_int;
}

/*
** Obtiene el tamaño de un tipo.
*/
tam_tipo(tipo)
  unsigned char *tipo;
{
  int tam;
  switch(*tipo) {
    case CHAR:   return 1;
    case USHORT:
    case SHORT:  return 2;
    case APUNTADOR:
    case UINT:
    case INT:
    case FLOAT:  return 4;
    case DOUBLE: return 8;
    case VOID:   error("Uso incorrecto de void");
    case FUNCION:
                 error("Uso incorrecto de tipo de función");
                 return 0;
    case MATRIZ: tam = lee_entero(tipo + 1);
                 if (tam == 0)
                   error("Tamaño nulo de matriz");
                 return tam_tipo(tipo + 5) * tam;
    case STRUCT: tam = lee_entero(lee_entero(tipo + 1) + EST_TAM);
                 if (tam == 0)
                   error("Estructura o unión incompleta");
                 return (tam + 3) & ~3;
  }
}

/*
** Obtiene el tamaño de una matriz.
**
** Invocada cuando una declaración es seguida
** por "[".
*/
subindice()
{
  int num;

  if (match("]"))
    return 0;                   /* Tamaño nulo */
  num = expr_constante();       /* Procesa una expresión constante */
  if (num == 0) {
    error("No se acepta una dimensión cero");
    num = 1;                    /* Forza a 1 */
  }
  if (num < 0) {
    error("Tamaño negativo");
    num = -num;
  }
  pide("]");                    /* Forza una dimensión */
  return num;                   /* y retorna el tamaño */
}

struct sentencia *sentencia();
struct sentencia *p_bloque();
struct sentencia *s_if();
struct sentencia *s_while();
struct sentencia *s_do();
struct sentencia *s_for();
struct sentencia *s_switch();
struct sentencia *s_case();
struct sentencia *s_default();
struct sentencia *s_goto();
struct sentencia *s_return();
struct sentencia *s_break();
struct sentencia *s_continue();
struct sentencia *p_etiqueta();

/*
** Compila una función.
**
** Invocada por "decl_glb", compila una función a partir de la entrada.
*/
nueva_func(n_func, parentesis)
  unsigned char *n_func;
  int parentesis;
{
  unsigned char n[TAM_NOMBRE];
  int num_args;
                                   /* Recuerda el comienzo de la función */
  comienzo_funcion = linea_actual;
  dentro_funcion = SI;          /* Indica que esta dentro de una función */
                                /* ¿ Ya estaba en la tabla de nombres ? */
  if (funcion_actual = busca_glb(n_func)) {
    if (funcion_actual[IDENT] != FUNCION)
      redefinido(n_func);       /* Ya hay una variable con ese nombre */
    else if (funcion_actual[POSICION] == FUNC_DEF)
      redefinido(n_func);       /* Se redefinio una función. */
    else {                      /* Es una función referenciada antes */
      funcion_actual[POSICION] = FUNC_DEF;
      escribe_entero(funcion_actual + TIPO, tipo_proc);
    }
  }

  /* No estaba en la tabla, definir cómo una función */

  else
    funcion_actual = nueva_glb(n_func, FUNCION, STATIC, tipo_proc, FUNC_DEF);

  hacia_consola();
  emite_texto("Compilando ");
  emite_texto(n_func);
  emite_texto("()...");
  emite_nueva_linea();
  hacia_archivo();

  total_regs = -1;        /* Registros requeridos para llamar otra función */
  variables_virtuales = 0;      /* Prepara la lista de variables virtuales */
  ap_loc = INICIO_LOC;          /* Limpia la tabla de variables locales */
  pila_args = 0;                /* Inicia la cuenta de argumentos */
  while (parentesis
    && (match(")") == 0)) {     /* Empieza a contar */

    /* Cualquier nombre legal incrementa la cuenta */

    if (nombre_legal(n)) {
      if (busca_loc(n))
        redefinido(n);
      else {
        nueva_loc(n, VARIABLE, AUTO, 0, 0);
        ++pila_args;
      }
    } else {
      error("Nombre ilegal para el argumento");
      basura();
    }
    espacios();

    /* Si no es parentesis de cierre, debe ser coma */

    if (car_act != ')') {
      if (match(",") == 0)
        error("Se requiere una coma");
    }
    if (fin_sentencia())
      break;
  }

  num_args = pila_args;

  while (pila_args > 0) {

    /* Ahora el usuario declara los tipos de los argumentos */

    if (p_tipo_1(NO)) {
      tipos_args();
      punto_y_coma();
    } else {
      error("Número incorrecto de argumentos");
      num_args = 0;
      break;
    }
  }

  ordena_args(num_args);

  etiq_lit = nueva_etiq;    /* Etiqueta para el buffer literal */
  ap_lit = 0;               /* Limpia el buffer literal */

  /* Procesa el bloque de sentencias */

  funcion = sentencia();

  vacia_lits();            /* Vaciamos los literales, lo hacemos antes de */
                           /* generar el codigo, así podemos saber donde */
                           /* comienza la función */

  gen_funcion(n_func, funcion); /* Generamos el codigo definitivo */
  libera_sentencias(funcion);   /* Liberamos las listas generadas */

  ap_loc = INICIO_LOC;     /* Elimina todas las variables locales */
  dentro_funcion = NO;     /* Ahora no esta dentro de una función */
}

/*
** Declara los tipos de los argumentos.
*/
tipos_args()
{
  unsigned char n[TAM_NOMBRE], *ap_arg;
  unsigned char *nuevo_tipo;
  int p;

  while (1) {
    p = p_tipo_2(n);
    if (*tipo_proc == FUNCION) {
      if (p) pide(")");
      error("No se puede usar una función cómo argumento");
    }
    if (*tipo_proc == MATRIZ) {
      nuevo_tipo = sig_tipo;
      guarda_tipo(APUNTADOR);
      copia_tipo(tipo_proc + 5);
      tipo_proc = nuevo_tipo;
    }
    if (*tipo_proc == FLOAT)
      tipo_proc = t_double;
    if (ap_arg = busca_loc(n)) {

       /* Pone el tipo correcto al argumento */

      if (lee_entero(ap_arg + TIPO))
        error("Argumento redefinido");
      escribe_entero(ap_arg + TIPO, tipo_proc);
    } else
      error("Se requiere el nombre de un argumento");
    --pila_args;                   /* cuenta hacia atras */
    if (fin_sentencia())
      return;
    if (match(",") == 0)
      error("Se requiere una coma");
  }
}

/*
** Ordena los argumentos.
*/
ordena_args(cuantos)
  int cuantos;
{
  unsigned char *ap_arg, *tipo, *tipo_func;
  int pos, pos2;   /* pos es para registros locales y pos2 para memoria */

  tipo_func = lee_entero(funcion_actual + TIPO);
  if (*++tipo_func != STRUCT)   /* El primer byte es FUNCION */
    pos2 = 0;                   /* Función común */
  else
    pos2 = (tam_tipo(tipo_func) + 3) & ~3;
  pos = 2;     /* Empezamos desde lr2 */
  ap_arg = INICIO_LOC;
  while (cuantos--) {
    tipo = lee_entero(ap_arg + TIPO);
    if (*tipo == DOUBLE)    /* Alineamos en registro par */
      pos = (pos + 1) & ~1;
    if (*tipo != STRUCT)
      escribe_entero(ap_arg + POSICION,
                     var_virtual((pos << 2) | 2, *tipo == DOUBLE));
    else
      escribe_entero(ap_arg + POSICION, var_virtual((pos2 << 2) | 3, 0));
    if (*tipo == STRUCT)
      pos2 += (tam_tipo(tipo) + 3) & ~3;
    else if (*tipo == DOUBLE)
      pos += 2;
    else
      pos++;
    ap_arg += TAM_SIM;
  }
  pila_args = (pos + 1) & ~1;
}

/*
** Analizador de sentencias.
**
** Llamado cuando la sintaxis requiere una
** sentencia, retorna un número que indica
** la última sentencia procesada.
*/
struct sentencia *sentencia()
{
  struct sentencia *temp;

  if ((car_act == 0) && (eof))
    return NULL;
  if (match("{"))
    return p_bloque();
  if (amatch("if", 2))
    return s_if();
  if (amatch("while", 5))
    return s_while();
  if (amatch("do", 5))
    return s_do();
  if (amatch("for", 3))
    return s_for();
  if (amatch("switch", 6))
    return s_switch();
  if (amatch("case", 4))
    return s_case();
  if (amatch("default", 7))
    return s_default();
  if (amatch("goto", 4))
    return s_goto();
  if ((temp = p_etiqueta()) != NULL)
    return temp;
  if (amatch("return", 6))
    return s_return();
  if (amatch("break", 5))
    return s_break();
  if (amatch("continue", 8))
    return s_continue();
  if (match(";"))
    return NULL;
  almacena_expresion(SI);   /* Asume que es una expresión */
  temp = nueva_sentencia(t_expresion);
  temp->def.t_expresion.expresion = ultimo_nodo;
  punto_y_coma();
  return temp;
}

/*
** Checa punto y coma.
*/
punto_y_coma()
{
  if (match(";") == 0)
    error("Falta punto y coma");
}

/*
** Bloque de sentencias.
*/
struct sentencia *p_bloque()
{
  int c_dentro_switch, cuenta = 0;
  struct nodo *nodo_vars, *nodo_var;
  unsigned char *local, *local2, *local3;
  unsigned char *pos_tipo, *tipo;
  struct sentencia *lista = NULL, *temp, *agregado;

  c_dentro_switch = dentro_switch;
  if (nivel - dentro_switch >= 1)
    dentro_switch = 0;          /* Vuelve a permitir declaraciones */
  pos_tipo = sig_tipo;          /* Tabla de tipos */
  local = ap_loc;               /* Variables locales */
  ++nivel;                      /* Un nuevo nivel */
  nodo_vars = ultimo_nodo;
  vars_inicializadas = 0;
  decl_loc();                   /* Procesa declaraciones locales */
  cuenta = 0;
  while (cuenta < vars_inicializadas) {
    agregado = nueva_sentencia(t_expresion);
    tipo = lee_entero(nodo_inic[cuenta].var + TIPO);
    nodo_var = crea_nodo(N_DIR, NULL, NULL,
                     lee_entero(nodo_inic[cuenta].var + POSICION));
    agregado->def.t_expresion.expresion =
         crea_nodo(N_ASIGNA, nodo_inic[cuenta].raiz, nodo_var, *tipo);
    if (lista == NULL)
      lista = agregado;
    else
      temp->sig = agregado;
    temp = agregado;
    cuenta++;
  }
  ultimo_nodo = nodo_vars;
  while (match("}") == 0) {
    if (lista == NULL) {
      lista = sentencia();      /* Procesa sentencias */
      temp = lista;
    } else {
      temp->sig = sentencia();
      if (temp->sig != NULL)
        temp = temp->sig;
    }
  }
  --nivel;                      /* Cierra el nivel */
  local2 = ap_loc;              /* Checa el número de vars. locales */
  if (nivel) {                   /* Mantiene las etiquetas para goto */
    ap_loc = local;
    while (local < local2) {
      local3 = local + TAM_SIM;
      if (local[IDENT] == ETIQUETA) {
        while (local < local3)
          *ap_loc++ = *local++;
      } else
        local = local3;
    }
  } else
    ap_loc = local;             /* Limpia las variables locales */
  sig_tipo = pos_tipo;
  dentro_switch = c_dentro_switch;
  return lista;
}

/*
** Sentencia "if"
*/
struct sentencia *s_if()
{
  struct sentencia *temp;

  temp = nueva_sentencia(t_if);
  pide("(");
  compara_no_cero(almacena_expresion(SI));
  pide(")");
  temp->def.t_if.expresion = ultimo_nodo;
  temp->def.t_if.lista1 = sentencia();
  if (amatch("else", 4) == 0) {
    temp->def.t_if.lista2 = NULL;
    return temp;
  }
  temp->def.t_if.lista2 = sentencia();
  return temp;
}

/*
** Sentencia "while"
*/
struct sentencia *s_while()
{
  int bucle[4];                    /* Crea una entrada */
  int *anterior;
  struct sentencia *temp;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);              /* Agrega a la cola (para el break) */
  temp = nueva_sentencia(t_while);
  temp->def.t_while.etiqueta_continue = bucle[B_BUCLE];
  temp->def.t_while.etiqueta_break = bucle[B_FIN];
  pide("(");
  compara_no_cero(almacena_expresion(SI));
  pide(")");
  temp->def.t_while.expresion = ultimo_nodo;
  temp->def.t_while.lista = sentencia();
  ultimo_bucle = anterior;         /* Borra de la cola */
  return temp;
}

/*
** Sentencia "do"
*/
struct sentencia *s_do()
{
  int bucle[4];                    /* Crea una entrada */
  int *anterior;
  struct sentencia *temp;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);              /* Agrega a la cola (para el break) */
  temp = nueva_sentencia(t_do);
  temp->def.t_while.etiqueta_continue = bucle[B_BUCLE];
  temp->def.t_while.etiqueta_break = bucle[B_FIN];
  temp->def.t_while.lista = sentencia();
  if (amatch("while", 5) == 0)
    error("Falta el while");
  pide("(");
  compara_no_cero(almacena_expresion(SI));
  pide(")");
  temp->def.t_while.expresion = ultimo_nodo;
  punto_y_coma();
  ultimo_bucle = anterior;         /* Borra de la cola */
  return temp;
}

/*
** Sentencia "for"
*/
struct sentencia *s_for()
{
  int bucle[4];
  int *anterior;
  struct sentencia *temp;

  anterior = ultimo_bucle;
  nuevo_bucle(bucle);
  temp = nueva_sentencia(t_for);
  pide("(");
  if (match(";") == 0) {
    almacena_expresion(SI);
    temp->def.t_for.expresion1 = ultimo_nodo;
    punto_y_coma();
  } else
    temp->def.t_for.expresion1 = NULL;
  if (match(";") == 0) {
    compara_no_cero(almacena_expresion(SI));
    temp->def.t_for.expresion2 = ultimo_nodo;
    punto_y_coma();
  } else
    temp->def.t_for.expresion2 = NULL;
  if (match(")") == 0) {
    almacena_expresion(SI);
    temp->def.t_for.expresion3 = ultimo_nodo;
    pide(")");
  } else
    temp->def.t_for.expresion3 = NULL;
  temp->def.t_for.lista = sentencia();
  temp->def.t_for.etiqueta_continue = bucle[B_BUCLE];
  temp->def.t_for.etiqueta_break = bucle[B_FIN];
  ultimo_bucle = anterior;
  return temp;
}

/*
** Sentencia "switch"
*/
struct sentencia *s_switch()
{
  int bucle[4];
  int *anterior;
  int c_dentro_switch;
  int c_etiqueta_default;
  struct sentencia *temp;

  anterior = ultimo_bucle;
  c_dentro_switch = dentro_switch;
  c_etiqueta_default = etiqueta_default;
  etiqueta_default = 0;
  dentro_switch = nivel;
  nuevo_bucle(bucle);
  bucle[B_BUCLE] = 0;
  temp = nueva_sentencia(t_switch);
  temp->def.t_while.etiqueta_continue = bucle[B_BUCLE];
  temp->def.t_while.etiqueta_break = bucle[B_FIN];
  pide("(");
  checa_entero(almacena_expresion(SI));
  pide(")");
  temp->def.t_while.expresion = ultimo_nodo;
  temp->def.t_while.lista = sentencia();
  dentro_switch = c_dentro_switch;
  etiqueta_default = c_etiqueta_default;
  ultimo_bucle = anterior;
  return temp;
}

/*
** Sentencia "case"
*/
struct sentencia *s_case()
{
  int num;
  struct sentencia *temp;

  if (!dentro_switch)
    error("El case no esta en un switch");
  num = expr_constante();       /* Busca el número */
  temp = nueva_sentencia(t_case);
  temp->def.t_case.constante = num;
  temp->def.t_case.etiqueta = nueva_etiq;
  pide(":");
  return temp;
}

/*
** Sentencia "default"
*/
struct sentencia *s_default()
{
  struct sentencia *temp;

  if (!dentro_switch)
    error("El default no esta en un switch");
  else if (etiqueta_default)
    error("El default esta repetido");
  pide(":");
  temp = nueva_sentencia(t_default);
  temp->def.t_break.etiqueta = etiqueta_default = nueva_etiq;
  return temp;
}

/*
** Sentencia "goto"
*/
struct sentencia *s_goto()
{
  unsigned char n[TAM_NOMBRE];
  struct sentencia *temp;

  temp = nueva_sentencia(t_goto);
  if (nombre_legal(n)) {
    temp->def.t_break.etiqueta = agrega_etiqueta(n);
  } else
    error("Etiqueta incorrecta");
  punto_y_coma();
  return temp;
}

/*
** Procesa una posible definición de etiqueta para goto
*/
struct sentencia *p_etiqueta()
{
  unsigned char *c_pos_linea;
  unsigned char n[TAM_NOMBRE];
  struct sentencia *temp;

  espacios();
  c_pos_linea = pos_linea;
  if (nombre_legal(n)) {
    if (car_act == ':') {
      pos_linea++;
      temp = nueva_sentencia(t_etiqueta);
      temp->def.t_break.etiqueta = agrega_etiqueta(n);
      return temp;
    }
    else pos_linea = c_pos_linea;
  }
  return NULL;
}

int agrega_etiqueta(nombre)
  unsigned char *nombre;
{
  unsigned char *ap;

  if (ap = busca_loc(nombre)) {
    if (ap[IDENT] != ETIQUETA)
      error("No es una etiqueta");
  } else
    ap = nueva_loc(nombre, ETIQUETA, AUTO, 0, nueva_etiq);
  return lee_entero(ap + POSICION);
}

/*
** Sentencia "return"
*/
struct sentencia *s_return()
{
  struct nodo *nodo_expr;
  unsigned char *tipo, *tipo2;
  struct sentencia *temp;

  temp = nueva_sentencia(t_return);
  temp->def.t_return.expresion = NULL;
  temp->def.t_return.informacion = 0;

  /* Checa si hay una expresión */
  if (fin_sentencia() == 0) {
    tipo2 = almacena_expresion(SI);
    tipo = lee_entero(funcion_actual + TIPO);
    if (*++tipo == STRUCT) {                /* el primer byte es FUNCION */
      if (*tipo2 != STRUCT)
        error("El resultado no tiene tipo de estructura");
      else if (lee_entero(tipo + 1) != lee_entero(tipo2 + 1))
        error("Estructuras incompatibles");
      temp->def.t_return.informacion = tam_tipo(tipo2);
    } else if (*tipo2 == STRUCT) {
      error("La función no tiene tipo de estructura");
    } else {
      nodo_expr = ultimo_nodo;
      convierte_tipo(&nodo_expr, tipo2, tipo);
    }
    temp->def.t_return.expresion = ultimo_nodo;
  }
  punto_y_coma();
  return temp;
}

/*
** Sentencia "break"
*/
struct sentencia *s_break()
{
  struct sentencia *temp;

  temp = nueva_sentencia(t_break);
  temp->def.t_break.etiqueta = 0;
  if (ultimo_bucle == NULL)     /* Checa si hay un bucle abierto */
    error("No hay ningun bucle abierto");
  else
    temp->def.t_break.etiqueta = ultimo_bucle[B_FIN];
  punto_y_coma();
  return temp;
}

/*
** Sentencia "continue"
*/
struct sentencia *s_continue()
{
  int *u_bucle;
  struct sentencia *temp;
 
  temp = nueva_sentencia(t_continue);
  u_bucle = ultimo_bucle;     /* Checa si hay un bucle abierto */
  while (1) {
    if (u_bucle == NULL) {
      error("No hay ningun bucle abierto");
      break;
    }
    if (u_bucle[B_BUCLE]) break;
    u_bucle = u_bucle[B_ANTERIOR];
  }
  if (u_bucle != NULL)
    temp->def.t_break.etiqueta = u_bucle[B_BUCLE];
  else
    temp->def.t_break.etiqueta = 0;
  punto_y_coma();
  return temp;
}

/*
** Detecta el fin de una sentencia, un punto y coma
** o el fin de archivo.
*/
fin_sentencia()
{
  espacios();
  return ((car_act == ';') || (car_act == 0));
}

nombre_ilegal()
{
  error("Nombre ilegal");
  basura();
}

redefinido(nombre)
  unsigned char *nombre;
{
  error("Nombre redefinido");
}

pide(cadena)
  unsigned char *cadena;
{
  if (match(cadena) == 0)
    error("Falta un parentesis, llave o corchete");
}
/*
** Compilador de C para G11.
** Preprocesador y funciones varias.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 1 de junio de 1995.
** Revisión: 25 de julio de 1995. Nueva función, isspace().
** Revisión: 25 de julio de 1995. Se agrega #ifdef, #ifndef, #else y #endif
**                                al preprocesador.
** Revisión: 26 de julio de 1995. Nueva función. encuentra().
** Revisión: 26 de julio de 1995. Se agrega #undef y #line al preprocesador.
**                                Nueva función. borra_macro().
**                                Modifico busca_macro() para que retorne
**                                la posición del principio de la macro.
** Revisión: 26 de julio de 1995. Se agrega #if.
** Revisión: 26 de julio de 1995. Se pasa #define y #include al preprocesador.
** Revisión: 26 de julio de 1995. Se pasa #asm al preprocesador.
** Revisión: 27 de julio de 1995. Soporte para concatenar líneas usando \.
** Revisión: 27 de julio de 1995. Nuevas funciones. strcpy(), strcat().
** Revisión: 27 de julio de 1995. Ahora soporta #include y #define estandard.
**                                El preprocesador ya esta casí completo,
**                                faltan los #include anidados.
** Revisión: 27 de julio de 1995. Corrección de un defecto en encuentra().
** Revisión: 10 de agosto de 1995. Nueva función. lee_entero().
** Revisión: 10 de agosto de 1995. Modificación de nueva_glb y nueva_loc,
**                                 para el soporte de tipos complejos.
** Revisión: 10 de agosto de 1995. Nueva función. escribe_entero().
** Revisión: 12 de agosto de 1995. Soporte para #include anidado.
** Revisión: 23 de agosto de 1995. Nuevas funciones, busca_miembro(),
**                                 nuevo_miembro(), busca_estructura(),
**                                 nueva_estructura().
** Revisión: 24 de agosto de 1995. Nuevas funciones, busca_enum(),
**                                 nuevo_enum().
** Revisión: 29 de noviembre de 1995. Se pasa la funcion car_act() al archivo
**                                    CCVARS.C
** Revisión: 27 de diciembre de 1995. Corrección de un defecto horrible en el
**                                    preprocesamiento de macros con pars.
** Revisión: 28 de diciembre de 1995. Corrección de un error que hacia que
**                                    quedara un archivo abierto al salir por
**                                    un error.
** Revisión: 20 de junio de 1996. Hago que nueva_glb() y nueva_loc agregen
**                                el nivel de profundidad.
*/

/*
** Un nuevo bucle, lo agrega al final de la lista enlazada
*/
nuevo_bucle(ap)
  int ap[];
{
  ap[B_ANTERIOR] = ultimo_bucle;  /* Bucle anterior */
  ap[B_PILA] = pila;              /* Nivel de la pila */
  ap[B_BUCLE] = nueva_etiq;       /* Etiqueta del bucle */
  ap[B_FIN] = nueva_etiq;         /* Etiqueta de salida */
  ultimo_bucle = ap;
}

/*
** Una nueva variable/función global.
*/
nueva_glb(nombre, id, clase, tipo, valor)
  unsigned char *nombre, *tipo;
  int valor, clase, id;
{
  unsigned char *ap;

  if (ap_glb >= FIN_GLB) {
    error("Tabla global llena");
    return 0;
  }
  ap = ap_glb;
  while (alfanum(*ap++ = *nombre++));  /* Copia el nombre */
  ap = ap_glb;
  ap[IDENT] = id;
  ap[CLASE] = clase;
  ap[NIVEL] = 0;
  escribe_entero(ap + TIPO, tipo);
  escribe_entero(ap + POSICION, valor);
  ap_glb += TAM_SIM;
  return ap;
}

/*
** Una nueva variable local.
*/
nueva_loc(nombre, id, clase, tipo, valor)
  unsigned char *nombre, *tipo;
  int valor, clase, id;
{
  unsigned char *ap;

  if (ap_loc >= FIN_LOC) {
    error("Tabla local llena");
    return 0;
  }
  ap = ap_loc;
  while (alfanum(*ap++ = *nombre++));  /* Copia el nombre */
  ap = ap_loc;
  ap[IDENT] = id;
  ap[CLASE] = clase;
  ap[NIVEL] = nivel;
  escribe_entero(ap + TIPO, tipo);
  escribe_entero(ap + POSICION, valor);
  ap_loc += TAM_SIM;
  return ap;
}

/*
** Una nueva estructura.
*/
nueva_estructura(nombre)
  unsigned char *nombre;
{
  int conteo;

  if (ultima_estruct != NULL)
    escribe_entero(ultima_estruct + EST_SIG, sig_tipo);
  ultima_estruct = sig_tipo;
  if (lista_estruct == NULL)
    lista_estruct = sig_tipo;
  conteo = 0;
  while (conteo++ < EST_NOMBRE)
    guarda_tipo(0);
  while (*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
  return ultima_estruct;
}

/*
** Un nuevo miembro de una estructura.
*/
nuevo_miembro(lista, nombre)
  unsigned char **lista, *nombre;
{
  unsigned char *sig, *nuevo;
  int conteo;

  if (*lista == NULL)
    *lista = sig_tipo;
  else {
    sig = *lista;
    while (lee_entero(sig + MIE_SIG) != NULL)
      sig = lee_entero(sig + MIE_SIG);
    escribe_entero(sig + MIE_SIG, sig_tipo);
  }
  nuevo = sig_tipo;
  conteo = 0;
  while (conteo++ < MIE_NOMBRE)
    guarda_tipo(0);
  while (*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
  return nuevo;
}

/*
** Una nueva constante de un enumerador.
*/
nuevo_enum(nombre, valor)
  unsigned char *nombre;
  int valor;
{
  int conteo;

  if (ultimo_enum != NULL)
    escribe_entero(ultimo_enum + ENUM_SIG, sig_tipo);
  ultimo_enum = sig_tipo;
  if (lista_enum == NULL)
    lista_enum = sig_tipo;
  guarda_tipo(valor);
  guarda_tipo(valor >> 8);
  guarda_tipo(valor >> 16);
  guarda_tipo(valor >> 24);
  conteo = 0;
  while (conteo++ < 4)
    guarda_tipo(0);
  while (*nombre)
    guarda_tipo(*nombre++);
  guarda_tipo(0);
}

/*
** Busca una variable/función global.
*/
busca_glb(nombre)
  unsigned char *nombre;
{
  unsigned char *ap;

  ap = INICIO_GLB;
  while (ap != ap_glb) {
    if (astreq(nombre, ap, MAX_NOMBRE))
      return ap;
    ap += TAM_SIM;
  }
  return NULL;
}

/*
** Busca una variable/función local.
*/
busca_loc(nombre)
  unsigned char *nombre;
{
  unsigned char *ap;

  ap = ap_loc;
  while (ap != INICIO_LOC) {
    ap -= TAM_SIM;
    if (astreq(nombre, ap, MAX_NOMBRE))
      return ap;
  }
  return NULL;
}

/*
** Busca una estructura.
*/
busca_estructura(nombre)
  unsigned char *nombre;
{
  unsigned char *ap;

  ap = lista_estruct;
  while (ap != NULL) {
    if (astreq(nombre, ap + EST_NOMBRE, MAX_NOMBRE))
      return ap;
    ap = lee_entero(ap + EST_SIG);
  }
  return NULL;
}

/*
** Busca un miembro de estructura.
*/
busca_miembro(lista, nombre)
  unsigned char *lista, *nombre;
{
  while (lista != NULL) {
    if (astreq(nombre, lista + MIE_NOMBRE, MAX_NOMBRE))
      return lista;
    lista = lee_entero(lista + MIE_SIG);
  }
  return NULL;
}

/*
** Busca un enumerador.
*/
busca_enum(nombre)
  unsigned char *nombre;
{
  unsigned char *ap;

  ap = lista_enum;
  while (ap != NULL) {
    if (astreq(nombre, ap + ENUM_NOMBRE, MAX_NOMBRE))
      return ap;
    ap = lee_entero(ap + ENUM_SIG);
  }
  return NULL;
}

/*
** Checa si la proxima cadena de entrada es un nombre legal.
*/
nombre_legal(nombre)
  unsigned char *nombre;
{
  int k;

  espacios();
  if (letra(car_act) == 0)
    return (*nombre = 0);
  k = 0;
  while (alfanum(car_act)) {
    if (k < MAX_NOMBRE)
      nombre[k++] = obt_car();
    else
      obt_car();
  }
  nombre[k] = 0;
  return 1;
}

/*
** Imprime un retorno de carro y una cadena a la consola.
*/
mensaje(cad)
  unsigned char *cad;
{
#ifndef FENIX
  putchar('\n');
  while (*cad)
    putchar(*cad++);
#endif
}

/*
** Siguiente caracter en la línea.
*/
prox_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea + 1];
}

/*
** Pasa al siguiente caracter, retorna el caracter anterior.
*/
obt_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea++];
}

/*
** Descarta la línea actual.
*/
descarta()
{
  pos_linea = 0;
  linea[pos_linea] = 0;
}

/*
** Obtiene un caracter, si era el último en la línea, carga
** otra línea.
*/
lee_car()
{
  while (car_act == 0) {
    if (eof)
      return 0;
    preprocesa();
  }
  return obt_car();
}

/*
** Obtiene otra linea de la entrada.
*/
lee_linea()
{
  int k;
  FILE *unidad;

  while (1) {
    descarta();
    if (entrada == 0) {
      eof = 1;
      return;
    }
    unidad = entrada;
    while ((k = fgetc(unidad)) > 0) {
      if (k == 13)
        continue;
      if ((k == '\n') || (pos_linea >= MAX_LINEA))
        break;
      linea[pos_linea++] = k;
    }
    linea[pos_linea] = 0;   /* Agrega un caracter nulo */
    linea_actual++;         /* Se ha leido una línea más */
    if (k <= 0) {
      if (nivel_incl)
        fin_include();
      else {
        fclose(entrada);
        entrada = 0;
      }
    }
    if (pos_linea) {
      pos_linea = 0;
      return;
    }
  }
}

/*
** Hace el preprocesamiento.
*/
preprocesa()
{
  int k, car, hay_if;
  unsigned char c, nombre[TAM_NOMBRE], *def, *busqueda;
  int subs, pars, args, paren, m;

  hay_if = 0;
  dentro_pp = SI;
  while (1) {
    primer_paso();
    if (eof) {
      dentro_pp = NO;
      return;
    }
    espacios();
    if (car_act == '#') {
      pos_linea++;
      espacios();
      if (match("ifdef")) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        nombre_legal(nombre);
        if (busca_macro(nombre) == 0)
          evadir_nivel = nivel_if;
        continue;
      }
      if (match("ifndef")) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        nombre_legal(nombre);
        if (busca_macro(nombre))
          evadir_nivel = nivel_if;
        continue;
      }
      if (match("if")) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        hay_if = pos_linea;
        break;
      }
      if (match("else")) {
        if (nivel_if) {
          if (evadir_nivel == nivel_if)
            evadir_nivel = 0;
          else if (evadir_nivel == 0)
            evadir_nivel = nivel_if;
        } else
          error("No hay #if...");
        continue;
      }
      if (match("endif")) {
        if (nivel_if) {
          if (evadir_nivel == nivel_if)
            evadir_nivel = 0;
          --nivel_if;
        } else error("No hay #if...");
        continue;
      }
      if (evadir_nivel)
        continue;
      if (match("asm")) {
        while (1) {
          lee_linea();
          if (eof) {
            dentro_pp = NO;
            return;
          }
          espacios();
          if (car_act == '#')
            break;
          emite_linea(linea + pos_linea);
        }
        continue;
      }
      if (match("include")) {
        p_include();
        continue;
      }
      if (match("define")) {
        nueva_macro();
        continue;
      }
      if (match("undef")) {
        nombre_legal(nombre);
        borra_macro(nombre);
        continue;
      }
      if (match("line"))
        continue;
    }
    if (evadir_nivel)
      continue;
    break;
  }
  pos_linea = hay_if;
  subs = SI;
  while (subs) {
    subs = NO;
    pos_linea_m = 0;
    while (car = car_act) {
      if (isspace(car))
        pp_espacios();
      else if (car == '"')
        pp_comillas();
      else if (car == 39)
        pp_apostrofe();
      else if (letra(car)) {
        k = 0;
        while (alfanum(car_act)) {
          if (k < MAX_NOMBRE)
            nombre[k++] = car_act;
          obt_car();
        }
        nombre[k] = 0;
        if (k = busca_macro(nombre)) {
          m = 0;
          while (macs[k++]);
          pars = macs[k++];
          def = macs + k;
          if (pars) {
            espacios();
            if (car_act != '(')
              error("falta (");
            obt_car();
            args = paren = 0;
            while (car_act && car_act != ')') {
              espacios();
              while (car_act) {
                if (car_act == ',' && paren == 0)
                  break;
                if (car_act == '(')
                  ++paren;
                if (car_act == ')') {
                  if (paren == 0)
                    break;
                  --paren;
                }
                if (m < MAX_AMAC)
                  amacs[m++] = obt_car();
                else {
                  error("Tabla de parametros de macros llena");
                  cancela();
                }
              }
              amacs[m++] = 0;
              if (car_act == ',')
                obt_car();
              ++args;
            }
            if (args != pars)
              error("Número incorrecto de argumentos");
            if (car_act != ')')
              error("Falta )");
            obt_car();
          }
          while (*def) {
            if (*def != 127)
              almacena_car(*def++);
            else {
              busqueda = amacs;
              k = *++def;
              while (--k)
                while (*busqueda++) ;
              while (*busqueda)
                almacena_car(*busqueda++);
              ++def;
            }
          }
          subs = SI;
        } else {
          k = 0;
          while (c = nombre[k++])
            almacena_car(c);
        }
      } else
        almacena_car(obt_car());
    }
    if (hay_if)                /* Si hay #if, agrega un ; para que no vaya */
      almacena_car(';');       /* a salirse de la linea en caso de error. */
    almacena_car(0);
    strcpy(linea, linea_m);
    pos_linea = 0;
    if (pos_linea_m >= MAX_LINEA) {
      error("Línea muy larga");
      break;
    }
  }
  if (hay_if) {
    if (expr_constante() == 0)
      evadir_nivel = nivel_if;
    descarta();
  }
  dentro_pp = NO;
}

/*
** Primer paso del preprocesamiento, pega líneas terminadas en \, y
** elimina los comentarios.
*/
primer_paso()
{
  int car;

  lee_linea();
  pos_linea = pos_linea_m = 0;
  if (eof)
    return;
  while (car = car_act) {
    if (isspace(car))
      pp_espacios();
    else if ((car == '\\') && (prox_car() == 0)) {
      lee_linea();
      if (eof)
        return;
    } else if (car == '"')
      pp_comillas();
    else if (car == 39)
      pp_apostrofe();
    else if ((car == '/') && (prox_car() == '*')) {
      almacena_car(' ');
      pp_comentarios();
    } else
      almacena_car(obt_car());
  }
  almacena_car(0);
  if (pos_linea_m >= MAX_LINEA)
    error("Línea muy larga");
  pos_linea = pos_linea_m = 0;
  while (linea[pos_linea++] = linea_m[pos_linea_m++]);
  pos_linea = 0;
}

almacena_car(c)
  unsigned char c;
{
  linea_m[pos_linea_m] = c;
  if (pos_linea_m < MAX_LINEA)
    pos_linea_m++;
  return c;
}

/*
** elimina espacios y tabuladores extras.
*/
pp_espacios()
{
  almacena_car(' ');
  while (isspace(car_act))
    obt_car();
}

/*
** Procesa cadenas de caracteres.
*/
pp_comillas()
{
  almacena_car(obt_car());
  while ((car_act != '"') ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Faltan comillas");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car('"');
}

/*
** Procesa caracteres encerrados entre '
*/
pp_apostrofe()
{
  almacena_car(39);
  obt_car();
  while ((car_act != 39) ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Falta un apostrofe");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car(39);
}

/*
** Procesa y elimina comentarios.
*/
pp_comentarios()
{
  pos_linea = pos_linea + 2;
  while ((car_act != '*') ||
         (prox_car() != '/')) {
    if (car_act == 0)
      lee_linea();
    else
      ++pos_linea;
    if (eof)
      break;
  }
  pos_linea = pos_linea + 2;
}

/*
** Añade una nueva macro en la tabla.
*/
nueva_macro()
{
  unsigned char nombre[TAM_NOMBRE];
  int k;
  int num_args;
  int l;        /* indice en la tabla de argumentos de macros */
  int numero;
  unsigned char *busqueda;

  if (nombre_legal(nombre) == 0) {
    nombre_ilegal();
    descarta();
    return;
  }
  borra_macro(nombre);
  k = 0;
  while (pone_macro(nombre[k++]));
  num_args = l = 0;
  if (car_act == '(') {   /* genera una lista de nombres de parametros */
    obt_car();
    while (car_act != ')') {
      espacios();
      if (letra(car_act)) {
        while (alfanum(car_act)) {
          if (l < MAX_AMAC)
            amacs[l++] = obt_car();
          else {
            error("Tabla de parametros de macros llena");
            cancela();
          }
        }
        amacs[l++] = 0;
        num_args++;
      }
      espacios();
      if (car_act == ',')
        obt_car();
      else if (car_act != ')') {
        error("Falta ) en #define");
        break;
      }
    }
    obt_car();
    amacs[l++] = 0;
  }
  espacios();
  pone_macro(num_args);  /* Ahora substituye los nombres por */
  while (car_act) {     /* secuencias 0x7f num */
    if (num_args && letra(car_act)) {
      numero = 1;
      busqueda = amacs;
      while (*busqueda) {         /* rastrea nombres */
        k = 0;
        while (1) {
          if (busqueda[k] != linea[pos_linea + k])
            break;
          if (linea[pos_linea + k] < ' ')
            break;
          k++;
        }
        if (alfanum(busqueda[k]) || alfanum(linea[pos_linea + k])) {
          ++numero;
          while (*busqueda++) ;
        } else {
          pos_linea += k;
          pone_macro(127);
          pone_macro(numero);
          break;
        }
      }
      if (*busqueda == 0)
        while (alfanum(car_act))
          pone_macro(obt_car());
    } else {
      pone_macro(obt_car());
    }
  }
  pone_macro(0);
  if (ap_mac >= MAX_MAC)
    error("Tabla de macros llena");
}

/*
** Elimina una macro de la tabla.
*/
borra_macro(nombre)
  unsigned char *nombre;
{
  int k, l, m;

  if (k = busca_macro(nombre)) {   /* Obtiene el comienzo de la macro */
    l = k;
    while (macs[k++]) ;            /* Busca el comienzo de la siguiente */
    k++;                           /* Evade la cuenta de argumentos */
    while (macs[k++]) ;
    m = k - l;
    while (k != ap_mac)            /* Mueve el bloque hacia atras */
      macs[l++] = macs[k++];
    ap_mac = ap_mac - m;           /* Ahora hay más espacio libre */
  }
}

pone_macro(c)
  unsigned char c;
{
  macs[ap_mac] = c;
  if (ap_mac < MAX_MAC)
    ap_mac++;
  return c;
}

/*
** Busca una macro en la tabla.
*/
busca_macro(nombre)
  unsigned char *nombre;
{
  int k;

  k = 1;
  while (k < ap_mac) {
    if (astreq(nombre, macs + k, MAX_NOMBRE))
      return k;
    while (macs[k++]);
    k++;
    while (macs[k++]);
  }
  return 0;
}

/*
** Desvia la salida a la consola.
*/
hacia_consola()
{
  desvio_salida = salida;
  salida = 0;
  color(15);
}

/*
** Regresa la salida al archivo.
*/
hacia_archivo()
{
  if (desvio_salida)
    salida = desvio_salida;
  desvio_salida = 0;
}

/*
** Vacia el buffer
*/
vacia_buffer()
{
  int a, b;

  a = total_lineas;
  for (b = 0; b < MAX_LIN; b++)
    emite_texto("\2\n");
  buffer_vacio = SI;
}

/*
** Manda un caracter a la salida.
*/
emite_car(c)
  unsigned char c;
{
  char *ap, *ap1;
  int val0, val1, val2;

  if (c == 0)
    return 0;
  if (salida) {
    *ap_buf_retrasado++ = c;
    if (c == '\n') {
      buffer_vacio = NO;
      *ap_buf_retrasado++ = 0;
      val0 = (total_lineas - 2) & (MAX_LIN - 1);
      val1 = (total_lineas - 3) & (MAX_LIN - 1);
      val2 = (total_lineas - 4) & (MAX_LIN - 1);
      if (((estado_buf[val0] >= 1 && estado_buf[val0] <= 4)
        || estado_buf[val0] == 7)
       && ((estado_buf[val1] == 8)
        || (estado_buf[val1] == 11 && estado_buf[val2] == 8))) {
        if (estado_buf[val1] == 8)
          linea_inst[val1] = NULL;
        else
          linea_inst[val2] = NULL;
      }
      if (estado_buf[val0] == 10 && estado_buf[val1] == 8
      && (estado_buf[val2] & ~1) == 12) {
        linea_inst[val1] = NULL;
        val0 = *(ap = linea_inst[val0] + 3) == 'i';
        ap1 = linea_inst[val2] + 3;
        *ap++ = *ap1++ ^ 0x12;
        if (val0)
          *ap++ = 'i';
        while ((*ap++ = *ap1++) != ',');
      }
      total_lineas = (total_lineas + 1) & (MAX_LIN - 1);
      if ((ap = linea_inst[total_lineas]) != NULL) {
        while (*ap && *ap != '\2') {
          if (*ap == '\1')
            ++ap;
          else if (fputc(*ap++, salida) <= 0) {
            cierra_salida();
            error("Error al escribir");
            cancela();
          }
        }
      }
      if (ap_buf_retrasado - buf_retrasado >= (MAX_BUFR - 64))
        ap_buf_retrasado = buf_retrasado;
      linea_inst[total_lineas] = ap_buf_retrasado;
      estado_buf[total_lineas] = 0;
    }
  } else
#ifndef FENIX
    putchar(c);
#else
    ;
#endif
  return c;
}

/*
** Cambio de linea a la salida.
*/
emite_nueva_linea()
{
  emite_car('\n');
}

/*
** Manda una línea a la salida, hace un cambio de linea también.
*/
emite_linea(ap)
  unsigned char *ap;
{
  emite_texto(ap);
  emite_nueva_linea();
}

/*
** Manda un texto a la salida.
*/
emite_texto(ap)
  unsigned char *ap;
{
  while (emite_car(*ap++));
}

/*
** Saca un número decimal en la salida.
*/
emite_numero(numero)
  int numero;
{
  if (numero < 0) {
    emite_car('-');
    if (numero < -9)
      emite_numero(-(numero / 10));
    emite_car(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      emite_numero(numero / 10);
    emite_car((numero % 10) + '0');
  }
}

/*
** Ilustra los mensajes de error.
*/
error(ap)
  unsigned char ap[];
{
  int k;
  unsigned char entrada[81];

#ifndef FENIX
  hacia_consola();
  color(11);
  emite_texto("Línea ");
  emite_numero(linea_actual);
  emite_texto(", ");
  if (!dentro_funcion)
    emite_car('(');
  if (funcion_actual == NULL)
    emite_texto("comienzo del archivo");
  else
    emite_texto(funcion_actual + NOMBRE);
  if (!dentro_funcion)
    emite_car(')');
  emite_texto(" + ");
  emite_numero(linea_actual - comienzo_funcion);
  emite_texto(": ");
  color(15);
  emite_texto(ap);
  emite_nueva_linea();

  color(14);
  emite_texto(linea);
  emite_nueva_linea();

  k = 0;                /* Busca la posición del error */
  while (k < pos_linea) {
    if (linea[k++] == 9)
      emite_car(9);
    else
      emite_car(' ');
  }
  emite_car('^');
  emite_nueva_linea();
  ++errores;

  hacia_archivo();
  if (pausa) {
    color(10);
    mensaje("¿ Continuar (Si, No, Pasar de largo) ? ");
    gets(entrada);
    k = entrada[0];
    if ((k == 'N') || (k == 'n'))
      cancela();
    if ((k == 'P') || (k == 'p'))
      pausa = NO;
  }
#endif
}

/*
** Checa si encuentra un operador de expresión.
*/
encuentra(op)
  unsigned char *op;
{
  int tam_op;

  espacios();
  if (tam_op = streq(linea + pos_linea, op))
    if ((*(linea + pos_linea + tam_op) != '=') &&
        (*(linea + pos_linea + tam_op) != *(linea + pos_linea + tam_op - 1)))
      return 1;
  return 0;
}

streq(cad1, cad2)
  unsigned char cad1[], cad2[];
{
  int k;

  k = 0;
  while (cad2[k]) {
    if ((cad1[k]) != (cad2[k]))
      return 0;
    k++;
  }
  return k;
}

astreq(cad1, cad2, len)
  unsigned char cad1[], cad2[];
  int len;
{
  int k;

  k = 0;
  while (k < len) {
    if ((cad1[k]) != (cad2[k]))
      break;
    if (cad1[k] == 0)
      break;
    if (cad2[k] == 0)
      break;
    k++;
  }
  if (alfanum(cad1[k]))
    return 0;
  if (alfanum(cad2[k]))
    return 0;
  return k;
}

match(lit)
  unsigned char *lit;
{
  int k;

  espacios();
  if (k = streq(linea + pos_linea, lit)) {
    pos_linea = pos_linea + k;
    return 1;
  }
  return 0;
}

amatch(lit, len)
  unsigned char *lit;
  int len;
{
  int k;

  espacios();
  if (k = astreq(linea + pos_linea, lit, len)) {
    pos_linea = pos_linea + k;
    while (alfanum(car_act))
      lee_car();
    return 1;
  }
  return 0;
}

/*
** Salta los espacios en la entrada.
*/
espacios()
{
  while (1) {
    while (car_act == 0) {
      if (dentro_pp) return;
      preprocesa();
      if (eof)
        break;
    }
    if (isspace(car_act))
      obt_car();
    else
      return;
  }
}

/*
** Compone un entero.
*/
lee_entero(dir)
  unsigned char *dir;
{
  return *dir | (*(dir+1) << 8) | (*(dir+2) << 16) | (*(dir+3) << 24);
}

/*
** Escribe un entero en una dirección.
*/
escribe_entero(dir, dato)
  unsigned char *dir;
  int dato;
{
  *dir++ = dato;
  *dir++ = dato >> 8;
  *dir++ = dato >> 16;
  *dir++ = dato >> 24;
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

/*
** Prueba si el caracter dado es alfanumérico.
*/
alfanum(c)
  unsigned char c;
{
  return ((letra(c)) || (isdigit(c)));
}

/*
** Evade basura en la entrada.
*/
basura()
{
  if (alfanum(lee_car()))
    while (alfanum(car_act))
      obt_car();
  else
    while (alfanum(car_act) == 0) {
      if (car_act == 0)
        break;
      obt_car();
    }
  espacios();
}

/*
** Prueba si el caracter dado es un número.
*/
isdigit(c)
  int c;
{
  return ((c >= '0') && (c <= '9'));
}

/*
** Checa si es un número hexadecimal.
*/
isxdigit(c)
  unsigned char c;
{
  return (((c >= '0') && (c <= '9')) ||
          ((c >= 'A') && (c <= 'F')) ||
          ((c >= 'a') && (c <= 'f')));
}

/*
** Checa si es un espacio.
*/
isspace(c)
  unsigned char c;
{
  return (c == ' ') || (c == 9);
}

/*
** Conversión a máyusculas.
*/
toupper(c)
  unsigned char c;
{
  if ((c >= 'a') && (c <= 'z'))
    c = c + ('A' - 'a');
  return (c);
}

/*
** Retorna el tamaño de una cadena.
*/
strlen(s)
  unsigned char *s;
{
  unsigned char *t;

  t = s;
  while (*s)
    s++;
  return (s - t);
}

/*
** Copia una cadena.
*/
strcpy(destino, origen)
  unsigned char *destino, *origen;
{
  while (*destino++ = *origen++);
}

/*
** Concatena una cadena.
*/
strcat(destino, origen)
  unsigned char *destino, *origen;
{
  while (*destino) ++destino;
  strcpy(destino, origen);
}
/*
** Compilador de C para G11.
** Evaluador de Expresiones.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 4 de junio de 1995.
** Revisión: 20 de julio de 1995. Ahora efectua correctamente suma y resta
**                                con apuntadores.
** Revisión: 25 de julio de 1995. Soporte para || y &&.
** Revisión: 25 de julio de 1995. Soporte para el operador trinario ?.
** Revisión: 26 de julio de 1995. Ahora da correctamente el resultado de una
**                                resta de dos apuntadores.
** Revisión: 26 de julio de 1995. Soporte para |= ^= &= += -= /= %= <<= >>=.
** Revisión: 26 de julio de 1995. Nueva función. expr_constante().
** Revisión: 27 de julio de 1995. Soporte para el operador coma.
** Revisión: 29 de julio de 1995. Corrección de los operadores ++ y -- para
**                                variables de tipo char.
** Revisión: 31 de julio de 1995. Corrección de un defecto en el posdecremento.
** Revisión: 10 de agosto de 1995. Soporte para tipos complejos.
** Revisión: 11 de agosto de 1995. Desaparece nivel14(), se combina con
**                                 primaria().
** Revisión: 11 de agosto de 1995. Soporte para short y unsigned short.
** Revisión: 11 de agosto de 1995. Soporte para sizeof.
** Revisión: 11 de agosto de 1995. Soporte para conversiones de tipo.
** Revisión: 12 de agosto de 1995. Corrección de defectos en sizeof y
**                                 conversiones de tipo.
** Revisión: 23 de agosto de 1995. Soporte para los operadores -> y .
** Revisión: 24 de agosto de 1995. Soporte para enumeradores.
** Revisión: 5 de septiembre de 1995. Soporte para asignación, paso cómo
**                                    parametros y resultados de estructuras.
** Revisión: 6 de septiembre de 1995. Nueva función. nivel13dir().
** Revisión: 12 de septiembre de 1995. Soporte para float y double.
** Revisión: 12 de septiembre de 1995. Nuevas funciones. convierte_tipo().
**                                     checa_entero(), checa_numerico(),
**                                     compara_no_cero(), compara_cero(),
**                                     checa_entero_o_apuntador(),
**                                     haz_compatible().
** Revisión: 22 de septiembre de 1995. Corrección de varios errores en
**                                     convierte_tipo y en nivel1().
** Revisión: 27 de septiembre de 1995. Corrección de varios errores.
** Revisión: 22 de noviembre de 1995. Soporte para números de punto flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    del exponente en números de punto
**                                    flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    de estructuras.
** Revisión: 30 de noviembre de 1995. Generación de nodos para optimización
**                                    de operaciones con float.
** Revisión: 1o. de diciembre de 1995. Corrección de varios defectos.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en el manejo
**                                    de unsigned int, no efectuaba bien las
**                                    comparaciones.
** Revisión: 28 de diciembre de 1995. Ahora soporta la letra L en los números.
** Revisión: 28 de diciembre de 1995. Ahora balancea correctamente unsigned.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en sizeof.
** Revisión: 3 de enero de 1996. Ahora evalua en tiempo de compilación las
**                               expresiones constantes de | & y ^. (¿cómo se
**                               me fue a olvidar?).
** Revisión: 18 de febrero de 1996. Soporte para el operador coma en sentencias
**                                  condicionales. (¿cómo se me fue a olvidar?)
** Revisión: 19 de febrero de 1996. Corrección de defectos en la sintaxis de
**                                  las expresiones.
** Revisión: 9 de marzo de 1996. Genera árboles más óptimos para comparaciones
**                               entre apuntadores.
** Revisión: 8 de abril de 1996. Corrección de un defecto en el procesamiento
**                               de números reales.
** Revisión: 10 de abril de 1996. Corrección de un defecto en la generación de
**                                codigo para el operador trinario.
** Revisión: 15 de abril de 1996. Corrección de un defecto que no permitia el
**                                acceso a estructuras en matrices.
** Revisión: 7 de mayo de 1998. Ahora los árboles de expresiones se crean
**                              dinámicamente.
** Revisión: 8 de mayo de 1998. Empieza la conversión a G11.
*/

/*
** La matriz info contiene información sobre la expresión.
**
** info[0] - Tipo de la expresión hasta el momento.
*/

/*
** Procesa una expresión constante.
*/
expr_constante()
{
  int origen, valor;

  origen = ultimo_nodo;
  checa_entero(almacena_expresion(SI));
  if (ultimo_nodo->oper != N_CONST) {
    libera_arbol(ultimo_nodo);
    ultimo_nodo = origen;
    error("No es una expresión constante");
    return 1;
  } else {
    valor = ultimo_nodo->esp;
    libera_arbol(ultimo_nodo);
    ultimo_nodo = origen;
    return valor;
  }
}

/*
** Analiza una expresión y la mantiene en memoria.
*/
almacena_expresion(operador_coma)
  int operador_coma;
{
  int info[1];

  if (operador_coma) {
    if (nivel0(info))
      carga_valor(info);
  } else {
    if (nivel1(info))
      carga_valor(info);
  }
  return info[0];
}

nivel0(info)
  int info[];
{
  int k;
  struct nodo *izq;

  k = nivel1(info);
  while (match(",")) {
    if (k)
      carga_valor(info);
    izq = ultimo_nodo;
    k = nivel1(info);
    crea_nodo(N_COMA, izq, ultimo_nodo, 0);
  }
  return k;
}

nivel1(info)
  int info[];
{
  int k, info2[1], op;
  struct nodo *der, *izq;
  unsigned char *tipo, *tipo2;

  k = nivel2(info);
  if (match("="))
    op = N_ASIGNA;
  else if (match("|="))
    op = N_AOR;
  else if (match("^="))
    op = N_AXOR;
  else if (match("&="))
    op = N_AAND;
  else if (match("<<="))
    op = N_ACI;
  else if (match(">>="))
    op = N_ACD;
  else if (match("+="))
    op = N_ASUMA;
  else if (match("-="))
    op = N_ARESTA;
  else if (match("*="))
    op = N_AMUL;
  else if (match("/="))
    op = N_ADIV;
  else if (match("%="))
    op = N_AMOD;
  else
    return k;
  der = ultimo_nodo;
  tipo = info[0];
  if (k == 0)
    if (*tipo != STRUCT)
      req_valorl();
  if (nivel1(info2)) {
    if (*tipo == STRUCT)
      req_valorl();
    carga_valor(info2);
    izq = ultimo_nodo;
  } else if (*tipo == STRUCT) {
    tipo2 = info2[0];
    if (*tipo2 != STRUCT)
      error("Se requiere una estructura o unión");
    else {
      if (lee_entero(tipo + 1) != lee_entero(tipo2 + 1))
        error("Estructuras incompatibles");
    }
    if (op != N_ASIGNA)
      error("Asignación incompatible");
    crea_nodo(N_COPIA, der, ultimo_nodo, (tam_tipo(tipo) + 3) / 4);
    return 0;
  } else
    izq = ultimo_nodo;
  tipo2 = info2[0];
  convierte_tipo(&izq, tipo2, tipo);
  if ((*tipo == DOUBLE || *tipo == FLOAT)
    && (op == N_AOR || op == N_AXOR
     || op == N_AAND || op == N_ACI
     || op == N_ACD || op == N_AMOD))
    error("No se puede hacer esta operación con reales");
  if ((op == N_ASUMA) || (op == N_ARESTA)) {
    if (k = dobla(tipo, izq)) {
      if (k == 2) {
        if (multi == 8) {
          crea_nodo(N_CONST, NULL, NULL, 3);
          crea_nodo(N_CI, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        } else if (multi == 4) {
          crea_nodo(N_CONST, NULL, NULL, 2);
          crea_nodo(N_CI, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        } else if (multi == 2) {
          crea_nodo(N_CONST, NULL, NULL, 1);
          crea_nodo(N_CI, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        } else if (multi != 1) {
          crea_nodo(N_CONST, NULL, NULL, multi);
          crea_nodo(N_MUL, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        }
      }
    }
  }
  if ((op == N_ARESTA) && (izq->oper == N_CONST)) {
    op = N_ASUMA;
    izq->esp = -izq->esp;
  }
  if (*tipo == APUNTADOR)
    tipo = t_int;
  crea_nodo(op, izq, der, *tipo);
  return 0;
}

nivel2(info)
  int info[];
{
  int k, info2[1], info3[1];
  struct nodo *ext, *izq, *der;

  k = nivel3(info);
  espacios();
  if (car_act != '?')
    return k;
  if (k)
    carga_valor(info);
  ext = ultimo_nodo;
  pos_linea++;
  if (nivel0(info2))
    carga_valor(info2);
  izq = ultimo_nodo;
  pide(":");
  if (nivel2(info3))
    carga_valor(info3);
  der = ultimo_nodo;
  if ((ext->oper == N_CONST) &&
     (izq->oper == N_CONST) &&
     (der->oper == N_CONST)) {
    if (ext->esp) ultimo_nodo->esp = izq->esp;
    libera_arbol(ext);
    libera_arbol(izq);
  } else {
    haz_compatible(&izq, info2, &der, info3);
    crea_nodo(N_TRI, izq, der, ext);
  }
  info[0] = info2[0];
  return 0;
}

nivel3(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel4(info);
  espacios();
  if (streq(linea + pos_linea, "||") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("||")) {
    checa_numerico(info[0]);
    compara_no_cero(info[0]);
    izq = ultimo_nodo;
    if (nivel4(info2))
      carga_valor(info2);
    checa_numerico(info2[0]);
    compara_no_cero(info2[0]);
    crea_nodo(N_ORB, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel4(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel5(info);
  espacios();
  if (streq(linea + pos_linea, "&&") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("&&")) {
    checa_numerico(info[0]);
    compara_no_cero(info[0]);
    izq = ultimo_nodo;
    if (nivel5(info2))
      carga_valor(info2);
    checa_numerico(info2[0]);
    compara_no_cero(info2[0]);
    crea_nodo(N_ANDB, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel5(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel6(info);
  if (encuentra("|") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("|")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel6(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp |= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_OR, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel6(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel7(info);
  if (encuentra("^") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("^")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel7(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp ^= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_XOR, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel7(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel8(info);
  if (encuentra("&") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("&")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(info[0]);
    if (nivel8(info2))
      carga_valor(info2);
    checa_entero(info2[0]);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp &= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_AND, izq, ultimo_nodo, 0);
  }
  return 0;
}

nivel8(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq, *der;

  k = nivel9(info);
  if ((encuentra("==") == 0) &&
      (encuentra("!=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("==")) {
      pos_linea += 2;
      if (nivel9(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2))
        crea_nodo(N_IGUALPF, izq, der, 0);
      else if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp == der->esp);
        libera_arbol(izq);
        libera_arbol(der);
      } else
        crea_nodo(N_IGUAL, izq, der, 0);
    } else if (encuentra("!=")) {
      pos_linea += 2;
      if (nivel9(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_IGUALPF, izq, der, 0);
        crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
      } else if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp != der->esp);
        libera_arbol(izq);
        libera_arbol(der);
      } else
        crea_nodo(N_NOIGUAL, izq, der, 0);
    } else
      return 0;
    info[0] = t_int;
  }
}

nivel9(info)
  int info[];
{
  int k;

  k = nivel10(info);
  if ((encuentra("<") == 0) &&
      (encuentra(">") == 0) &&
      (encuentra("<=") == 0) &&
      (encuentra(">=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    if (encuentra("<=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(1, info);
    } else if (encuentra(">=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(2, info);
    } else if (encuentra("<")) {
      pos_linea++;
      nivel9eval(3, info);
    } else if (encuentra(">")) {
      pos_linea++;
      nivel9eval(4, info);
    } else
      return 0;
  }
}

nivel9eval(k, info)
  int k, info[];
{
  int info2[1];
  struct nodo *izq, *der;
  unsigned char *tipo;

  izq = ultimo_nodo;
  if (nivel10(info2))
    carga_valor(info2);
  der = ultimo_nodo;
  if (haz_compatible(&izq, info, &der, info2)) {
    if (k == 4)
      crea_nodo(N_MAYORPF, izq, der, 0);
    else if (k == 2)
      crea_nodo(N_MAYORIPF, izq, der, 0);
    else if (k == 3)
      crea_nodo(N_MAYORPF, der, izq, 0);
    else
      crea_nodo(N_MAYORIPF, der, izq, 0);
    info[0] = t_int;
    return;
  }
  tipo = info[0];
  if (*tipo == UINT) {
    nivel9op(izq, k);
    info[0] = t_int;
    return;
  }
  tipo = info2[0];
  if (*tipo == UINT) {
    nivel9op(izq, k);
    info[0] = t_int;
    return;
  }
  if (k == 4) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp > der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MAYOR, izq, der, 0);
  } else if (k == 3) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp < der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MENOR, izq, der, 0);
  } else if (k == 1) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp <= der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MENORI, izq, der, 0);
  } else {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp >= der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MAYORI, izq, der, 0);
  }
  info[0] = t_int;
}

nivel9op(izq, k)
  int izq, k;
{
  if (k == 4)
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
  else if (k == 3)
    crea_nodo(N_SMENOR, izq, ultimo_nodo, 0);
  else if (k == 1)
    crea_nodo(N_SMENORI, izq, ultimo_nodo, 0);
  else
    crea_nodo(N_SMAYORI, izq, ultimo_nodo, 0);
}

nivel10(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq;

  k = nivel11(info);
  if ((encuentra(">>") == 0) &&
      (encuentra("<<") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra(">>")) {
      pos_linea += 2;
      checa_entero(info[0]);
      if (nivel11(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      if ((izq->oper == N_CONST) && (ultimo_nodo->oper == N_CONST)) {
        ultimo_nodo->esp = izq->esp >> ultimo_nodo->esp;
        libera_arbol(izq);
      } else
        crea_nodo(N_CD, izq, ultimo_nodo, 0);
    } else if (encuentra("<<")) {
      pos_linea += 2;
      checa_entero(info[0]);
      if (nivel11(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      if ((izq->oper == N_CONST) && (ultimo_nodo->oper == N_CONST)) {
        ultimo_nodo->esp = izq->esp << ultimo_nodo->esp;
        libera_arbol(izq);
      } else
        crea_nodo(N_CI, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

nivel11(info)
  int info[];
{
  int k, info2[1], tam;
  struct nodo *izq, *der;
  unsigned char *tipo;

  k = nivel12(info);
  if ((encuentra("+") == 0) &&
      (encuentra("-") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("+")) {
      pos_linea++;
      if (nivel12(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_SUMAPF, izq, der, 0);
      } else {
        if (k = dobla(info[0], der)) {
          if (k == 2 && ultimo_nodo->oper == N_CONST)
            ultimo_nodo->esp *= multi;
          else if (k == 2 && ((multi == 2) || (multi == 4) || (multi == 8))) {
            if (multi == 2)
              crea_nodo(N_CONST, NULL, NULL, 1);
            else if (multi == 4)
              crea_nodo(N_CONST, NULL, NULL, 2);
            else if (multi == 8)
              crea_nodo(N_CONST, NULL, NULL, 3);
            crea_nodo(N_CI, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          } else if (k == 2 && multi != 1) {
            crea_nodo(N_CONST, NULL, NULL, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        } else {
          info[0] = info2[0];
          if (k = dobla(info[0], izq)) {
            if (k == 2 && ((multi == 2) || (multi == 4) || (multi == 8))) {
              if (multi == 2)
                crea_nodo(N_CONST, NULL, NULL, 1);
              else if (multi == 4)
                crea_nodo(N_CONST, NULL, NULL, 2);
              else if (multi == 8)
                crea_nodo(N_CONST, NULL, NULL, 3);
              crea_nodo(N_CI, izq, ultimo_nodo, 0);
              izq = ultimo_nodo;
            } else if (k == 2 && multi != 1) {
              crea_nodo(N_CONST, NULL, NULL, multi);
              crea_nodo(N_MUL, izq, ultimo_nodo, 0);
              izq = ultimo_nodo;
            }
          }
        }
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp + der->esp);
          libera_arbol(der);
          libera_arbol(izq);
        } else
          crea_nodo(N_SUMA, izq, der, 0);
      }
    } else if (encuentra("-")) {
      pos_linea++;
      if (nivel12(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_RESTAPF, izq, der, 0);
      } else {
        tipo = info[0];
        if (*tipo == APUNTADOR || *tipo == MATRIZ) {
          tipo = info2[0];
          if (*tipo == APUNTADOR || *tipo == MATRIZ) {
            crea_nodo(N_RESTA, izq, der, 0);
            if (*tipo == APUNTADOR)
              tam = tam_tipo(tipo + 1);
            else
              tam = tam_tipo(tipo + 5);
            if ((tam == 2) || (tam == 4) || (tam == 8)) {
              izq = ultimo_nodo;
              if (tam == 2)
                crea_nodo(N_CONST, NULL, NULL, 1);
              else if (tam == 4)
                crea_nodo(N_CONST, NULL, NULL, 2);
              else if (tam == 8)
                crea_nodo(N_CONST, NULL, NULL, 3);
              crea_nodo(N_CD, izq, ultimo_nodo, 0);
            } else if (tam != 1) {
              izq = ultimo_nodo;
              crea_nodo(N_CONST, NULL, NULL, tam);
              crea_nodo(N_DIV, izq, ultimo_nodo, 0);
            }
            info[0] = t_int;
            continue;
          }
        }
        if (k = dobla(info[0], der)) {
          if (k == 2 && ultimo_nodo->oper == N_CONST)
            ultimo_nodo->esp *= multi;
          else if (k == 2 && ((multi == 2) || (multi == 4) || (multi == 8))) {
            if (multi == 2)
              crea_nodo(N_CONST, NULL, NULL, 1);
            else if (multi == 4)
              crea_nodo(N_CONST, NULL, NULL, 2);
            else if (multi == 8)
              crea_nodo(N_CONST, NULL, NULL, 3);
            crea_nodo(N_CI, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          } else if (k == 2 && multi != 1) {
            crea_nodo(N_CONST, NULL, NULL, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        }
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp - der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else
          crea_nodo(N_RESTA, izq, der, 0);
      }
    } else
      return 0;
  }
}

nivel12(info)
  int info[];
{
  int k, info2[1];
  struct nodo *izq, *der;
  unsigned char *tipo, *tipo2;

  k = nivel13(info);
  if ((encuentra("*") == 0) &&
      (encuentra("/") == 0) &&
      (encuentra("%") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("*")) {
      pos_linea++;
      if (nivel13(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_MULPF, izq, der, 0);
      } else {
        checa_entero(info[0]);
        checa_entero(info2[0]);
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp * der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else
          crea_nodo(N_MUL, izq, der, 0);
      }
    } else if (encuentra("/")) {
      pos_linea++;
      if (nivel13(info2))
        carga_valor(info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, info2)) {
        crea_nodo(N_DIVPF, izq, der, 0);
      } else {
        checa_entero(info[0]);
        checa_entero(info2[0]);
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp / der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else {
          tipo = info[0];
          tipo2 = info2[0];
          if ((*tipo == UINT) || (*tipo2 == UINT))
            crea_nodo(N_SDIV, izq, der, 0);
          else
            crea_nodo(N_DIV, izq, der, 0);
        }
      }
    } else if (encuentra("%")) {
      pos_linea++;
      checa_entero(info[0]);
      if (nivel13(info2))
        carga_valor(info2);
      checa_entero(info2[0]);
      der = ultimo_nodo;
      if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp % der->esp);
        libera_arbol(izq);
        libera_arbol(der);
      } else {
        tipo = info[0];
        tipo2 = info2[0];
        if ((*tipo == UINT) || (*tipo2 == UINT))
          crea_nodo(N_SMOD, izq, der, 0);
        else
          crea_nodo(N_MOD, izq, der, 0);
      }
    } else
      return 0;
  }
}

nivel13(info)
  int info[];
{
  int k;
  unsigned char *tipo, *tipo2;

  if (match("++")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13inc(info);
    return 0;
  } else if (match("--")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13dec(info);
    return 0;
  } else if (match("-")) {
    if (nivel13(info))
      carga_valor(info);
    checa_numerico(info[0]);
    tipo = info[0];
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = -ultimo_nodo->esp;
    else if (*tipo == DOUBLE || *tipo == FLOAT)
      crea_nodo(N_NEGPF, ultimo_nodo, NULL, 0);
    else
      crea_nodo(N_NEG, ultimo_nodo, NULL, 0);
    return 0;
  } else if (match("~")) {
    if (nivel13(info))
      carga_valor(info);
    checa_entero(info[0]);
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = ~ultimo_nodo->esp;
    else
      crea_nodo(N_COM, ultimo_nodo, NULL, 0);
    return 0;
  } else if (match("!")) {
    if (nivel13(info))
      carga_valor(info);
    checa_numerico(info[0]);
    tipo = info[0];
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = !ultimo_nodo->esp;
    else if (*tipo == DOUBLE || *tipo == FLOAT)
      compara_cero(tipo);
    else
      crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
    info[0] = t_int;
    return 0;
  } else if (match("*")) {
    return nivel13ap(info);
  } else if (match("&")) {
    return nivel13dir(info);
  } else if (amatch("sizeof", 6)) {
    if (match("(")) {
      if (p_tipo_1(NO)) {
        p_tipo_2(NULL);
        pide(")");
        crea_nodo(N_CONST, NULL, NULL, tam_tipo(tipo_proc));
        info[0] = t_int;
        return 0;
      } else
        k = primaria(info, SI);
    } else
      k = nivel13(info);
    crea_nodo(N_CONST, NULL, NULL, tam_tipo(info[0]));
    info[0] = t_int;
    return 0;
  } else if (match("(")) {
    if (p_tipo_1(NO)) {
      p_tipo_2(NULL);
      pide(")");
      tipo = tipo_proc;
      if (nivel13(info))
        carga_valor(info);
      tipo2 = info[0];
      info[0] = tipo;
      k = ultimo_nodo;
      convierte_tipo(&k, tipo2, tipo);
      return 0;
    } else k = primaria(info, SI);
  } else k = primaria(info, NO);
  if (match("++")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13pinc(info);
    return 0;
  }
  if (match("--")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info[0]);
    nivel13pdec(info);
    return 0;
  }
  return k;
}

nivel13ap(info)
  int info[];
{
  int k;
  unsigned char *ap;

  k = nivel13(info);
  if (k) {
    carga_valor(info);
    if (ultimo_nodo->oper == N_DIR)
      virtuales[ultimo_nodo->esp + 1]--;
  }
  ap = info[0];
  if (*ap == APUNTADOR)
    ap++;
  else if (*ap == MATRIZ)
    ap += 5;
  else
    error("No es un apuntador o matriz");
  info[0] = ap;
  if (*ap == FUNCION || *ap == MATRIZ || *ap == STRUCT)
    return 0;
  else
    return 1;
}

nivel13dir(info)
  int info[];
{
  unsigned char *tipo;

  if (nivel13(info) == 0) {
    tipo = info[0];
    if (*tipo != STRUCT)
      error("Dirección ilegal");
  }
  tipo = sig_tipo;
  guarda_tipo(APUNTADOR);
  copia_tipo(info[0]);
  if (ultimo_nodo->oper == N_DIR)
    virtuales[ultimo_nodo->esp + 1]++;
  info[0] = tipo;
  return 0;
}

nivel13inc(info)
  int info[];
{
  unsigned char *tipo;
  int inc;

  tipo = info[0];
  if (*tipo == APUNTADOR) {
    inc = tam_tipo(tipo + 1);
    tipo = t_int;
  } else
    inc = 1;
  crea_nodo(N_INC, ultimo_nodo, inc, *tipo);
}

nivel13dec(info)
  int info[];
{
  nivel13inc(info);
  ultimo_nodo->der = -(int) ultimo_nodo->der;
}

nivel13pinc(info)
  int info[];
{
  nivel13inc(info);
  ultimo_nodo->oper = N_PINC;
}

nivel13pdec(info)
  int info[];
{
  nivel13dec(info);
  ultimo_nodo->oper = N_PINC;
}

primaria(info, sin_parentesis)
  int info[], sin_parentesis;
{
  unsigned char *ap, nombre[TAM_NOMBRE];
  int k, tam, punto;
  struct nodo *izq, *der;
  unsigned char *tipo;
  int info2[1];

  if (sin_parentesis || match("(")) {
    k = nivel0(info);
    pide(")");
  } else if (nombre_legal(nombre)) {
    if ((ap = busca_loc(nombre))
     || (ap = busca_glb(nombre))) {
      if (ap[IDENT] == TYPEDEF
       || ap[IDENT] == ETIQUETA)
        error("No es una variable o función");
      if (ap[IDENT] != FUNCION) {
        if (ap[CLASE] == AUTO)
          dir_var_loc(ap);
        else
          dir_var_glb(ap);
      } else
        dir_func(ap);
      info[0] = tipo = lee_entero(ap + TIPO);
      if (*tipo == MATRIZ || *tipo == FUNCION || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if ((ap = busca_enum(nombre)) != NULL) {
      crea_nodo(N_CONST, NULL, NULL, lee_entero(ap + ENUM_VALOR));
      info[0] = t_int;
      k = 0;
    } else {
      dir_func(nueva_glb(nombre, FUNCION, STATIC, t_func, FUNC_REF));
      info[0] = t_func;
      k = 0;
    }
  } else if (constante(info)) {
    k = 0;
  } else {
    error("Expresión inválida");
    crea_nodo(N_CONST, NULL, NULL, 0);
    basura();
    info[0] = t_int;
    k = 0;
  }
  tipo = info[0];
  while (1) {
    if (match("[")) {
      if (*tipo != APUNTADOR && *tipo != MATRIZ) {
        error("No se puede usar subscripto");
        basura();
        pide("]");
        return 0;
      }
      if (k)
        carga_valor(info);
      izq = ultimo_nodo;
      if (nivel1(info2))
        carga_valor(info2);
      pide("]");
      if (*tipo == APUNTADOR)
        tam = tam_tipo(++tipo);
      else
        tam = tam_tipo(tipo += 5);
      der = ultimo_nodo;
      if (der->oper == N_CONST) {
        if (der->esp == 0) {
          libera_arbol(der);
          ultimo_nodo = izq;
        } else {
          der->esp *= tam;
          crea_nodo(N_SUMA, izq, der, 0);
        }
      } else if (tam == 8) {
        crea_nodo(N_CONST, NULL, NULL, 3);
        crea_nodo(N_CI, der, ultimo_nodo, 0);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      } else if (tam == 4) {
        crea_nodo(N_CONST, NULL, NULL, 2);
        crea_nodo(N_CI, der, ultimo_nodo, 0);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      } else if (tam == 2) {
        crea_nodo(N_CONST, NULL, NULL, 1);
        crea_nodo(N_CI, der, ultimo_nodo, 0);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      } else if (tam == 1) {
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      } else {
        crea_nodo(N_CONST, NULL, NULL, tam);
        crea_nodo(N_MUL, der, ultimo_nodo, 0);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      }
      info[0] = tipo;
      if (*tipo == MATRIZ || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if (match("(")) {
      if (k)
        carga_valor(info);
      if (*tipo != FUNCION)
        error("El tipo no es de función");
      else
        ++tipo;
      if (ultimo_nodo->oper == N_APFUNC) {
        izq = ultimo_nodo;
        llama_funcion(ultimo_nodo->esp, tipo);
        libera_arbol(izq);
      } else
        llama_funcion(0, tipo);
      info[0] = tipo;
      k = 0;
    } else {
      if (match("."))
        punto = 1;
      else if (match("->"))
        punto = 2;
      else
        punto = 0;
      if (punto) {
        if (nombre_legal(nombre) == 0)
          error("Nombre ilegal para el miembro");
        if (punto == 2) {
          if (k)
            carga_valor(info);
          if (*tipo == APUNTADOR)
            tipo++;
          else if (*tipo == MATRIZ)
            tipo += 5;
          else
            error("No es un apuntador o matriz");
          info[0] = tipo;
        }
        if (*tipo != STRUCT) {
          error("No es una estructura o unión");
          continue;
        }
        ap = lee_entero(tipo + 1);
        if (lee_entero(ap + EST_TAM) == 0) {
          error("Estructura o unión incompleta");
          continue;
        }
        ap = lee_entero(ap + EST_LISTA);
        while (ap != NULL) {
          if (astreq(nombre, ap + MIE_NOMBRE, MAX_NOMBRE)) {
            info[0] = tipo = lee_entero(ap + MIE_TIPO);
            if (*tipo == FUNCION || *tipo == MATRIZ || *tipo == STRUCT)
              k = 0;
            else
              k = 1;
            if (lee_entero(ap + MIE_POSICION)) {
              izq = ultimo_nodo;
              crea_nodo(N_CONST, NULL, NULL, lee_entero(ap + MIE_POSICION));
              crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
            }
            break;
          }
          ap = lee_entero(ap + MIE_SIG);
        }
        if (ap == NULL)
          error("Miembro indefinido");
      } else
        break;
    }
  }
  return k;
}

req_valorl()
{
  error("Debe ser un valor-l");
}

/*
** Compila una llamada a una función
**
** Invocada por "primaria", esta función llamará a la función
** nombrada o a una función indirecta.
*/
llama_funcion(ap, tipo_funcion)
  unsigned char *ap, *tipo_funcion;
{
  int info[1], tam;
  struct nodo *izq, *anterior = NULL, *primero = NULL;
  unsigned char *tipo;
  int registros = 0;

  espacios();                /* Ya ha sido tomado el parentesis inicial */
  if (ap == NULL)
    izq = ultimo_nodo;       /* Llamada indirecta */
  if (*tipo_funcion == STRUCT) {
    tam = (tam_tipo(tipo_funcion) + 3) / 4;
    crea_nodo(N_RESULTA, ultimo_nodo, tam, 0);
    primero = anterior = ultimo_nodo;
  }
  while (car_act != ')') {
    if (fin_sentencia())
      break;
    tam = 0;
    if (nivel1(info))
      carga_valor(info);     /* Obtiene un argumento */
    else {
      tipo = info[0];
      if (*tipo == STRUCT)
        tam = (tam_tipo(tipo) + 3) / 4;
    }
    tipo = info[0];
    if (*tipo == DOUBLE || *tipo == FLOAT) {
      crea_nodo(N_PARF, ultimo_nodo, tam, 0);
      registros = (registros + 1) & ~1;
    } else {
      crea_nodo(N_PAR, ultimo_nodo, tam, 0);
      if (*tipo != STRUCT)
        registros++;
    }
    if (primero == 0)
      primero = ultimo_nodo;
    if (anterior != 0)
      anterior->esp = ultimo_nodo;
    anterior = ultimo_nodo;
    if (match(",") == 0)
      break;
    espacios();
  }
  pide(")");
  if (ap == NULL)
    crea_nodo(N_FUNCI, primero, *tipo_funcion == STRUCT, izq);
  else
    crea_nodo(N_FUNC, primero, *tipo_funcion == STRUCT, ap);
  registros = (registros + 1) & ~1;
  if (registros > total_regs)
    total_regs = registros;
}

/*
** Carga el valor de una dirección de memoria.
*/
carga_valor(info)
  int info[];
{
  unsigned char *tipo;
  tipo = info[0];
  if (*tipo == CHAR) {
    crea_nodo(N_CBYTE, ultimo_nodo, NULL, 0);
  } else if (*tipo == SHORT) {
    crea_nodo(N_CSHORT, ultimo_nodo, NULL, 0);
  } else if (*tipo == USHORT) {
    crea_nodo(N_CUSHORT, ultimo_nodo, NULL, 0);
  } else if (*tipo == FLOAT)
    crea_nodo(N_CFLOAT, ultimo_nodo, NULL, 0);
  else if (*tipo == DOUBLE)
    crea_nodo(N_CDOUBLE, ultimo_nodo, NULL, 0);
  else if (*tipo == VOID)
    error("Tiene tipo void");
  else
    crea_nodo(N_CPAL, ultimo_nodo, NULL, 0);
}

/*
** Carga la dirección de una variable local
*/
dir_var_loc(var)
  unsigned char *var;
{
  crea_nodo(N_DIR, NULL, NULL, ((var[POSICION] & 255) +
                               ((var[POSICION + 1] & 255) << 8) +
                               ((var[POSICION + 2] & 255) << 16) +
                               ((var[POSICION + 3] & 255) << 24)));
}

/*
** Carga la dir. de una variable global.
*/
dir_var_glb(var)
  unsigned char *var;
{
  crea_nodo(N_DIRG, NULL, NULL, var);
}

/*
** Carga la dir. de una función.
*/
dir_func(ap)
  unsigned char *ap;
{
  crea_nodo(N_APFUNC, NULL, NULL, ap);
}

/*
** Checa si es necesario doblar para suma o resta con apuntadores.
*/
dobla(tipo, nodo)
  unsigned char *tipo;
  struct nodo *nodo;
{
  int cuanto;

  if ((*tipo != APUNTADOR) && (*tipo != MATRIZ))
    return 0;                         /* no es necesario */
  if (*tipo == APUNTADOR)
    cuanto = tam_tipo(tipo + 1);
  else
    cuanto = tam_tipo(tipo + 5);
  if (nodo->oper == N_CONST) {
    nodo->esp *= cuanto;              /* es una constante */
    return 1;
  }
  multi = cuanto;
  return 2;    /* optimizar segun sea suma o resta */
}

constante(info)
  int info[];
{
  int val[1], queonda;

  if (queonda = numero_real(val)) {
    if (queonda == 2)
      crea_nodo(N_CEROPF, NULL, NULL, 0);
    else
      crea_nodo(N_NUMPF, NULL, NULL, val[0]);
    info[0] = t_double;
  } else if (numero(val)) {
    crea_nodo(N_CONST, NULL, NULL, val[0]);
    info[0] = t_int;
  } else if (cad_caracteres(val)) {
    crea_nodo(N_CONST, NULL, NULL, val[0]);
    info[0] = t_int;
  } else if (cad_literal(val)) {
    crea_nodo(N_LIT, NULL, NULL, val[0]);
    info[0] = t_achar;
  } else
    return 0;
  return 1;
}

numero_real(val)
  int val[];
{
  double num, escala;
  unsigned char *comienzo, *codigo;
  int k, menos;

  comienzo = codigo = linea + pos_linea;
  k = menos = 1;
  while (k) {
    k = 0;
    if (*codigo == '+') {
      ++codigo;
      k = 1;
    }
    if (*codigo == '-') {
      ++codigo;
      k = 1;
      menos = -menos;
    }
  }
  while (isdigit(*codigo))
    ++codigo;
  if (*codigo != '.' && toupper(*codigo) != 'E')
    return 0;
  num = 0;
  if (*codigo == '.') {
    ++codigo;
    while (isdigit(*codigo))
      ++codigo;
    pos_linea = codigo - linea;
    while (*--codigo != '.')
      num = (num + (*codigo - '0')) / 10;
  } else
    pos_linea = codigo - linea;
  escala = 1;
  while (--codigo >= comienzo) {
    num += escala * (*codigo - '0');
    escala *= 10;
  }
  if (toupper(car_act) == 'E') {
    int neg, exp;

    obt_car();
    if (numero(&exp) == 0) {
      error("Exponente incorrecto");
      exp = 0;
    }
    if (exp < 0) {
      neg = 1;
      exp = -exp;
    } else
      neg = 0;
    k = 32;
    escala = 1;
    while (k) {
      escala *= escala;
      if (k & exp)
        escala *= 10;
      k >>= 1;
    }
    if (neg) num /= escala;
    else     num *= escala;
  }
  if (menos < 0)
    num = -num;
  if (num == 0)
    return 2;
  while (ap_lit & 3)
    lits[ap_lit++] = 0;
  if (ap_lit > (MAX_LITS - TAM_DOUBLE))
    error("Demasiadas constantes de punto flotante");
  val[0] = ap_lit;
  constan.valor = num;
  for (k = 0; k < TAM_DOUBLE; ++k)
    lits[ap_lit++] = constan.byte[k];
  return 1;
}

numero(val)
  int val[];
{
  int k, menos;
  unsigned char c;

  k = menos = 1;
  while (k) {
    k = 0;
    if (match("+"))
      k = 1;
    if (match("-")) {
      menos = -menos;
      k = 1;
    }
  }
  if (isdigit(car_act) == 0)
    return 0;
  if (car_act == '0') {
    while (car_act == '0') obt_car();
    if (toupper(car_act) == 'X') {
      obt_car();
      while (isxdigit(car_act)) {
        c = toupper(obt_car()) - '0';
        if (c > 9) c = c - 7;
        k = (k << 4) | c;
      }
    } else {
      while ((car_act >= '0') && (car_act <= '7'))
        k = k * 8 + (obt_car() - '0');
    }
  } else {
    while (isdigit(car_act))
      k = k * 10 + (obt_car() - '0');
  }
  if (toupper(car_act) == 'L')
    obt_car();
  if (menos < 0)
    k = -k;
  val[0] = k;
  return 1;
}

cad_caracteres(val)
  int val[];
{
  int k;

  k = 0;
  if (match("'") == 0)
    return 0;
  while (car_act != 39)
    k = (k << 8) + (caracter_literal() & 255);
  ++pos_linea;
  val[0] = k;
  return 1;
}

cad_literal(val)
  int val[];
{
  if (match("\"") == 0)
    return 0;
  val[0] = ap_lit;
  while (car_act != '"') {
    if (car_act == 0)
      break;
    if (ap_lit >= MAX_LITS) {
      error("Espacio de almacenamiento de cadenas agotado");
    while (match("\"") == 0)
      if (obt_car() == 0)
        break;
      return 1;
    }
    lits[ap_lit++] = caracter_literal();
  }
  obt_car();
  lits[ap_lit++] = 0;
  return 1;
}

caracter_literal()
{
  int i, oct = 0;

  if ((car_act != 92) || (prox_car() == 0))
    return obt_car();
  obt_car();
  if (car_act == 'n') {
    ++pos_linea;
    return 10;
  }
  if (car_act == 't') {
    ++pos_linea;
    return 9;
  }
  if (car_act == 'b') {
    ++pos_linea;
    return 8;
  }
  if (car_act == 'f') {
    ++pos_linea;
    return 12;
  }
  if (car_act == 'r') {
    ++pos_linea;
    return 13;
  }
  if (car_act == 'x') {
    ++pos_linea;
    while (isxdigit(car_act)) {
      i = toupper(obt_car()) - '0';
      if (i > 9) i = i - 7;
      oct = (oct << 4) | i;
    }
    return oct;
  }
  i = 3;
  while ((i-- > 0) && (car_act >= '0') && (car_act <= '7'))
    oct = (oct << 3) + obt_car() - '0';
  if (i == 2)
    return obt_car();
  else
    return oct;
}

/*
** Funciones de conversión y chequeo de tipos.
*/

checa_entero(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT)
    error("No es un tipo entero");
}

checa_numerico(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != DOUBLE
   && *tipo != FLOAT && *tipo != APUNTADOR)
    error("No es un tipo númerico");
}

checa_entero_o_apuntador(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != APUNTADOR)
    error("No es un tipo entero");
}

compara_no_cero(tipo)
  unsigned char *tipo;
{
  int izq;

  if (*tipo != DOUBLE && *tipo != FLOAT)
    return;
  izq = ultimo_nodo;
  crea_nodo(N_CEROPF, NULL, NULL, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
  crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
}

compara_cero(tipo)
  unsigned char *tipo;
{
  int izq;

  izq = ultimo_nodo;
  crea_nodo(N_CEROPF, NULL, NULL, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
}

convierte_tipo(nodo, tipo_original, nuevo_tipo)
  struct nodo **nodo;
  unsigned char *tipo_original, *nuevo_tipo;
{
  if (*tipo_original == STRUCT && *nuevo_tipo != STRUCT)
    error("No se puede convertir de estructura");
  else if (*tipo_original != STRUCT && *nuevo_tipo == STRUCT)
    error("No se puede convertir a estructura");
  else if (*tipo_original == VOID)
    error("No se puede convertir de void");
  else if (*tipo_original == APUNTADOR &&
          (*nuevo_tipo == DOUBLE || *nuevo_tipo == FLOAT))
    error("No se puede convertir un apuntador a real");
  else if (*nuevo_tipo == APUNTADOR &&
          (*tipo_original == DOUBLE || *tipo_original == FLOAT))
    error("No se puede convertir un real a apuntador");
  else {
    if ((*tipo_original == FLOAT || *tipo_original == DOUBLE)
     && (*nuevo_tipo == FLOAT || *nuevo_tipo == DOUBLE))
      return;
    if (*nuevo_tipo == FLOAT || *nuevo_tipo == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    }
  }
}

haz_compatible(nodo_izq, info_izq, nodo_der, info_der)
  struct nodo **nodo_izq, **nodo_der;
  int info_izq[], info_der[];
{
  unsigned char *tipo_izq, *tipo_der;

  tipo_izq = info_izq[0];
  tipo_der = info_der[0];
  if (*tipo_izq == STRUCT || *tipo_der == STRUCT)
    error("No se pueden efectuar operaciones con estructuras");
  if ((*tipo_izq == FLOAT || *tipo_izq == DOUBLE) &&
      (*tipo_der == FLOAT || *tipo_der == DOUBLE)) {
    info_izq[0] = t_double;
    return 1;
  }
  if (*tipo_izq == FLOAT || *tipo_izq == DOUBLE) {
    crea_nodo(N_ENTPF, *nodo_der, NULL, 0);
    *nodo_der = ultimo_nodo;
    info_izq[0] = t_double;
    return 1;
  }
  if (*tipo_der == FLOAT || *tipo_der == DOUBLE) {
    crea_nodo(N_ENTPF, *nodo_izq, NULL, 0);
    *nodo_izq = ultimo_nodo;
    info_izq[0] = t_double;
    return 1;
  }
  if ((*tipo_izq == UINT || *tipo_der == UINT)
  && (*tipo_izq != APUNTADOR && *tipo_der != APUNTADOR)
  && (*tipo_izq != MATRIZ && *tipo_der != MATRIZ))
    info_izq[0] = info_der[0] = t_uint;
  return 0;
}
/*
** Compilador de C para G11.
** Generador de Codigo.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 3 de junio de 1995.
** Revisión: 20 de julio de 1995. Optimación de sumas de constantes que son
**                                multiplos de 4.
** Revisión: 20 de julio de 1995. Mejor optimación de pos-incrementos.
** Revisión: 24 de julio de 1995. Nueva función, compara_y_salta().
** Revisión: 25 de julio de 1995. Generación de codigo para N_ANDB y N_ORB.
** Revisión: 25 de julio de 1995. Generación de codigo para N_TRI.
** Revisión: 25 de julio de 1995. Nueva función, salto_no_int().
** Revisión: 26 de julio de 1995. Generación de codigo para N_ASIGNA, N_AOR,
**                                N_AXOR, N_AAND, N_ACI, N_ACD, N_ASUMA,
**                                N_ARESTA, N_AMUL, N_ADIV, y N_AMOD.
** Revisión: 26 de julio de 1995. Optimación más amplia sobre N_PINC, N_INC,
**                                y los nodos de asignación, calculando
**                                con exactitud el número de registros usados.
** Revisión: 26 de julio de 1995. Generación de codigo para N_AIXP y
**                                optimación de N_ASUMA con constantes.
** Revisión: 27 de julio de 1995. Generación de codigo para N_COMA.
** Revisión: 28 de julio de 1995. Optimación mejor de N_PINC y N_INC.
** Revisión: 29 de julio de 1995. Corrección de un defecto en N_ASIGNA.
** Revisión: 11 de agosto de 1995. Soporte para short y unsigned short.
** Revisión: 11 de agosto de 1995. Nuevas funciones. almacena(), carga(),
**                                 libreria().
** Revisión: 5 de septiembre de 1995. Soporte para N_COPIA.
** Revisión: 5 de septiembre de 1995. Soporte para paso de estructuras cómo
**                                    parametros de funciones, y también
**                                    cómo resultados.
** Revisión: 5 de septiembre de 1995. Nuevas funciones. copia_resultado(),
**                                    y estructura().
** Revisión: 6 de septiembre de 1995. Nueva función. asigna().
** Revisión: 22 de septiembre de 1995. Generación de codigo para N_CFLOAT,
**                                     N_CDOUBLE, N_IGUALPF, N_MAYORPF,
**                                     N_SUMAPF, N_RESTAPF, N_MULPF, N_DIVPF,
**                                     N_CEROPF, N_ENTPF, N_PFENT, y N_IXF. 
** Revisión: 23 de septiembre de 1995. Generación de codigo para N_PARF y
**                                     N_NUMPF, nueva función. copia_reg().
** Revisión: 23 de septiembre de 1995. Simplifico algunas comparaciones.
** Revisión: 27 de septiembre de 1995. Añado el codigo faltante para cargar
**                                     short y unsigned short.
** Revisión: 27 de septiembre de 1995. Corrección de un defecto en el manejo
**                                     de operadores de asignación, manejaba
**                                     incorrectamente la estructura del
**                                     arbol.
** Revisión: 27 de septiembre de 1995. Optimación de N_PFENT y N_ENTPF.
** Revisión: 22 de noviembre de 1995. Generación de codigo final para N_NUMPF.
** Revisión: 24 de noviembre de 1995. Corrección de un defecto en la generación
**                                    de codigo para N_NUMPF.
** Revisión: 25 de noviembre de 1995. Corrección de un defecto en la función
**                                    gen_oper(), no generaba fprev sino rev.
** Revisión: 29 de noviembre de 1995. Se pasa la función nueva_etiq() al
**                                    archivo CCVARS.C
** Revisión: 29 de noviembre de 1995. Optimación de expresiones de punto
**                                    flotante, para evaluar en precisión
**                                    simple.
** Revisión: 30 de noviembre de 1995. Optimación extra de evaluación de
**                                    expresiones de punto flotante.
** Revisión: 1o. de diciembre de 1995. Generación de codigo para N_CEROF,
**                                     N_ENTF y N_CONVDF.
** Revisión: 2 de diciembre de 1995. Optimación de N_CEROPF-N_CONVDF a N_CEROF.
** Revisión: 9 de diciembre de 1995. Corrección de un defecto en la función
**                                   asigna(), no recibia el parametro de tipo.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en la generación
**                                    de codigo para unsigned int.
** Revisión: 8 de marzo de 1996. Pasa los argumentos argc y argv a main().
** Revisión: 8 de abril de 1996. Corrección de un defecto en paso de
**                               estructuras como argumentos.
** Revisión: 9 de abril de 1996. Corrección de defectos en manejo de
**                               estructuras.
** Revisión: 11 de abril de 1996. Corrección de un defecto tremendo en la
**                                generación de codigo para short y unsigned
**                                short.
** Revisión: 15 de abril de 1996. Corrección de defectos en manejo de
**                                estructuras.
** Revisión: 19 de abril de 1996. Corrección de un defecto en el paso de
**                                parametros reales a una función, cuando no
**                                era una expresión simple.
** Revisión: 20 de junio de 1996. Corrección de un defecto en la generación
**                                de la dirección para N_COPIA, cuando N_COPIA
**                                era descendiente de otro N_COPIA.
** Revisión: 7 de mayo de 1998. Los árboles de expresiones ahora son
**                              dinámicos, nueva función libera_arbol()
** Revisión: 8 de mayo de 1998. Nuevas funciones nueva_sentencia(),
**                              gen_funcion(), libera_sentencias() y
**                              funciones asociadas.
** Revisión: 8 de mayo de 1998. Empieza la conversión a G11, cambios enormes.
** Revisión: 12 de mayo de 1998. Termina la conversión a G11.
*/

/*
** Crea una variable virtual
*/
var_virtual(def, tipo)
  int def, tipo;
{
  int numero = variables_virtuales;

  if (variables_virtuales >= MAX_VIRTUALES)
    error("Demasiadas variables");
  virtuales[variables_virtuales++] = def;   /* Estado actual */
  virtuales[variables_virtuales++] = 0;     /* ¿Se pide su dirección? */
  virtuales[variables_virtuales++] = tipo;  /* Tipo de variable */
                                            /* 0= Int, 1= Double */
  return numero;
}

/*
** Nueva sentencia
*/
nueva_sentencia(tipo)
  enum tipo_sentencia tipo;
{
  struct sentencia *temp;

  temp = malloc(sizeof(struct sentencia));
  temp->sig = NULL;
  temp->tipo = tipo;
  return temp;
}

/*
** Libera una lista de sentencias
*/
libera_sentencias(lista)
  struct sentencia *lista;
{
  struct sentencia *enlace, *temp;

  enlace = lista;
  while (enlace != NULL) {
    switch (enlace->tipo) {
      case t_if      : libera_sentencias(enlace->def.t_if.lista1);
                       libera_sentencias(enlace->def.t_if.lista2);
                       libera_arbol(enlace->def.t_if.expresion);
                       break;

      case t_while   :
      case t_do      :
      case t_switch  : libera_sentencias(enlace->def.t_while.lista);
                       libera_arbol(enlace->def.t_while.expresion);
                       break;

      case t_for     : libera_sentencias(enlace->def.t_for.lista);
                       libera_arbol(enlace->def.t_for.expresion1);
                       libera_arbol(enlace->def.t_for.expresion2);
                       libera_arbol(enlace->def.t_for.expresion3);
                       break;

      case t_case    :
      case t_default :
      case t_goto    :
      case t_etiqueta:
      case t_continue:
      case t_break   : break;

      case t_return  : libera_arbol(enlace->def.t_return.expresion);
                       break;

      case t_expresion:libera_arbol(enlace->def.t_expresion.expresion);
                       break;

      default        : error("Error interno del compilador");
                       break;
    }
    temp = enlace->sig;
    free(enlace);
    enlace = temp;
  }
}

/*
** Genera el codigo para una función.
*/
gen_funcion(n_func, lista)
  char *n_func;
  struct sentencia *lista;
{
  pila = 0;                  /* Desplazamiento de la pila de memoria */
  pila_regs = 0;             /* Desplazamiento de la pila de registros */
  libera_temporales();       /* Libera los registros temporales */
  vacia_buffer();            /* Vacia todo del buffer */
  emite_nombre(n_func);      /* Imprime el nombre de la función */
  dos_puntos();
  emite_nueva_linea();
  prologo_funcion();         /* Emite el prologo de la función */
  ultima_sentencia = t_expresion;
  gen_sentencias(lista);     /* Genera el codigo */
  epilogo_funcion();         /* Emite el epilogo de la función */
}

/*
** Libera los registros temporales (gr98-gr111)
*/
libera_temporales()
{
  int a;

  temporales[0] = 1;    /* Mantenemos ocupados gr96 y gr97, */
  temporales[1] = 1;    /* solo son liberados para cargar el resultado */
  for (a = 2; a < 16; a++)
    temporales[a] = 0;
  pila_temporal = 0;    /* Nada temporal en pila de memoria */
}

/*
** Prologo de función:
**
** o Asigna las variables virtuales a los registros o a la memoria.
** o Asigna el espacio requerido.
** o Copia los argumentos de la entrada (si es requerido)
*/
prologo_funcion()
{
  int variable, temp, por_copiar = 0, posicion, registro;

/*
** Asignamos los registros (por el momento no se sabe si van a ser locales
** o globales), también asignamos espacio en la pila pero aún falta
** determinar si va a ser corrida para hacer espacio a argumentos que
** deben ser copiados.
*/
  variable = 0;
  while (variable < variables_virtuales) {
    switch (virtuales[variable] & 3) {
      case 0:   /* Variable para asignar como se pueda */
        if (virtuales[variable + 1] != 0) {  /* ¿ Necesita apuntador ? */
          virtuales[variable] = (pila << 2) | 1;
          pila += virtuales[variable + 2] ? 8 : 4;
        } else {                             /* No, queda en registro */
          if (virtuales[variable + 2])       /* Alinea punto flotante */
            pila_regs = (pila_regs + 1) & ~1;
          virtuales[variable] = pila_regs << 2;
          pila_regs += virtuales[variable + 2] ? 2 : 1;
        }
        virtuales[variable + 1] = 0;
        break;
      case 1:   /* Variable que debe quedar en memoria */
        temp = virtuales[variable] >> 2;
        virtuales[variable] = (pila << 2) | 1;
        pila += temp;
        virtuales[variable + 1] = 0;
        break;
      case 2:   /* Cálcular cuantos argumentos debemos copiar */
        if (virtuales[variable + 1] != 0)    /* ¿ Necesita copiar ? */
          por_copiar += virtuales[variable + 2] ? 8 : 4;
        break;
    }
    variable += 3;
  }
/*
** Corremos la pila para hacer espacio a los argumentos que deben copiarse,
** también copiamos los argumentos y pre-asignamos registros a los args.
*/
  pila += por_copiar;
  variable = 0;
  while (variable < variables_virtuales) {
    switch (virtuales[variable] & 3) {
      case 1:   /* Variable que debe quedar en memoria */
        virtuales[variable] = (((virtuales[variable] >> 2) +
                                por_copiar) << 2) | 1;
        break;
    }
    variable += 3;
  }
  if (pila != 0)
    gen_inst1("sub", SI, 125, 125, pila);
  pila_regs = (pila_regs + 1) & ~1;
  posicion = 0;
  variable = 0;
  while (variable < variables_virtuales) {
    switch (virtuales[variable] & 3) {
      case 2:   /* Copiamos los argumentos requeridos */
        if (virtuales[variable + 1] != 0) {
          virtuales[variable + 1] = 0;
          registro = virtuales[variable] >> 2;
          virtuales[variable] = (posicion << 2) | 1;
          if (posicion == 0) {
            gen_inst2("store 0,4,", NO, registro + 128, 125);
            posicion += 4;
            if (virtuales[variable + 2]) {
              gen_inst1("add", SI, 96, 125, posicion);
              gen_inst2("store 0,4,", NO, registro + 128, 96);
              posicion += 4;
            }
          } else {
            gen_inst1("add", SI, 96, 125, posicion);
            gen_inst2("store 0,4,", NO, registro + 128, 96);
            posicion += 4;
            if (virtuales[variable + 2]) {
              gen_inst1("add", SI, 96, 96, 4);
              gen_inst2("store 0,4,", NO, registro + 129, 96);
              posicion += 4;
            }
          }
        } else {
          if (total_regs == -1 && pila_regs <= 4)
            temp = 128;
          else if (total_regs == -1)
            temp = 130 + pila_regs;
          else
            temp = 130 + total_regs + pila_regs;
          virtuales[variable] = (((virtuales[variable] >> 2) + temp)
                                 << 2) | 2;
        }
        break;
      case 3:    /* Ajustamos los argumentos que vienen en memoria */
        virtuales[variable + 1] = 0;
        virtuales[variable] = (((virtuales[variable] >> 2) + pila) << 2) | 1;
        break;
    }
    variable += 3;
  }
  if (total_regs == -1 &&    /* Si no se llama ninguna función y solo hay */
      pila_regs <= 4) {      /* 4 registros utilizados o menos, */
    pila_regs = 0;           /* No nos hace falta la pila de registros */
    variable = 0;
    while (variable < variables_virtuales) {
      switch (virtuales[variable] & 3) {
        case 0:    /* Asignar registros gr116 - gr119 */
          virtuales[variable] = ((virtuales[variable] >> 2) + 116) << 2;
          break;
        case 2:    /* Los parametros siguen en locales */
          virtuales[variable] &= ~3;
          break;
      }
      variable += 3;
    }
  } else {                   /* Pedimos espacio en la pila de registros */
    variable = 0;
    while (variable < variables_virtuales) {
      switch (virtuales[variable] & 3) {
        case 0:    /* Asignar registros locales */
          virtuales[variable] = ((virtuales[variable] >> 2) +
                                  total_regs + 130) << 2;
          break;
        case 2:    /* Los parametros ya tienen sus posiciones */
          virtuales[variable] &= ~3;
          break;
      }
      variable += 3;
    }
    pila_regs += total_regs;
    pila_regs += 2;
    if (pila_regs > 128)
      error("Demasiadas variables locales");
    else if (pila_regs + pila_args > 508)
      error("Demasiados argumentos");
    gen_inst1("sub", SI, 1, 1, pila_regs << 2);
    emite_linea("asgeu 64,gr1,gr126");
    gen_inst1("add", SI, 129, 1, (pila_regs + pila_args) << 2);
  }
}

epilogo_funcion()
{
  if (buffer_vacio)
    return;
  if (pila_regs != 0) {
    gen_inst1("add", SI, 1, 1, pila_regs << 2);
    if (pila == 0)
      gen_libre(0);
    else
      gen_inst1("add", SI, 125, 125, pila);
    emite_linea("jmpi lr0");
    emite_linea("asleu 65,lr1,gr127");
  } else {
    if (pila != 0)
      gen_inst1("add", SI, 125, 125, pila);
    estado_buf[total_lineas] = 10;
    emite_linea("jmpi \1\1\1\1\1\1\1lr0");
    gen_libre(1);
  }
  vacia_buffer();
}

/*
** Genera codigo para una lista de sentencias
*/
gen_sentencias(lista)
  struct sentencia *lista;
{
  struct sentencia *codigo;

  codigo = lista;
  while (codigo != NULL) {
    switch (codigo->tipo) {
      case t_if      : gen_if(codigo);
                       break;
      case t_while   : gen_while(codigo);
                       break;
      case t_do      : gen_do(codigo);
                       break;
      case t_for     : gen_for(codigo);
                       break;
      case t_switch  : gen_switch(codigo);
                       break;
      case t_case    : gen_case(codigo);
                       break;
      case t_default :
      case t_etiqueta: gen_etiqueta(codigo);
                       break;
      case t_break   :
      case t_continue:
      case t_goto    : gen_break(codigo);
                       break;
      case t_return  : gen_return(codigo);
                       break;
      case t_expresion:gen_expresion(codigo);
                       break;
      default        : error("Error interno del compilador");
                       break;
    }
    ultima_sentencia = codigo->tipo;
    codigo = codigo->sig;
  }
}

gen_if(codigo)
  struct sentencia *codigo;
{
  struct nodo *expr;
  int etiq1, etiq2;

  expr = codigo->def.t_if.expresion;
  if (expr->oper == N_CONST && expr->esp == 0) {
    if (codigo->def.t_if.lista2 != NULL)
      gen_sentencias(codigo->def.t_if.lista2);
    return;
  } else if (expr->oper == N_CONST && expr->esp != 0) {
    if (codigo->def.t_if.lista1 != NULL)
      gen_sentencias(codigo->def.t_if.lista1);
    return;
  }
  etiq1 = nueva_etiq;
  prueba(etiq1, expr);                 /* Checa la expresión */
  gen_sentencias(codigo->def.t_if.lista1);
  if (codigo->def.t_if.lista2 == NULL) {   /* ¿ Es IF..ELSE ? */
    gen_destino(etiq1);
    return;
  }
  if ((ultima_sentencia != t_return) &&
      (ultima_sentencia != t_break) &&
      (ultima_sentencia != t_continue) &&
      (ultima_sentencia != t_goto))
    salto(etiq2 = nueva_etiq);  /* Salta alrededor del codigo de else */
  else
    etiq2 = 0;
  gen_destino(etiq1);
  gen_sentencias(codigo->def.t_if.lista2);  /* Codigo ELSE */
  gen_destino(etiq2);
}

gen_while(codigo)
  struct sentencia *codigo;
{
  struct nodo *expr;

  expr = codigo->def.t_while.expresion;
  if (expr->oper == N_CONST && expr->esp == 0)
    return;
  gen_destino(codigo->def.t_while.etiqueta_continue); /* Etiqueta del bucle */
  if (expr->oper == N_CONST)
    gen_sentencias(codigo->def.t_while.lista);
  else if (codigo->def.t_while.lista != NULL) {
    prueba(codigo->def.t_while.etiqueta_break, expr); /* Checa la expresión */
    gen_sentencias(codigo->def.t_while.lista);
  } else {
    crea_nodo(N_NOT, expr, NULL, 0);
    codigo->def.t_while.expresion = expr = ultimo_nodo;
    prueba(codigo->def.t_while.etiqueta_continue, expr);
  }
  if ((codigo->def.t_while.lista != NULL || expr->oper == N_CONST) &&
      (ultima_sentencia != t_return) &&
      (ultima_sentencia != t_break) &&
      (ultima_sentencia != t_continue) &&
      (ultima_sentencia != t_goto))
    salto(codigo->def.t_while.etiqueta_continue);     /* Vuelve al bucle */
  if (codigo->def.t_while.lista != NULL)
    gen_destino(codigo->def.t_while.etiqueta_break); /* Etiqueta de salida */
}

gen_do(codigo)
  struct sentencia *codigo;
{
  struct nodo *expr;

  expr = codigo->def.t_while.expresion;
  gen_destino(codigo->def.t_while.etiqueta_continue); /* Etiqueta del bucle */
  gen_sentencias(codigo->def.t_while.lista);       /* Procesa una sentencia */
  if (expr->oper != N_CONST)
    prueba(codigo->def.t_while.etiqueta_break, expr);   /* Checa la expresión */
  if (expr->oper != N_CONST || (expr->oper == N_CONST && expr->esp != 0))
    salto(codigo->def.t_while.etiqueta_continue);
  gen_destino(codigo->def.t_while.etiqueta_break);   /* Etiqueta de salida */
}

gen_for(codigo)
  struct sentencia *codigo;
{
  struct nodo *expr;
  int etiq = nueva_etiq;

  usa_expr = NO;
  etiqueta(codigo->def.t_for.expresion1);
  gen_codigo(0, codigo->def.t_for.expresion1);
  expr = codigo->def.t_for.expresion2;
  gen_destino(etiq);
  if (expr != NULL) {
    if (expr->oper == N_CONST && expr->esp == 0)
      return;
    else if (expr->oper != N_CONST)
      prueba(codigo->def.t_for.etiqueta_break, expr); /* Checa la expresión */
  }
  gen_sentencias(codigo->def.t_for.lista);
  usa_expr = NO;
  gen_destino(codigo->def.t_for.etiqueta_continue);
  etiqueta(codigo->def.t_for.expresion3);
  gen_codigo(0, codigo->def.t_for.expresion3);
  salto(etiq);
  gen_destino(codigo->def.t_for.etiqueta_break);
}

gen_switch(codigo)
  struct sentencia *codigo;
{
  int etiq_default = 0;
  struct sentencia *analisis;

  usa_expr = SI;
  etiqueta(codigo->def.t_while.expresion);
  gen_codigo(0, codigo->def.t_while.expresion);
  analisis = codigo->def.t_while.lista;
  while (analisis != NULL) {
    if (analisis->tipo == t_case)
      compara_y_salta(analisis->def.t_case.constante, analisis->def.t_case.etiqueta);
    else if (analisis->tipo == t_default)
      etiq_default = analisis->def.t_break.etiqueta;
    analisis = analisis->sig;
  }
  if (etiq_default)
    salto(etiq_default);
  else
    salto(codigo->def.t_while.etiqueta_break);
  gen_sentencias(codigo->def.t_while.lista);
  gen_destino(codigo->def.t_while.etiqueta_break);
}

gen_case(codigo)
  struct sentencia *codigo;
{
  gen_destino(codigo->def.t_case.etiqueta);
}

gen_etiqueta(codigo)
  struct sentencia *codigo;
{
  gen_destino(codigo->def.t_break.etiqueta);
}

gen_break(codigo)
  struct sentencia *codigo;
{
  salto(codigo->def.t_break.etiqueta);
}

gen_return(codigo)
  struct sentencia *codigo;
{
  usa_expr = SI;
  if (codigo->def.t_return.informacion) {
    copia_resultado(&codigo->def.t_return.expresion,
                     codigo->def.t_return.informacion);
  }
  etiqueta(codigo->def.t_return.expresion);
  gen_codigo(0, codigo->def.t_return.expresion);
  epilogo_funcion();
}

gen_expresion(codigo)
  struct sentencia *codigo;
{
  usa_expr = NO;
  etiqueta(codigo->def.t_expresion.expresion);
  gen_codigo(0, codigo->def.t_expresion.expresion);
}

/*
** Prueba si la expresión es cero y salta.
*/
prueba(etiq, nodo)
  int etiq;
  struct nodo *nodo;
{
  usa_expr = SI;
  if (nodo->oper == N_NOT)
    if (nodo->izq->oper == N_NOT)
      nodo = nodo->izq->izq;
  etiqueta(nodo);
  gen_codigo(etiq, nodo);
}

int es_comparacion(nodo)
  struct nodo *nodo;
{
  return (nodo->oper >= N_IGUAL && nodo->oper <= N_MAYORIPF);
}

/*
** Genera codigo para un árbol de expresiones
*/
gen_codigo(etiq, expr)
  int etiq;
  struct nodo *expr;
{
  if (expr == NULL)
    return;
  if (etiq)
    salta_expr(expr, etiq, NO);
  else
    gen_nodo(expr, usa_expr ? 96 : 0);
}

/*
** Libera un arbol de expresiones.
*/
libera_arbol(nodo)
  struct nodo *nodo;
{
  int op;

  if (nodo == NULL)
    return;
  op = nodo->oper;
  if (nodo->izq != NULL)
    libera_arbol(nodo->izq);
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA) && (op != N_FUNC)
   && (op != N_FUNCI) && (op != N_PAR) && (op != N_PARF)
   && (nodo->der != NULL))
    libera_arbol(nodo->der);
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if (nodo->esp != NULL)
      libera_arbol(nodo->esp);
  free(nodo);
}

/*
** Crea un nodo del arbol de expresiones.
*/
crea_nodo(op, izq, der, val)
  int op, val;
  struct nodo *izq, *der;
{
  if (op == N_SUMA && der->oper == N_CONST
  && izq->oper == N_SUMA && izq->der->oper == N_CONST) {
    izq->der->esp += der->esp;
    libera_arbol(der);
    ultimo_nodo = izq;
    return;
  }
  ultimo_nodo = malloc(sizeof(struct nodo));
  if (ultimo_nodo == NULL) {
    error("Expresión muy compleja");
    cancela();
  }
  ultimo_nodo->izq = izq;
  ultimo_nodo->der = der;
  ultimo_nodo->oper = op;
  ultimo_nodo->esp = val;
  ultimo_nodo->regs = 0;
}

/*
** Etiqueta un arbol.
**
** Cada nodo es etiquetado con el número de registros
** que requiere para evaluarse.
*/
etiqueta(nodo)
  struct nodo *nodo;
{
  int min, max, op;
  struct nodo *temp;

  if (nodo == NULL)
    return;
  op = nodo->oper;
  if (nodo->izq != NULL)
    etiqueta(nodo->izq);
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA) && (op != N_FUNC)
   && (op != N_FUNCI) && (op != N_PAR) && (op != N_PARF)
   && (nodo->der != NULL))
    etiqueta(nodo->der);
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if (nodo->esp != NULL)
      etiqueta(nodo->esp);
  if ((op == N_FUNCI) || (op == N_FUNC) ||
      (op == N_ANDB) || (op == N_ORB) ||
      (op == N_COMA)) {
    nodo->regs = 1000;
  } else if (op == N_TRI) {
    temp = nodo->esp;
    nodo->regs = nodo->izq->regs + nodo->der->regs + temp->regs;
  } else if (op == N_COPIA) {
    nodo->regs = nodo->izq->regs + nodo->der->regs + 1;
  } else if (op >= N_AOR && op <= N_ASIGNA) {
    min = nodo->izq->regs;
    max = nodo->der->regs;
    if (en_registro(nodo->der))
      max = 0;
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    nodo->regs = max;
  } else if ((op == N_INC) || (op == N_PINC)) {
    max = nodo->izq->regs;
    if (en_registro(nodo->izq))
      nodo->izq->regs = max = 0;
    nodo->regs = max;
  } else if (op == N_CPAL && nodo->izq->oper == N_DIR
          && en_registro(nodo->izq)) {
    nodo->regs = 0;
  } else if (nodo->izq == NULL) {
    nodo->regs = 1;
  } else if (nodo->der == NULL || op == N_PAR || op == N_PARF
          || op == N_RESULTA) {
    nodo->regs = nodo->izq->regs;
    if (op == N_PFENT)
      nodo->regs++;
  } else {
    min = nodo->izq->regs;
    max = nodo->der->regs;
    if (op >= N_OR && op <= N_RESTA) {
      if (es_constante(nodo->der))
        nodo->der->regs = max = 0;
      else if (op != N_CD && op != N_CI && es_constante(nodo->izq)) {
        temp = nodo->izq;
        nodo->izq = nodo->der;
        nodo->der = temp;
        min = max;
        nodo->der->regs = max = 0;
        if (op == N_MAYOR)
          nodo->oper = N_MENORI;
        else if (op == N_MAYORI)
          nodo->oper = N_MENOR;
        else if (op == N_MENOR)
          nodo->oper = N_MAYORI;
        else if (op == N_MENORI)
          nodo->oper = N_MAYOR;
        else if (op == N_SMAYOR)
          nodo->oper = N_SMENORI;
        else if (op == N_SMAYORI)
          nodo->oper = N_SMENOR;
        else if (op == N_SMENOR)
          nodo->oper = N_SMAYORI;
        else if (op == N_SMENORI)
          nodo->oper = N_SMAYOR;
        else if (op == N_RESTA)
          nodo->oper = N_RESTAI;
      }
    }
    if (min > max)
      max = min;
    else if (min == max)
      max++;
    nodo->regs = max;
  }
}

/*
** Averigua si la variable esta en un registro.
*/
int en_registro(nodo)
  struct nodo *nodo;
{
  if (nodo->oper != N_DIR)
    return 0;
  if (virtuales[nodo->esp] & 3)
    return 0;
  return virtuales[nodo->esp + 2] + 1;
}

/*
** Obtiene el registro de un nodo
*/
int registro(nodo)
  struct nodo *nodo;
{
  return virtuales[nodo->esp] >> 2;
}

/*
** Averigua si hay una constante pequeña
*/
int es_constante(nodo)
  struct nodo *nodo;
{
  if (nodo->oper != N_CONST)
    return 0;
  return (nodo->esp >= 0 && nodo->esp <= 255);
}

/*
** Codigo para cada operador binario, y algunos unarios.
*/
gen_oper(oper, inmediato, reg1, reg2, constreg, control)
  int oper, inmediato, reg1, reg2, constreg, control;
{
  int reg;

  if (oper == N_OR || oper == N_AOR) {
    gen_inst1("or", inmediato, reg1, reg2, constreg);
  } else if (oper == N_XOR || oper == N_AXOR) {
    gen_inst1("xor", inmediato, reg1, reg2, constreg);
  } else if (oper == N_AND || oper == N_AAND) {
    gen_inst1("and", inmediato, reg1, reg2, constreg);
  } else if (oper == N_CD || oper == N_ACD) {
    gen_inst1("sra", inmediato, reg1, reg2, constreg);
  } else if (oper == N_CI || oper == N_ACI) {
    gen_inst1("sll", inmediato, reg1, reg2, constreg);
  } else if (oper == N_SUMA || oper == N_ASUMA) {
    gen_inst1("add", inmediato, reg1, reg2, constreg);
  } else if (oper == N_RESTA || oper == N_ARESTA) {
    gen_inst1("sub", inmediato, reg1, reg2, constreg);
  } else if (oper == N_MUL || oper == N_AMUL) {
    gen_inst1("multiply", NO, reg1, reg2, constreg);
  } else if (oper == N_DIV || oper == N_ADIV) {
    reg = pedir_reg(NO);
    gen_inst1("sra", SI, reg, reg2, 31);
    emite_texto("mtsr q,");
    emite_registro(reg);
    emite_nueva_linea();
    gen_inst1("divide", NO, reg1, reg2, constreg);
    libera_reg(reg);
  } else if (oper == N_SDIV) {
    emite_linea("mtsrim q,0");
    gen_inst1("dividu", NO, reg1, reg2, constreg);
  } else if (oper == N_MOD || oper == N_AMOD) {
    reg = pedir_reg(NO);
    gen_inst1("sra", SI, reg, reg2, 31);
    emite_texto("mtsr q,");
    emite_registro(reg);
    emite_nueva_linea();
    gen_inst1("divide", NO, reg1, reg2, constreg);
    emite_texto("mfsr ");
    emite_registro(reg1);
    emite_linea(",q");
    libera_reg(reg);
  } else if (oper == N_SMOD) {
    emite_linea("mtsrim q,0");
    gen_inst1("dividu", NO, reg1, reg2, constreg);
    emite_texto("mfsr ");
    emite_registro(reg1);
    emite_linea(",q");
  } else if (oper == N_NOT) {
    gen_inst1("cpeq", SI, reg1, reg1, 0);
    if (!control)
      gen_inst1("srl", SI, reg1, reg1, 31);
  } else if (oper == N_NEG) {
    gen_inst1("subr", SI, reg1, reg1, 0);
  } else if (oper == N_NEGPF) {
    reg = pedir_reg(NO);
    gen_inst3("const", SI, reg, 1);
    gen_inst1("sll", SI, reg, reg, 31);
    gen_inst1("xor", SI, reg1, reg1, reg);
    libera_reg(reg);
  } else if (oper == N_COM)
    gen_inst1("nand", NO, reg1, reg1, reg1);
  else if (oper == N_CPAL)
    gen_inst2("load 0,4,", NO, reg1, reg1);
  else if (oper == N_CBYTE) {
    gen_inst2("load 0,20,", NO, reg1, reg1);
    gen_inst1("exbyte", SI, reg1, reg1, 0);
  } else if (oper == N_CSHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg1);
    gen_inst3("exhws", NO, reg1, reg1);
  } else if (oper == N_CUSHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg1);
    gen_inst1("exhw", SI, reg1, reg1, 0);
  } else if (oper == N_CFLOAT) {
    gen_inst2("load 0,4,", NO, reg1, reg1);
    gen_inst4(reg1, reg1, "2,1");
  } else if (oper == N_CDOUBLE) {
    gen_inst1("add", SI, reg1 + 1, reg1, 4);
    gen_inst2("load 0,4,", NO, reg1 + 1, reg1 + 1);
    gen_inst2("load 0,4,", NO, reg1, reg1);
  } else if (oper == N_SUMAPF) {
    gen_inst1("dadd", NO, reg1, reg2, constreg);
  } else if (oper == N_RESTAPF) {
    gen_inst1("dsub", NO, reg1, reg2, constreg);
  } else if (oper == N_MULPF) {
    gen_inst1("dmul", NO, reg1, reg2, constreg);
  } else if (oper == N_DIVPF) {
    gen_inst1("ddiv", NO, reg1, reg2, constreg);
  } else if (oper == N_ENTPF) {
    gen_inst4(reg1, reg1, "2,0");
  } else if (oper == N_PFENT) {
    gen_inst4(reg1, reg1, "0,2");
  } else if (oper == N_IGUALPF) {
    gen_inst1("deq", NO, reg1, reg2, constreg);
  } else if (oper == N_MAYORPF) {
    gen_inst1("dgt", NO, reg1, reg2, constreg);
  } else if (oper == N_MAYORIPF) {
    gen_inst1("dge", NO, reg1, reg2, constreg);
  } else if (oper == N_IGUAL) {
    gen_inst1("cpeq", inmediato, reg1, reg2, constreg);
  } else if (oper == N_NOIGUAL) {
    gen_inst1("cpneq", inmediato, reg1, reg2, constreg);
  } else if (oper == N_MAYOR) {
    gen_inst1("cpgt", inmediato, reg1, reg2, constreg);
  } else if (oper == N_MAYORI) {
    gen_inst1("cpge", inmediato, reg1, reg2, constreg);
  } else if (oper == N_MENOR) {
    gen_inst1("cplt", inmediato, reg1, reg2, constreg);
  } else if (oper == N_MENORI) {
    gen_inst1("cple", inmediato, reg1, reg2, constreg);
  } else if (oper == N_SMAYOR) {
    gen_inst1("cpgtu", inmediato, reg1, reg2, constreg);
  } else if (oper == N_SMAYORI) {
    gen_inst1("cpgeu", inmediato, reg1, reg2, constreg);
  } else if (oper == N_SMENOR) {
    gen_inst1("cpltu", inmediato, reg1, reg2, constreg);
  } else if (oper == N_SMENORI) {
    gen_inst1("cpleu", inmediato, reg1, reg2, constreg);
  }
  if (oper >= N_IGUAL && oper <= N_MAYORIPF)
    if (!control)
      gen_inst1("srl", SI, reg1, reg1, 31);
}

pedir_reg(pareja)
  int pareja;
{
  int num;

  if (!pareja) {
    for (num = 0; num < 16; num++) {
      if (temporales[num] == 0) {
        temporales[num] = 1;
        return num + 96;
      }
    }
    error("Error interno en registros temporales");
  } else {
    for (num = 0; num < 16; num += 2) {
      if (temporales[num] == 0 && temporales[num + 1] == 0) {
        temporales[num] = 2;
        temporales[num + 1] = 2;
        return num + 96;
      }
    }
    error("Error interno en registros temporales");
  }
}

libera_reg(registro)
  int registro;
{
  if (registro < 98 || registro > 111)
    return;
  registro -= 96;
  if (temporales[registro] == 1) {
    temporales[registro] = 0;
    return;
  }
  if (temporales[registro] == 2) {
    temporales[registro] = 0;
    temporales[registro + 1] = 0;
    return;
  }
  error("Liberando registro temporal incorrecto");
}

/*
** Genera codigo para el nodo del arbol.
**
** Se le puede indicar el registro temporal donde debe caer el resultado,
** un 0 indica que el resultado no es necesario.
*/
gen_nodo(nodo, resultado)
  struct nodo *nodo;
  int resultado;
{
  int op, total, cuenta, pila_extra, tam_resultado, pos_pila, anota_pila;
  int dir_complicada, regresa_struct, reg, temp, reg1, reg2, reg3, tam;
  int real, etiq;
  struct nodo *nodo_temp;

  op = nodo->oper;
  if ((op == N_FUNC) || (op == N_FUNCI)) {
    /*
    ** Paso 1: Si es una función indirecta puede tener una dirección
    **         complicada o simple.
    */
    if (op == N_FUNCI) {
      nodo_temp = nodo->esp;
      dir_complicada = nodo_temp->regs >= 12;
    }

    /*
    ** Paso 2: Hacemos una lista de los argumentos complicados de evaluar
    **         Cálculamos espacio extra en pila para estructuras como
    **         argumentos o como resultados, el espacio se pide antes
    **         de evaluar cualquier otro argumento.
    */
    pila_extra = 0;
    total = 0;
    tam_resultado = 0;
    nodo_temp = nodo->izq;
    regresa_struct = NO;
    while (nodo_temp != NULL) {
      if (nodo_temp->oper == N_RESULTA) {
        regresa_struct = SI;
        pila_extra += tam_resultado = (int) nodo_temp->der * 4;
      } else if (nodo_temp->oper == N_PAR || nodo_temp->oper == N_PARF) {
        if (total == MAX_ARGS)
          error("Demasiados argumentos en función");
        if (nodo_temp->der != 0)
          pila_extra += (int) nodo_temp->der * 4;
        else if (nodo_temp->regs >= 12)
          total++;
      }
      nodo_temp = nodo_temp->esp;
    }

    /*
    ** Paso 3: Asignamos el espacio extra en pila de memoria y copiamos
    **         los argumentos de estructura.
    */
    if (pila_extra) {
      gen_inst1("sub", SI, 125, 125, pila_extra);
      pila_temporal += pila_extra;
    }
    pos_pila = tam_resultado - pila_temporal;
    nodo_temp = nodo->izq;
    while (nodo_temp != NULL) {
      if (nodo_temp->oper == N_PAR && nodo_temp->der != 0) {
        anota_pila = pila_temporal;
        reg = pedir_reg(NO);
        gen_nodo(nodo_temp->izq, reg);
        reg1 = pedir_reg(NO);
        reg2 = pedir_reg(NO);
        reg3 = pedir_reg(NO);
        gen_inst1("add", SI, reg1, 125, pos_pila + pila_temporal);
        gen_inst3("const", SI, reg2, tam = (int) nodo_temp->der - 2);
        if (tam < -65536 && tam > 65535)
          gen_inst3("consth", SI, reg2, tam);
        gen_destino(etiq = nueva_etiq);
        gen_inst2("load 0,4,", NO, reg3, reg);
        gen_inst1("add", SI, reg, reg, 4);
        gen_inst2("store 0,4,", NO, reg3, reg1);
        emite_texto("jmpfdec ");
        emite_registro(reg2);
        emite_texto(",");
        emite_etiq(etiq);
        emite_nueva_linea();
        gen_inst1("add", SI, reg1, reg1, 4);
        libera_reg(reg3);
        libera_reg(reg2);
        libera_reg(reg1);
        libera_reg(reg);
        if (pila_temporal - anota_pila)
          gen_inst1("add", SI, 125, 125, pila_temporal - anota_pila);
        pila_temporal -= pila_temporal - anota_pila;
        pos_pila += (int) nodo_temp->der * 4;
      }
      nodo_temp = nodo_temp->esp;
    }

    /*
    ** Paso 4: Todos los argumentos que requieren todos los registros
    **         (ej. llamada a función) se evaluan primero, en el
    **         caso de multiples argumentos complicados entonces
    **         se van guardando en la pila y luego se cargan.
    **         También resolvemos los casos de dirección complicada.
    */
    if (op == N_FUNCI && dir_complicada) {
      gen_nodo(nodo->esp, 120);      
      gen_inst1("sub", 1, 125, 125, 4);
      pila_temporal += 4;
      gen_inst2("store 0,4,", NO, 120, 125);
    }
    if (total != 0) {
      cuenta = total - 1;
      reg = 130;
      temp = 0;
      nodo_temp = nodo->izq;
      while (nodo_temp != NULL) {
        if ((nodo_temp->oper == N_PAR || nodo_temp->oper == N_PARF)
          && nodo_temp->der == 0) {
          if (nodo_temp->oper == N_PARF)
            reg = (reg + 1) & ~1;
          if (nodo_temp->regs >= 12) {
            if (cuenta--) {
              gen_nodo(nodo_temp->izq, 120);
              gen_inst1("sub", 1, 125, 125, 4);
              pila_temporal += 4;
              gen_inst2("store 0,4,", NO, 120, 125);
              if (nodo_temp->oper == N_PARF) {
                gen_inst1("sub", 1, 125, 125, 4);
                pila_temporal += 4;
                gen_inst2("store 0,4,", NO, 121, 125);
              }
              clase_argumento[temp] = (reg << 1) |
                                      (nodo_temp->oper == N_PARF);
              temp++;
            } else
              gen_nodo(nodo_temp->izq, reg);
          }
          if (nodo_temp->oper == N_PAR)
            reg++;
          else
            reg += 2;
        }
        nodo_temp = nodo_temp->esp;
      }
      for (cuenta = temp - 1; cuenta >= 0; cuenta--) {
        real = clase_argumento[cuenta] & 1;
        reg = clase_argumento[cuenta] >> 1;
        if (real) {
          gen_inst2("load 0,4,", NO, reg + 1, 125);
          gen_inst1("add", 1, 125, 125, 4);
          pila_temporal -= 4;
        }
        gen_inst2("load 0,4,", NO, reg, 125);
        gen_inst1("add", 1, 125, 125, 4);
        pila_temporal -= 4;
      }
    }
    nodo_temp = nodo->izq;
    reg = 130;
    while (nodo_temp != NULL) {
      if ((nodo_temp->oper == N_PAR || nodo_temp->oper == N_PARF)
        && nodo_temp->der == 0) {
        if (nodo_temp->oper == N_PARF)
          reg = (reg + 1) & ~1;
        if (nodo_temp->regs < 12)
          gen_nodo(nodo_temp->izq, reg);
        if (nodo_temp->oper == N_PAR)
          reg++;
        else
          reg += 2;
      }
      nodo_temp = nodo_temp->esp;
    }

    /*
    ** Paso 5: Generamos la llamada de función.
    */
    if (op == N_FUNCI) {
      if (dir_complicada) {
        gen_inst2("load 0,4,", NO, 120, 125);
        emite_linea("calli lr0,gr120");
        gen_inst1("add", 1, 125, 125, 4);
        pila_temporal -= 4;
      } else {
        gen_nodo(nodo_temp->esp, 120);
        emite_linea("calli lr0,gr120");
        gen_libre(1);
      }
    } else {
      llamada(nodo->esp);
    }
    if (resultado == 0)
      resultado = 96;
    if (!regresa_struct && pila_extra) {
      gen_inst1("add", SI, 125, 125, pila_extra);
      pila_temporal -= pila_extra;
    } else if (resultado != 96)
      gen_inst1("or", SI, resultado, 96, 0);
    return;
  }
  gen_nodo2(nodo, resultado);
}

gen_nodo2(nodo, resultado)
  struct nodo *nodo;
  int resultado;
{
  int op, temp, etiq;
  char *ap;

  op = nodo->oper;
  if (op == N_APFUNC) {
    if (resultado == 0)
      resultado = 96;
    temp = tipo_reg(resultado);
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(nodo->esp);
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(nodo->esp);
    emite_nueva_linea();
    return;
  }
  if (op == N_CEROPF) {
    if (resultado == 0)
      resultado = 96;
    gen_inst3("const", SI, resultado, 0);
    gen_inst3("consth", SI, resultado + 1, 0);
    return;
  }
  if (op == N_NUMPF) {
    if (resultado == 0)
      resultado = 96;
    temp = tipo_reg(resultado);
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado + 1);
    emite_texto(",");
    emite_etiq(etiq_lit);
    emite_texto("+");
    emite_numero(nodo->esp);
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado + 1);
    emite_texto(",");
    emite_etiq(etiq_lit);
    emite_texto("+");
    emite_numero(nodo->esp);
    emite_nueva_linea();
    gen_inst2("load 0,4,", NO, resultado, resultado + 1);
    gen_inst1("add", SI, resultado + 1, resultado + 1, 4);
    gen_inst2("load 0,4,", NO, resultado + 1, resultado + 1);
    return;
  }
  if (op == N_CONST) {
    if (resultado == 0)
      resultado = 96;
    gen_inst3("const", SI, resultado, nodo->esp);
    if (nodo->esp < -65536 || nodo->esp > 65535)
      gen_inst3("consth", SI, resultado, nodo->esp);
    return;
  }
  if (op == N_LIT) {
    if (resultado == 0)
      resultado = 96;
    temp = tipo_reg(resultado);
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    emite_etiq(etiq_lit);
    emite_texto("+");
    emite_numero(nodo->esp);
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    emite_etiq(etiq_lit);
    emite_texto("+");
    emite_numero(nodo->esp);
    emite_nueva_linea();
    return;
  }
  if (op == N_APRES) {
    if (resultado == 0)
      resultado = 96;
    gen_inst1("add", SI, resultado, 125, pila_temporal + pila);
    return;
  }
  if (op == N_DIR) {
    if (resultado == 0)
      resultado = 96;
    gen_inst1("add", SI, resultado, 125,
              (virtuales[nodo->esp] >> 2) + pila_temporal);
    return;
  }
  if (op == N_DIRG) {
    if (resultado == 0)
      resultado = 96;
    ap = nodo->esp;
    etiq = lee_entero(ap + POSICION);
    temp = tipo_reg(resultado);
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    if (etiq)
      emite_etiq(etiq);
    else
      emite_nombre(ap);
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    if (etiq)
      emite_etiq(etiq);
    else
      emite_nombre(ap);
    emite_nueva_linea();
    return;
  }
  gen_nodo3(nodo, resultado);
}

gen_nodo3(nodo, resultado)
  struct nodo *nodo;
  int resultado;
{
  int op, reg, temp, reg1, reg2;
  int real, inmediato, etiq;

  op = nodo->oper;
  if (op == N_ASIGNA) {
    real = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    if (en_registro(nodo->der)) {
      if (nodo->esp == FLOAT) {
        if (resultado == 0)
          resultado = 96;
        gen_nodo(nodo->izq, resultado);
        gen_inst4(registro(nodo->der), resultado, "2,1");
      } else {
        gen_nodo(nodo->izq, registro(nodo->der));
        if (resultado == 96 &&
           (nodo->izq->oper == N_FUNC || nodo->izq->oper == N_FUNCI))
          return;
        if (resultado && resultado != registro(nodo->der)) {
          gen_inst1("or", 1, resultado, registro(nodo->der), 0);
          if (real)
            gen_inst1("or", 1, resultado + 1, registro(nodo->der) + 1, 0);
        }
      }
      return;
    }
    if (nodo->der->regs < 12) {
      if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
        if (resultado && resultado != registro(nodo->izq->izq)) {
          gen_inst1("or", 1, resultado, registro(nodo->izq->izq), 0);
          if (real)
            gen_inst1("or", 1, resultado + 1, registro(nodo->izq->izq) + 1, 0);
        }
        resultado = registro(nodo->izq->izq);
      } else {
        if (resultado == 0)
          resultado = 96;
        gen_nodo(nodo->izq, resultado);
      }
      if (nodo->der->oper == N_CPAL && en_registro(nodo->der->izq))
        almacena_mem(nodo->esp, resultado, registro(nodo->der->izq));
      else {
        reg = pedir_reg(NO);
        gen_nodo(nodo->der, reg);
        almacena_mem(nodo->esp, resultado, reg);
        libera_reg(reg);
      }
      return;
    }
    gen_nodo(nodo->der, 120);
    gen_inst1("sub", 1, 125, 125, 4);
    pila_temporal += 4;
    gen_inst2("store 0,4,", NO, 120, 125);
    if (resultado == 0)
      resultado = 96;
    gen_nodo(nodo->izq, resultado);
    gen_inst2("load 0,4,", NO, 120, 125);
    gen_inst1("add", 1, 125, 125, 4);
    pila_temporal -= 4;
    almacena_mem(nodo->esp, resultado, 120);
    return;
  }
  if (op >= N_AOR && op <= N_AMOD) {
    real = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    if (en_registro(nodo->der)) {    /* Es operación con registro */
      if (nodo->esp == INT || nodo->esp == UINT) {
        if (es_constante(nodo->izq) &&
           (op >= N_AOR && op <= N_ARESTA)) {
          gen_oper(op, SI, registro(nodo->der),
                           registro(nodo->der), nodo->izq->esp, NO);
        } else {
          if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
            reg = registro(nodo->izq->izq);
          else {
            reg = pedir_reg(NO);
            gen_nodo(nodo->izq, reg);
          }
          gen_oper(op, NO, registro(nodo->der), registro(nodo->der), reg, NO);
          libera_reg(reg);
        }
        if (resultado)
          gen_inst1("or", SI, resultado, registro(nodo->der), 0);
        return;
      } else {
        if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
          reg = registro(nodo->izq->izq);
        else {
          reg = pedir_reg(NO);
          gen_nodo(nodo->izq, reg);
        }
        temp = resultado;
        if (resultado == 0)
          resultado = 96;
        carga_reg(nodo->esp, resultado, registro(nodo->der));
        if (nodo->esp != FLOAT && temp == 0)
          gen_oper(op, NO, registro(nodo->der), resultado, reg, NO);
        else
          gen_oper(op, NO, resultado, resultado, reg, NO);
        if (nodo->esp == FLOAT)
          gen_inst4(registro(nodo->der), resultado, "1,2");
        libera_reg(reg);
        return;
      }
    } else {                         /* Una dirección compleja */
      if ((nodo->izq->regs >= nodo->der->regs) &&
          (nodo->der->regs < 12)) {
        if (resultado == 0)
          resultado = 96;
        inmediato = NO;
        if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
          reg1 = registro(nodo->izq->izq);
        else if (nodo->izq->oper == N_CONST
             && (op >= N_AOR && op <= N_ARESTA)) {
          inmediato = SI;
          reg1 = nodo->izq->esp;
        } else {
          reg1 = resultado;
          gen_nodo(nodo->izq, reg1);
        }
        reg2 = pedir_reg(NO);
        gen_nodo(nodo->der, reg2);
        temp = pedir_reg(real);
        carga_mem(nodo->esp, temp, reg2);
        gen_oper(op, inmediato, resultado, temp, reg1, NO);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        libera_reg(reg2);
        return;
      } else if ((nodo->der->regs > nodo->izq->regs) &&
                 (nodo->izq->regs < 12)) {
        if (resultado == 0)
          resultado = 96;
        inmediato = NO;
        reg2 = pedir_reg(NO);
        gen_nodo(nodo->der, reg2);
        if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
          reg1 = registro(nodo->izq->izq);
        else if (nodo->izq->oper == N_CONST
             && (op >= N_AOR && op <= N_ARESTA)) {
          inmediato = SI;
          reg1 = nodo->izq->esp;
        } else {
          reg1 = resultado;
          gen_nodo(nodo->izq, reg1);
        }
        temp = pedir_reg(real);
        carga_mem(nodo->esp, temp, reg2);
        gen_oper(op, inmediato, resultado, temp, reg1, NO);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        libera_reg(reg2);
        return;
      } else {
        gen_nodo(nodo->der, reg2 = 120);
        gen_inst1("sub", 1, 125, 125, 4);
        pila_temporal += 4;
        gen_inst2("store 0,4,", NO, 120, 125);
        if (resultado == 0)
          resultado = 96;
        gen_nodo(nodo->izq, resultado);
        gen_inst2("load 0,4,", NO, 120, 125);
        gen_inst1("add", 1, 125, 125, 4);
        pila_temporal -= 4;
        temp = pedir_reg(real);
        carga_mem(nodo->esp, temp, reg2);
        gen_oper(op, NO, resultado, temp, resultado, NO);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        return;
      }
    }
  }
  if ((op == N_INC) || (op == N_PINC)) {
    temp = (int) nodo->der;
    if (en_registro(nodo->izq)) {
      if (op == N_PINC)
        if (resultado)
          carga_reg(nodo->esp, resultado, registro(nodo->izq));
      if (temp < 0)
        gen_inst1("sub", SI, registro(nodo->izq), registro(nodo->izq), -temp);
      else
        gen_inst1("add", SI, registro(nodo->izq), registro(nodo->izq), temp);
      if (op == N_INC)
        if (resultado)
          gen_inst1("or", SI, resultado, registro(nodo->izq), 0);
    } else {
      reg1 = pedir_reg(NO);
      gen_nodo(nodo->izq, reg1);
      if (resultado == 0)
        resultado = 96;
      if (op == N_PINC) {
        carga_mem(nodo->esp, resultado, reg1);
        reg2 = pedir_reg(NO);
        if (temp < 0)
          gen_inst1("sub", SI, reg2, resultado, -temp);
        else
          gen_inst1("add", SI, reg2, resultado, temp);
        almacena_mem(nodo->esp, reg2, reg1);
        libera_reg(reg2);
      } else {
        carga_mem(nodo->esp, resultado, reg1);
        if (temp < 0)
          gen_inst1("sub", SI, resultado, resultado, -temp);
        else
          gen_inst1("add", SI, resultado, resultado, temp);
        almacena_mem(nodo->esp, resultado, reg1);
      }
      libera_reg(reg1);
    }
    return;
  }
  if (op == N_ANDB) {
    salta_expr(nodo->izq, temp = nueva_etiq, NO);
    salta_expr(nodo->der, temp, NO);
    if (resultado == 0)
      resultado = 96;
    gen_inst3("const", SI, resultado, 1);
    salto(etiq = nueva_etiq);
    gen_destino(temp);
    gen_inst3("const", SI, resultado, 0);
    gen_destino(etiq);
    return;
  }
  if (op == N_ORB) {
    salta_expr(nodo->izq, etiq = nueva_etiq, SI);
    salta_expr(nodo->der, temp = nueva_etiq, NO);
    if (resultado == 0)
      resultado = 96;
    gen_destino(etiq);
    gen_inst3("const", SI, resultado, 1);
    salto(etiq = nueva_etiq);
    gen_destino(temp);
    gen_inst3("const", SI, resultado, 0);
    gen_destino(etiq);
    return;
  }
  if (op == N_TRI) {
    temp = nueva_etiq;
    salta_expr(nodo->esp, etiq = nueva_etiq, NO);
    if (resultado == 0)
      resultado = 96;
    gen_nodo(nodo->izq, resultado);
    salto(temp = nueva_etiq);
    gen_destino(etiq);
    gen_nodo(nodo->der, resultado);
    gen_destino(temp);
    return;
  }
  if (op == N_COMA) {
    gen_nodo(nodo->izq, resultado);
    gen_nodo(nodo->der, resultado);
    return;
  }
  gen_nodo4(nodo, resultado);
}

gen_nodo4(nodo, resultado)
  struct nodo *nodo;
  int resultado;
{
  int op, anota_pila;
  int reg, temp, reg1, reg2, reg3, tam;
  int real, inmediato, etiq;

  op = nodo->oper;
  if (op == N_COPIA) {
    if (resultado == 0)
      resultado = 96;
    anota_pila = pila_temporal;
    if ((nodo->izq->regs >= nodo->der->regs) &&
        (nodo->der->regs < 12)) {
      reg = pedir_reg(NO);
      gen_nodo(nodo->izq, reg);
      gen_nodo(nodo->der, resultado);
    } else if ((nodo->der->regs > nodo->izq->regs) &&
               (nodo->izq->regs < 12)) {
      gen_nodo(nodo->der, resultado);
      reg = pedir_reg(NO);
      gen_nodo(nodo->izq, reg);
    } else {
      gen_nodo(nodo->der, resultado);
      gen_inst1("sub", 1, 125, 125, 4);
      pila_temporal += 4;
      gen_inst2("store 0,4,", NO, resultado, 125);
      reg = pedir_reg(NO);
      gen_nodo(nodo->izq, reg);
      gen_inst2("load 0,4,", NO, resultado, 125);
      gen_inst1("add", 1, 125, 125, 4);
      pila_temporal -= 4;
    }
    reg1 = pedir_reg(NO);
    gen_inst1("or", SI, reg1, resultado, 0);
    reg2 = pedir_reg(NO);
    reg3 = pedir_reg(NO);
    gen_inst3("const", SI, reg2, tam = nodo->esp - 2);
    if (tam < -65536 && tam > 65535)
      gen_inst3("consth", SI, reg2, tam);
    gen_destino(etiq = nueva_etiq);
    gen_inst2("load 0,4,", NO, reg3, reg);
    gen_inst1("add", SI, reg, reg, 4);
    gen_inst2("store 0,4,", NO, reg3, reg1);
    emite_texto("jmpfdec ");
    emite_registro(reg2);
    emite_texto(",");
    emite_etiq(etiq);
    emite_nueva_linea();
    gen_inst1("add", SI, reg1, reg1, 4);
    libera_reg(reg3);
    libera_reg(reg2);
    libera_reg(reg1);
    libera_reg(reg);
    if (pila_temporal - anota_pila)
      gen_inst1("add", SI, 125, 125, pila_temporal - anota_pila);
    pila_temporal -= pila_temporal - anota_pila;
    return;
  }
  if (op == N_CPAL) {
    if (en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst1("or", SI, resultado, registro(nodo->izq), 0);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, registro(nodo->izq->izq));
      return;
    }
  }
  if (op == N_CBYTE) {
    if (temp = en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst1("and", SI, resultado, registro(nodo->izq), 255);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("exbyte", SI, resultado, resultado, 0);
      return;
    }
  }
  if (op == N_CSHORT) {
    if (temp = en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst1("sll", 1, resultado, registro(nodo->izq), 16);
      gen_inst1("sra", 1, resultado, resultado, 16);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst3("exhws", NO, resultado, resultado);
      return;
    }
  }
  if (op == N_CUSHORT) {
    if (temp = en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst1("or", SI, resultado, registro(nodo->izq), 0);
      gen_inst3("consth", SI, resultado, 0);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("exhw", SI, resultado, resultado, 0);
      return;
    }
  }
  if (op == N_CFLOAT) {
    if (temp = en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst4(resultado, registro(nodo->izq), "2,1");
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, registro(nodo->izq->izq));
      gen_inst4(resultado, resultado, "2,1");
      return;
    }
  }
  if (op == N_CDOUBLE) {
    if (temp = en_registro(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst1("or", 1, resultado, registro(nodo->izq), 0);
      gen_inst1("or", 1, resultado + 1, registro(nodo->izq) + 1, 0);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado + 1, registro(nodo->izq->izq));
      gen_inst1("add", SI, resultado, registro(nodo->izq->izq), 4);
      gen_inst2("load 0,4,", NO, resultado, resultado);
      return;
    }
  }
  real = (op >= N_IGUALPF && op <= N_MAYORIPF)
      || (op >= N_SUMAPF && op <= N_DIVPF);
  if (nodo->der != NULL) {
    if ((nodo->izq->regs >= nodo->der->regs) &&
        (nodo->der->regs < 12)) {
      if (resultado == 0)
        resultado = 96;
      if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
        reg1 = registro(nodo->izq->izq);
      else if (nodo->izq->oper == N_FUNC || nodo->izq->oper == N_FUNCI) {
        reg1 = 96;
        gen_nodo(nodo->izq, reg1);
      } else {
        reg1 = resultado;
        gen_nodo(nodo->izq, reg1);
      }
      inmediato = NO;
      if (nodo->der->oper == N_CPAL && en_registro(nodo->der->izq))
        reg2 = registro(nodo->der->izq);
      else if (nodo->der->oper == N_CONST && (op >= N_OR && op <= N_RESTA)) {
        inmediato = SI;
        reg2 = nodo->der->esp;
      } else {
        reg2 = pedir_reg(real);
        gen_nodo(nodo->der, reg2);
      }
      gen_oper(op, inmediato, resultado, reg1, reg2, nodo->esp);
      if (!inmediato)
        libera_reg(reg2);
      return;
    } else if ((nodo->der->regs > nodo->izq->regs) &&
               (nodo->izq->regs < 12)) {
      if (resultado == 0)
        resultado = 96;
      if (nodo->der->oper == N_CPAL && en_registro(nodo->der->izq))
        reg2 = registro(nodo->der->izq);
      else if ((nodo->der->oper == N_FUNC || nodo->der->oper == N_FUNCI)
             && resultado != 96) {
        reg2 = 96;
        gen_nodo(nodo->der, reg2);
      } else {
        reg2 = pedir_reg(real);
        gen_nodo(nodo->der, reg2);
      }
      if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
        reg1 = registro(nodo->izq->izq);
      else {
        reg1 = resultado;
        gen_nodo(nodo->izq, reg1);
      }
      gen_oper(op, NO, resultado, reg1, reg2, nodo->esp);
      libera_reg(reg2);
      return;
    } else {
      if (nodo->der->oper == N_FUNC || nodo->der->oper == N_FUNCI)
        reg2 = 96;
      else
        reg2 = 120;
      gen_nodo(nodo->der, reg2);
      gen_inst1("sub", 1, 125, 125, 4);
      pila_temporal += 4;
      gen_inst2("store 0,4,", NO, reg2, 125);
      if (real) {
        gen_inst1("sub", 1, 125, 125, 4);
        pila_temporal += 4;
        gen_inst2("store 0,4,", NO, reg2 + 1, 125);
      }
      if (resultado == 0)
        resultado = 96;
      gen_nodo(nodo->izq, resultado);
      if (real) {
        gen_inst2("load 0,4,", NO, 121, 125);
        gen_inst1("add", 1, 125, 125, 4);
        pila_temporal -= 4;
      }
      gen_inst2("load 0,4,", NO, 120, 125);
      gen_inst1("add", 1, 125, 125, 4);
      pila_temporal -= 4;
      gen_oper(op, NO, resultado, resultado, 120, nodo->esp);
      return;
    }
  } else {
    gen_nodo(nodo->izq, resultado);
    gen_oper(op, NO, resultado, resultado, resultado, NO);
    return;
  }
}

carga_reg(tipo, reg1, reg2)
  int tipo, reg1, reg2;
{
  if (tipo == INT || tipo == UINT) {
    gen_inst1("or", SI, reg1, reg2, 0);
  } else if (tipo == SHORT) {
    gen_inst1("sll", SI, reg1, reg2, 16);
    gen_inst1("sra", SI, reg1, reg1, 16);
  } else if (tipo == USHORT) {
    gen_inst1("or", SI, reg1, reg2, 0);
    gen_inst3("consth", SI, reg1, 0);
  } else if (tipo == DOUBLE) {
    gen_inst1("or", SI, reg1, reg2, 0);
    gen_inst1("or", SI, reg1 + 1, reg2 + 2, 0);
  } else if (tipo == FLOAT) {
    gen_inst4(reg1, reg2, "2,1");
  } else {
    gen_inst1("and", SI, reg1, reg2, 255);
  }
}

carga_mem(tipo, reg1, reg2)
  int tipo, reg1, reg2;
{
  if (tipo == INT || tipo == UINT)
    gen_inst2("load 0,4,", NO, reg1, reg2);
  else if (tipo == SHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst3("exhws", NO, reg1, reg1);
  } else if (tipo == USHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst1("exhw", SI, reg1, reg1, 0);
  } else if (tipo == DOUBLE) {
    gen_inst2("load 0,4,", NO, reg1 + 1, reg2);
    gen_inst1("add", SI, reg1, reg2, 4);
    gen_inst2("load 0,4,", NO, reg1, reg1);
  } else if (tipo == FLOAT) {
    gen_inst2("load 0,4,", NO, reg1, reg2);
    gen_inst4(reg1, reg1, "2,1");
  } else {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst1("exbyte", SI, reg1, reg1, 0);
  }
}

almacena_mem(tipo, reg1, reg2)
  int tipo, reg1, reg2;
{
  int reg;

  if (tipo == INT || tipo == UINT)
    gen_inst2("store 0,4,", NO, reg1, reg2);
  else if (tipo == SHORT || tipo == USHORT) {
    reg = pedir_reg(NO);
    gen_inst2("load 0,20,", NO, reg, reg2);
    gen_inst1("inhw", NO, reg, reg, reg1);
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  } else if (tipo == DOUBLE) {
    gen_inst2("store 0,4,", NO, reg1 + 1, reg2);
    reg = pedir_reg(NO);
    gen_inst1("add", NO, reg, reg2, 4);
    gen_inst2("store 0,4,", NO, reg1, reg);
    libera_reg(reg);
  } else if (tipo == FLOAT) {
    reg = pedir_reg(NO);
    gen_inst4(reg, reg1, "1,2");
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  } else {
    reg = pedir_reg(NO);
    gen_inst2("load 0,20,", NO, reg, reg2);
    gen_inst1("inbyte", NO, reg, reg, reg1);
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  }
}

/*
** Copia una estructura para resultado de función.
*/
copia_resultado(nodo, tam)
  struct nodo **nodo;
  int tam;
{
  crea_nodo(N_APRES, NULL, NULL, 0);
  crea_nodo(N_COPIA, ultimo_nodo, *nodo, tam);
  *nodo = ultimo_nodo;
}

/*
** Pone el prologo para el codigo generado.
*/
prologo()
{
  emite_texto("; ");
  emite_linea(PROGRAMA);
  emite_linea("COMIENZO:");
  emite_linea("const gr96,INICIO");
  emite_linea("consth gr96,INICIO");
  emite_linea("jmpi gr96");
  emite_linea("nop");
}

/*
** Pone el epilogo para el codigo generado.
*/
epilogo()
{
  emite_linea("; --- Fin de compilación ---");
  emite_linea("INICIO:");
  emite_linea("sub gr1,gr1,32");
  emite_linea("asgeu 64,gr1,gr126");
  emite_linea("add lr1,gr1,40");
  emite_linea("call lr0,__main");
  emite_linea("nop");
  emite_linea("cpeq gr97,gr96,0");
  emite_linea("jmpf gr97,FIN");
  emite_linea("nop");
  emite_linea("call lr0,_main");
  emite_linea("nop");
  emite_linea("FIN:");
  emite_linea("call lr0,__exit");
  emite_linea("nop");
  emite_linea("add gr1,gr1,32");
  emite_linea("nop");
  emite_linea("jmpi lr0");
  emite_linea("asleu 65,lr1,gr127");
  vacia_buffer();
}

/*
** Emite un nombre que no entre en conflicto con las
** palabras reservadas del ensamblador.
*/
emite_nombre(nombre)
  unsigned char *nombre;
{
  emite_texto("_");
  emite_texto(nombre);
}

/*
** Llama a la función especificada.
*/
llamada(nombre)
  unsigned char *nombre;
{
  emite_texto("call lr0,");
  emite_nombre(nombre);
  emite_nueva_linea();
  gen_libre(1);
}

/*
** Salta a la etiqueta interna especificada.
*/
salto(etiq)
  int etiq;
{
  estado_buf[total_lineas] = 10;
  emite_texto("jmp \1\1\1\1\1\1\1");
  emite_etiq(etiq);
  emite_nueva_linea();
  gen_libre(3);
}

/*
** Prueba una expresión, genera también secuencias óptimas de
** expresiones booleanas de corto circuito (también llamadas codigo
** saltado)
*/
salta_expr(expr, etiq, forza_not)
  struct nodo *expr;
  int etiq, forza_not;
{
  int etiq_or, etiq_and;

  if (forza_not && expr->oper == N_NOT) {
    expr = expr->izq;
    forza_not = NO;
  }
  if (!forza_not && expr->oper == N_ANDB) {
    salta_expr(expr->izq, etiq, NO);
    salta_expr(expr->der, etiq, NO);
  } else if (!forza_not && expr->oper == N_ORB) {
    salta_expr(expr->izq, etiq_or = nueva_etiq, SI);
    salta_expr(expr->der, etiq, NO);
    gen_destino(etiq_or);
  } else if (forza_not && expr->oper == N_ANDB) {
    salta_expr(expr->izq, etiq_and = nueva_etiq, NO);
    salta_expr(expr->der, etiq, SI);
    gen_destino(etiq_and);
  } else if (forza_not && expr->oper == N_ORB) {
    salta_expr(expr->izq, etiq, SI);
    salta_expr(expr->der, etiq, SI);
  } else if (expr->oper == N_NOT && expr->izq->oper == N_ANDB) {
    salta_expr(expr->izq->izq, etiq_and = nueva_etiq, NO);
    salta_expr(expr->izq->der, etiq, SI);
    gen_destino(etiq_and);
  } else if (expr->oper == N_NOT && expr->izq->oper == N_ORB) {
    salta_expr(expr->izq->izq, etiq, SI);
    salta_expr(expr->izq->der, etiq, SI);
  } else if (es_comparacion(expr)) {
    expr->esp = 1;
    gen_nodo(expr, 96);
    if (forza_not)
      salta_si_verdadero(etiq, 96);
    else
      salta_si_falso(etiq, 96);
  } else if (expr->oper == N_NOT && es_comparacion(expr->izq)) {
    expr->izq->esp = 1;
    gen_nodo(expr->izq, 96);
    if (forza_not)
      salta_si_falso(etiq, 96);
    else
      salta_si_verdadero(etiq, 96);
  } else if (expr->oper == N_NOT) {
    if (expr->izq->oper == N_CPAL && en_registro(expr->izq->izq))
      gen_inst1("cpneq", SI, 96, registro(expr->izq->izq), 0);
    else {
      gen_nodo(expr->izq, 96);
      gen_inst1("cpneq", SI, 96, 96, 0);
    }
    if (forza_not)
      salta_si_falso(etiq, 96);
    else
      salta_si_verdadero(etiq, 96);
  } else {
    if (expr->oper == N_CPAL && en_registro(expr->izq))
      gen_inst1("cpneq", SI, 96, registro(expr->izq), 0);
    else {
      gen_nodo(expr, 96);
      gen_inst1("cpneq", SI, 96, 96, 0);
    }
    if (forza_not)
      salta_si_verdadero(etiq, 96);
    else
      salta_si_falso(etiq, 96);
  }
}

/*
** Prueba el registro y salta si es falso.
*/
salta_si_falso(etiq, reg)
  int etiq, reg;
{
  estado_buf[total_lineas] = 12;
  emite_texto("jmpf ");
  emite_registro(reg);
  emite_texto(",c");
  emite_numero(etiq);
  emite_nueva_linea();
  gen_libre(2);
}

/*
** Prueba el registro y salta si es verdadero.
*/
salta_si_verdadero(etiq, reg)
  int etiq, reg;
{
  estado_buf[total_lineas] = 13;
  emite_texto("jmpt ");
  emite_registro(reg);
  emite_texto(",c");
  emite_numero(etiq);
  emite_nueva_linea();
  gen_libre(2);
}

/*
** Imprime el número especificado cómo una etiqueta.
*/
emite_etiq(etiq)
  int etiq;
{
  emite_texto("c");
  emite_numero(etiq);
}

dos_puntos()
{
  emite_car(58);
}

/*
** Seudo-operacion para definir un byte.
*/
def_byte()
{
  emite_texto(".byte ");
}

/*
** Seudo-operacion para definir una palabra.
*/
def_palabra()
{
  emite_texto(".word ");
}

/*
** Define espacio
*/
def_espacio(val)
  int val;
{
  emite_texto(".space ");
  emite_numero(val);
  emite_nueva_linea();
}

/*
** Define nombre global
*/
def_global(nombre)
  char *nombre;
{
  emite_texto(".global ");
  emite_nombre(nombre);
  emite_nueva_linea();
}

/*
** Hace una comparación y un salto. (para switch)
** No pierde el valor con el que esta comparando.
*/
compara_y_salta(valor, etiqueta)
  int valor, etiqueta;
{
  gen_inst1("cpeq", SI, 97, 96, valor);
  estado_buf[total_lineas] = 13;
  emite_texto("jmpt gr97,");
  emite_etiq(etiqueta);
  emite_nueva_linea();
  gen_libre(2);
}

/*
** Vacia el almacenamiento de cadenas
*/
vacia_lits()
{
  int j, k;

  if (ap_lit == 0)
    return;             /* No hay nada, volver... */
  gen_destino(etiq_lit);/* Imprime la etiqueta */
  k = 0;                /* Inicia un indice... */
  while (k < ap_lit) {  /* para vaciar el almacenamiento */
    def_byte();         /* Define byte */
    j = 4;              /* Bytes por línea */
    while (j--) {
      emite_numero(lits[k++]);
      if ((j == 0) || (k >= ap_lit)) {
        emite_nueva_linea();  /* Otra línea */
        break;
      }
      emite_car(',');   /* Separa los bytes */
    }
  }
  if (ap_lit & 3)
    emite_linea(".align");
}

gen_inst1(instruccion, inmediato, reg1, reg2, valreg)
  char *instruccion;
  int inmediato, reg1, reg2, valreg;
{
  int reg, valor;

  if (inmediato && (valreg < 0 || valreg > 255)) {
    reg = pedir_reg(NO);
    gen_inst3("const", SI, reg, valreg);
    if (valreg < -65536 || valreg > 65535)
      gen_inst3("consth", SI, reg, valreg);
  }
  emite_texto(instruccion);
  emite_texto(" ");
  emite_registro(reg1);
  emite_texto(",");
  emite_registro(reg2);
  emite_texto(",");
  valor = tipo_reg(reg1);
  if (valor >= 2 && valor <= 4)
    valor += 3;
  valor = max(valor, tipo_reg(reg2));
  if (!inmediato) {
    valor = max(valor, tipo_reg(valreg));
    emite_registro(valreg);
  } else if (valreg >= 0 && valreg <= 255) {
    emite_numero(valreg);
  } else {
    emite_registro(reg);
    libera_reg(reg);
  }
  estado_buf[total_lineas] = valor;
  emite_nueva_linea();
}

gen_inst2(instruccion, inmediato, reg1, valreg)
  char *instruccion;
  int inmediato, reg1, valreg;
{
  int reg, valor;

  if (inmediato && (valreg < 0 || valreg > 255)) {
    reg = pedir_reg(NO);
    gen_inst3("const", SI, reg, valreg);
    if (valreg < -65536 || valreg > 65535)
      gen_inst3("consth", SI, reg, valreg);
  }
  emite_texto(instruccion);
  emite_registro(reg1);
  emite_texto(",");
  valor = tipo_reg(reg1);
  if (valor >= 2 && valor <= 4)
    valor += 3;
  if (!inmediato) {
    emite_registro(valreg);
    valor = max(valor, tipo_reg(valreg));
  } else if (valreg >= 0 && valreg <= 255)
    emite_numero(valreg);
  else {
    emite_registro(reg);
    libera_reg(reg);
  }
  estado_buf[total_lineas] = valor;
  emite_nueva_linea();
}

gen_inst3(instruccion, inmediato, reg1, valreg)
  char *instruccion;
  int inmediato, reg1, valreg;
{
  int valor;

  emite_texto(instruccion);
  emite_texto(" ");
  emite_registro(reg1);
  emite_texto(",");
  valor = tipo_reg(reg1);
  if (valor >= 2 && valor <= 4)
    valor += 3;
  if (inmediato) {
    emite_numero(valreg);
  } else {
    emite_registro(valreg);
    valor = max(valor, tipo_reg(valreg));
  }
  estado_buf[total_lineas] = valor;
  emite_nueva_linea();
}

gen_inst4(reg1, reg2, pars)
  int reg1, reg2;
  char *pars;
{
  int valor;

  valor = tipo_reg(reg1);
  if (valor >= 2 && valor <= 4)
    valor += 3;
  estado_buf[total_lineas] = max(valor, tipo_reg(reg2));
  emite_texto("convert ");
  emite_registro(reg1);
  emite_texto(",");
  emite_registro(reg2);
  emite_texto(",0,0,");
  emite_linea(pars);
}

gen_destino(etiq)
  int etiq;
{
  if (etiq == 0)
    return;
  estado_buf[total_lineas] = 11;
  emite_etiq(etiq);
  dos_puntos();
  emite_nueva_linea();
}

/*
** Genera una instrucción retardada (disponible para ser usada por una
** instrucción que no afecte la ejecución)
**
** Si tipo == 0  ->  Se acepta instrucción que no accese local > lr0
** Si tipo == 1  ->  Se acepta instrucción activa (1, 2, 3 o 7).
*/
gen_libre(tipo)
  int tipo;
{
  int a, b;
  char *ap;

  a = (total_lineas - 2) & (MAX_LIN - 1);
  b = (total_lineas - 1) & (MAX_LIN - 1);
  if ((tipo == 0
   && (estado_buf[a] == 1 || estado_buf[a] == 2 || estado_buf[a] == 5))
   || ((tipo == 1 || tipo == 3)
   && (estado_buf[a] >= 1 && estado_buf[a] <= 7))) {
    ap = linea_inst[a];
    linea_inst[a] = linea_inst[b];
    linea_inst[b] = ap;
    estado_buf[a] = estado_buf[b];
    estado_buf[b] = 0;
    return;
  }
  if (tipo == 2 || tipo == 3)
    estado_buf[total_lineas] = 8;
  else
    estado_buf[total_lineas] = 0;
  emite_linea("nop");
}

tipo_reg(reg)
  int reg;
{
  if (total_regs >= 0 && reg >= 128 && reg <= total_regs + 129)
    return 4;
  if (total_regs < 0 && reg >= 128 && reg <= 129)
    return 4;
  if (reg >= 96 && reg <= 115)
    return 1;
  if (reg >= 116 && reg <= 127)
    return 2;
  return 3;
}

max(a, b)
  int a, b;
{
  if (a > b)
    return a;
  else
    return b;
}

emite_registro(registro)
  int registro;
{
  if (registro < 128) {
    emite_texto("gr");
    emite_numero(registro);
  } else {
    emite_texto("lr");
    emite_numero(registro - 128);
  }
}

