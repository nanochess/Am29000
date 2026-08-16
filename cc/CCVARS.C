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
