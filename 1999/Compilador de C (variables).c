/*
** Compilador de C para G11.
** Definiciones de variables.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 02-jun-1995.
** Revisión: 05-sep-1998. Los datos de revisiones anteriores son guardados
**                        en la carpeta de antiguedades. Rediseño global
**                        del compilador, con objetivo de ANSI C más parte del
**                        lenguaje ANSI C++. Me vuelvo a superar.
** Revisión: 07-sep-1998. El analizador léxico empieza a funcionar.
** Revisión: 12-sep-1998. Se cambian los nombres matriz->arreglo.
** Revisión: 12-sep-1998. Soporte para signed char.
** Revisión: 15-sep-1998. Desaparece el limite de #include
** Revisión: 12-oct-1998. Rediseño general para soportar tipos dinámicos y
**                        prototipos.
** Revisión: 26-oct-1998. Nuevas variables adv_cs y adv_ansi.
** Revisión: 24-nov-1998. Nueva variable prog_grande. Se extienden las tablas
**                        de dispersión a 257.
*/

#define PROGRAMA     "Compilador de ANSI C para G11, "\
                     "(c) Copyright Oscar Toledo G.1995-1998"

#define NO           0
#define SI           1

#define NULL         ((void *) 0)

/*
** La memoria para tipos se asigna en bloques
*/
#define TAM_TIPOS    1024

/*
** Define el almacenamiento de cadenas
*/
#define TAM_LITS     4096
#define MAX_LITS     (TAM_LITS - 1)

/*
** Define la linea de entrada
*/
#define TAM_LINEA    1024
#define MAX_LINEA    (TAM_LINEA - 1)

/*
** Define el espacio disponible para substitución de argumentos de macros
*/
#define TAM_AMAC     1024
#define MAX_AMAC     (TAM_AMAC - 1)

/*
** Un número primo para las tablas de dispersión
*/
#define NUM_PRIMO    257

/*
** Tamaño máximo de los nombres
*/
#define TAM_NOMBRE   32
#define MAX_NOMBRE   31

/*
** Valores posibles para "IDENT"
*/
#define VARIABLE     1
#define ETIQUETA     2
/*#define FUNCION   13*/ /* Definido más abajo*/
#define TYPEDEF      4

/*
** Valores posibles para "CLASE"
*/
#define STATIC       1
#define AUTO         2
#define EXTERN       3
/*define TYPEDEF     4*/
#define REGISTER     5

/*
** Valores posibles para "TIPO"
*/
#define SCHAR        0
#define SSHORT       1
#define SINT         2
#define UCHAR        3
#define USHORT       4
#define UINT         5
#define FLOAT        6
#define DOUBLE       7
#define VOID         8
#define STRUCT       9
#define ENUM        10

#define APUNTADOR   11
#define ARREGLO     12
#define FUNCION     13

#define CONST       14
#define VOLATILE    15

#define FUNC_REF     0
#define FUNC_TIPO    1
#define FUNC_DEF     2

struct nombres {          /* REPRESENTACIÓN DE UN NOMBRE LOCAL O GLOBAL */
  struct nombres *sig;    /* Siguiente nombre en la tabla */
  struct tipo *tipo;      /* Tipo del identificador */
  int posicion;           /* Etiqueta o No. de variable virtual */
  char ident;             /* Clase de identificador */
  char clase;             /* Clase de almacenamiento */
  char nivel;             /* Nivel de declaración */
  char nombre[1];         /* Nombre del simbolo */
};

struct bucle {            /* REPRESENTACIÓN DE LA COLA DE BUCLES */
  struct bucle *ant;      /* Apuntador a bucle anterior */
  int bucle;              /* Etiqueta de bucle */
  int fin;                /* Etiqueta de fin */
};

struct macro {            /* DEFINICIÓN DE MACRO */
  struct macro *sig;      /* Siguiente macro */
  char *definicion;       /* Definición de la macro */
  signed char parametros; /* Total de parametros */
  char nombre[1];         /* Nombre de la macro */
};

struct tipo {             /* DEFINICIÓN DE TIPO */
  struct tipo *sig;       /* Apuntador a siguiente tipo */
  char tipo;              /* Tipo */
  unsigned char num_pars; /* Número de parametros de función */
                          /*   0 -  63 si parametros ANSI */
                          /*  64 - 127 si parametros ANSI variables */
                          /* 128       si Kernighan & Ritchie */
  union {                 /* Dependiente del tipo: */
    int tam;              /* ARREGLO: Tamaño */
    struct rotulo *est;   /* STRUCT: Apuntador a definición */
    struct proto *proto;  /* FUNCION: Prototipo de función (NULL si K&R) */
  } especial;
};

struct proto {            /* PROTOTIPO DE FUNCIÓN */
  struct proto *sig;      /* Siguiente parametro */
  struct tipo *tipo;      /* Tipo del parametro */
  char nombre[1];         /* Nombre del parametro */
};

struct rotulo {           /* DEFINICIÓN DE ESTRUCTURA */
  struct rotulo *sig;     /* Siguiente rótulo */
  int tam;                /* Tamaño total de la estructura */
  struct miembro *lista;  /* Lista de miembros */
  char que_es;            /* Indica si es un rótulo de struct o enum */
  char es_union;          /* Indica si es una unión o una estructura */
  char nombre[1];         /* Nombre */
};

struct miembro {          /* DEFINICIÓN DE MIEMBRO DE ESTRUCTURA */
  struct miembro *sig;    /* Siguiente miembro */
  int posicion;           /* Posición dentro de la estructura */
  struct tipo *tipo;      /* Tipo declarado */
  char nombre[1];         /* Nombre */
};

struct enumerador {       /* DEFINICIÓN DE ENUMERADOR */
  struct enumerador *sig; /* Siguiente enumerador */
  int valor;              /* Valor del enumerador */
  char nombre[1];         /* Nombre */
};

#define TAM_BLOQUE   1000 /* Asigna bloques en pasos de 1000 bytes */

struct bloque {           /* BLOQUE GENÉRICO DE ASIGNACIÓN */
  int pos;                /* Siguiente posición disponible */
  unsigned
  char datos[TAM_BLOQUE]; /* Espacio para datos */
  struct bloque *ant;     /* Bloque anterior */
  struct bloque *sig;     /* Bloque siguiente */
};

struct contexto {         /* CONTEXTO DE BLOQUE */
  struct bloque **variable;
  struct bloque *bloque;
  int posicion;
};

#define MAX_FUNCIONES 64  /* Hasta 64 funciones distintas por bloque */

struct funciones {        /* FUNCIONES LLAMADAS EN BLOQUE */
  struct nombres *func;   /* Función */
  int etiqueta;           /* Etiqueta utilizada */
} funciones[MAX_FUNCIONES];

int total_funciones;      /* Total de funciones llamadas */

/* Reserva espacio para las variables */
#define MAX_VIRTUALES   /* 300 */ 150  /* Total variables virtuales x 3 */
#define MAX_ARGS         32
#define TAM_DOUBLE  8   /* Tamaño en bytes del tipo double */
#define MAX_BUFR  768   /* Buffer para instrucciones retrasadas (delay slot) */
#define MAX_LIN     8   /* Total máximo de instrucciones almacenadas y */
                        /* debe ser un multiplo de 2. */

struct nombres *ap_loc; /* Apuntador a la tabla de nombres */

struct bucle *ultimo_bucle; /* Apuntador al último bucle abierto */

int ap_lit;             /* Apuntador a la sig. entrada para las cadenas */

char *pos_linea = 0,    /* Apuntadores a las lineas de análisis */
     *pos_linea_m = 0;

/* Almacenamiento miscelaneo */

int sig_etiq = 0;       /* Siguiente etiqueta disponible */
int etiq_lit,           /* Etiqueta para el buffer de cadenas */
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
    lineas_totales,     /* Total de líneas compiladas */
    dentro_funcion,     /* Indica si esta dentro de una funcion */
    dentro_pp,          /* Indica si esta dentro del preprocesador */
    nivel_if,           /* Nivel de anidamiento de #if... */
    nivel_incl,         /* Nivel de anidamiento de #include */
    ultima_sentencia,   /* Ultima sentencia procesada */
    evadir_nivel,       /* Nivel que esta evadiendo en el preprocesador */
    dentro_switch,      /* Indica si esta dentro de una sentencia switch */
    etiqueta_default;   /* Etiqueta para el default */

int adv_cs,             /* Advertencia de conversión sospechosa */
    adv_ansi,           /* Advertencia estricta de ANSI C */
    prog_grande;        /* Genera codigo para programas grandes */

struct sentencia *lista_case, *ultimo_case;

int entrada,            /* Archivo actual de entrada */
    salida;             /* Archivo de salida */

                        /* Apuntador a la definición de la función actual */
struct nombres *funcion_actual;

int clase_alm;          /* Clase de almacenamiento */

struct bloque *primer_tipo, *ultimo_tipo;

struct tipo
     *tipo_basico,      /* Tipo básico de la declaración actual */
     *t_primero,        /* Tipo procesado */
     *t_ultimo,         /* Parte final del tipo procesado */
     *t_schar,          /* Tipo signed char */
     *t_sshort,         /* Tipo signed short */
     *t_sint,           /* Tipo signed int o signed long */
     *t_uchar,          /* Tipo unsigned char */
     *t_ushort,         /* Tipo unsigned short */
     *t_uint,           /* Tipo unsigned int o unsigned long */
     *t_float,          /* Tipo float */
     *t_double,         /* Tipo double o long double */
     *t_void,           /* Tipo void */
     *t_achar,          /* Tipo apuntador a char */
     *t_awchar,         /* Tipo apuntador a wchar_t */
     *t_func;           /* Función que retorna int */

                        /* Tabla de nombres de estructuras */
struct rotulo *tabla_estruct[NUM_PRIMO];
                        /* Tabla de nombres de enumeradores */
struct enumerador *tabla_enum[NUM_PRIMO];

char *ap_c;             /* Apuntador de trabajo */
int *ap_e;              /* Apuntador de trabajo */
int usa_expr;           /* Indica si se usa el resultado de la expr. */

/*
** Nodo del árbol de expresiones
*/
struct nodo {
  int usos;             /* Cuantos apuntadores requieren este nodo */
  struct nodo **ap;     /* Apuntador al apuntador a este nodo */
  struct nodo *sig;     /* Siguiente nodo en la lista */
  struct nodo *izq;     /* Descendiente izquierdo */
  struct nodo *der;     /* Descendiente derecho */
  int oper;             /* Operación */
  int esp;              /* Especial (a veces otro descendiente) */
  int regs;             /* Total de registros requeridos para evaluarse */
};

struct nodo *subexpresion[NUM_PRIMO];  /* Tabla de dispersión de nodos */

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
** Observe como se puede efectuar fácilmente optimización de las
** construcciones del tipo:
**
**    if (0)
**      xxxx;
*/
struct sentencia {
  struct sentencia *sig;        /* Siguiente sentencia en la lista */
  enum tipo_sentencia tipo;     /* Tipo de sentencia */
  union {
    struct {                    /* Esta unión es para los if */
      struct nodo *expresion;
      struct sentencia *lista1;
      struct sentencia *lista2;
    } t_if;
    struct {                    /* Esta unión sirve para while, do y switch */
      struct nodo *expresion;
      struct sentencia *lista;
      int etiqueta_break;
      int etiqueta_continue;    /* Un 0 indica que continue no es válido */
    } t_while;
    struct {                    /* Esta unión es para for */
      struct nodo *expresion1;
      struct nodo *expresion2;
      struct nodo *expresion3;
      struct sentencia *lista;
      int etiqueta_break;
      int etiqueta_continue;
    } t_for;
    struct {                 /* Al generar el codigo, se buscan todos los */
      struct sentencia *sig_case;
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

struct sentencia_if {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct nodo *expresion;
  struct sentencia *lista1;
  struct sentencia *lista2;
};

struct sentencia_while {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct nodo *expresion;
  struct sentencia *lista;
  int etiqueta_break;
  int etiqueta_continue;
};

struct sentencia_for {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct nodo *expresion1;
  struct nodo *expresion2;
  struct nodo *expresion3;
  struct sentencia *lista;
  int etiqueta_break;
  int etiqueta_continue;
};

struct sentencia_case {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct sentencia *sig_case;
  int constante;
  int etiqueta;
};

struct sentencia_break {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  int etiqueta;
};

struct sentencia_return {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct nodo *expresion;
  int informacion;
};

struct sentencia_expr {
  struct sentencia *sig;
  enum tipo_sentencia tipo;
  struct nodo *expresion;
};

int variables_virtuales;   /* Total de variables virtuales */
int pila_temporal;         /* Describe el espacio asignado temporalmente */
                           /* en la pila */
struct sentencia *funcion; /* La función actual esta aquí */
struct nodo *ultimo_nodo = 0;  /* Ultimo nodo definido */
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

#define TEMP     20            /* Total de registros temporales disponibles */

int ultimo_temporal;           /* Último registro temporal asignado */
int temporales[TEMP];          /* Describe los registros temporales */
                               /* gr96 - ..., 1 = ocupado */
struct nodo *exp_temp[TEMP];   /* Describe el nodo contenido en un registro */

struct nombres *tabla[NUM_PRIMO];
struct nombres *locales;
struct macro *macros[NUM_PRIMO];

char linea[TAM_LINEA],         /* Buffer de análisis */
     linea_m[TAM_LINEA],       /* Buffer para el preproceso */
     lits[TAM_LITS],           /* Almacenamiento de cadenas literales */
     amacs[TAM_AMAC],          /* Buffer para argumentos de macros */
     camino_tarea[TAM_LINEA],  /* Camino a la tarea */
     nombre_archivo[TAM_LINEA],/* Archivo a compilar */
     nombre_salida[TAM_LINEA], /* Nombre del archivo de salida */
     opciones[TAM_LINEA],      /* Opciones extras de Fénix C */
     nombre_tipo[TAM_NOMBRE];  /* Nombre procesado con el tipo */

union {
  float valor_1;
  double valor;           /* Valor de la constante */
  int valor_2[2];
  unsigned
  char byte[TAM_DOUBLE];  /* Esto es dependiente del procesador destino */
} constan;

char *ventana;

/*
** Datos para el anidamiento de archivos
*/
struct archivo {              /* Archivo actual en compilación */
  struct archivo *anterior;   /* Apuntador al archivo que lo incluyó */
  int archivo;                /* Número de control */
  char *nombre_actual;        /* Nombre actual (modificable con __FILE__) */
  char *nombre_real;          /* Nombre real, mostrado en la ventana */
  int linea_actual;           /* Línea actual (modificable con __LINE__) */
  int linea_real;             /* Línea real, mostrada en la ventana */
};                            /* Nota: El nombre actual no incluye el camino */

struct archivo *archivo_actual;  /* El archivo actual en compilación */

enum comlex {

/* Palabras reservadas de C++ */
  C_ASM = 0, C_AUTO, C_BREAK, C_CASE, C_CATCH, C_CHAR, C_CLASS, C_CONST,
  C_CONTINUE, C_DEFAULT, C_DELETE, C_DO, C_DOUBLE, C_ELSE, C_ENUM, C_EXTERN,
  C_FLOAT, C_FOR, C_FRIEND, C_GOTO, C_IF, C_INLINE, C_INT, C_LONG,
  C_NEW, C_OPERATOR, C_PRIVATE, C_PROTECTED, C_PUBLIC, C_REGISTER, C_RETURN, C_SHORT,
  C_SIGNED, C_SIZEOF, C_STATIC, C_STRUCT, C_SWITCH, C_TEMPLATE, C_THIS, C_THROW,
  C_TRY, C_TYPEDEF, C_UNION, C_UNSIGNED, C_VIRTUAL, C_VOID, C_VOLATILE, C_WHILE,

/* Signos del lenguaje C++ */
  C_COMA, C_IGUAL, C_MASIGUAL, C_MENOSIGUAL, C_PORIGUAL, C_DIVIGUAL, C_MODIGUAL,
  C_IZQIGUAL, C_DERIGUAL, C_ANDIGUAL, C_XORIGUAL, C_ORIGUAL, C_TRINARIO,
  C_DPUNTOS, C_OROR, C_ANDAND, C_OR, C_XOR, C_AND, C_IGUALIGUAL, C_NOIGUAL,
  C_MENOR, C_MAYOR, C_MENORIGUAL, C_MAYORIGUAL, C_IZQ, C_DER, C_MAS, C_MENOS,
  C_MUL, C_DIV, C_MOD, C_MIEMBRO, C_APMIEMBRO, C_NOT, C_COM, C_INC, C_DEC,
  C_PARENI, C_PAREND, C_CORCHI, C_CORCHD, C_LLAVEI, C_LLAVED, C_APUNTA, C_PUNTO,
  C_ALCANCE, C_PCOMA, C_PUNTOS, C_PREPROC1, C_PREPROC2, C_NUM, C_NUMF,
  C_CAD, C_IDENT, C_ERROR, C_NULO
} clave_lex;

enum grupos {
  NINGUNO,     /* Ningún grupo */
  GRUPO_0,     /* Palabras reservadas */
  GRUPO_1,     /* Operadores de asignación */
  GRUPO_2,     /* == y != */
  GRUPO_3,     /* < > <= >= */
  GRUPO_4,     /* << >> */
  GRUPO_5,     /* + - */
  GRUPO_6,     /* * / % */
  GRUPO_7      /* .* ->* */
} grupo_lex;

int valor_lex;
struct tipo *tipo_lex;
char cad_lex[TAM_LINEA];

/*
** Tipos de operadores, usados en cada nodo de árboles de expresión.
*/
#define N_OR       1      /* OR binario */
#define N_XOR      2      /* XOR binario */
#define N_AND      3      /* AND binario */
#define N_IGUAL    4      /* Compara si es igual */
#define N_NOIGUAL  5      /* Compara si no es igual */
#define N_MAYOR    6      /* Compara si es mayor */
#define N_MAYORI   7      /* Compara si es mayor igual */
#define N_MENOR    8      /* Compara si es menor */
#define N_MENORI   9      /* Compara si es menor igual */
#define N_SMAYOR   10     /* Compara si es mayor sin signo */
#define N_SMAYORI  11     /* Compara si es mayor igual sin signo */
#define N_SMENOR   12     /* Compara si es menor sin signo */
#define N_SMENORI  13     /* Compara si es menor igual sin signo */
#define N_IGUALPF  14     /* Compara si es igual (punto flotante) */
#define N_MAYORPF  15     /* Compara si es mayor (punto flotante) */
#define N_MAYORIPF 16     /* Compara si es mayor igual (punto flotante) */
#define N_CD       17     /* Corrimiento a la derecha */
#define N_CI       18     /* Corrimiento a la izquierda */
#define N_SUMA     19     /* Suma */
#define N_RESTA    20     /* Resta */
#define N_AOR      21     /* |= */
#define N_AXOR     22     /* ^= */
#define N_AAND     23     /* &= */
#define N_ACI      24     /* <<= */
#define N_ACD      25     /* >>= */
#define N_ASUMA    26     /* += */
#define N_ARESTA   27     /* -= */
#define N_AMUL     28     /* *= */
#define N_ADIV     29     /* /= */
#define N_AMOD     30     /* %= */
#define N_ASIGNA   31     /* = */
#define N_MUL      32     /* Multiplicación */
#define N_DIV      33     /* División */
#define N_SDIV     34     /* División sin signo */
#define N_MOD      35     /* Resto */
#define N_SMOD     36     /* Resto sin signo */
#define N_NEG      37     /* Negación */
#define N_NEGPF    38     /* Negación (punto flotante) */
#define N_COM      39     /* Complemento binario */
#define N_INC      40     /* Preincremento */
#define N_PINC     41     /* Posincremento */
#define N_NOT      42     /* NOT boleano */
#define N_APFUNC   43     /* Obtiene un apuntador a una función */
#define N_CONST    44     /* Carga de una constante */
#define N_LIT      45     /* Carga la dirección de una cadena */
#define N_CBYTE    46     /* Lee un byte de la memoria */
#define N_CPAL     47     /* Lee una palabra de la memoria */
#define N_DIR      48     /* Dirección de una variable */
#define N_DIRG     49     /* Dirección de una variable global */
#define N_DIRE     50     /* Dirección de una variable estática */
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
#define N_PFENT    68     /* Convierte double a entero */
#define N_FUNC     69     /* Llamada a función */
#define N_FUNCI    70     /* Llamada a función indirecta */
#define N_PAR      71     /* Parametro de una función */
#define N_PARF     72     /* Parametro de una función (punto flotante) */
#define N_RESULTA  73     /* Reserva espacio para recibir una estructura */
#define N_RESTAI   74     /* Resta invertida */
#define N_NOR      75     /* Not OR bits */
#define N_NAND     76     /* Not AND bits */
#define N_NXOR     77     /* Not XOR bits */
#define N_CCHAR    78     /* Obtiene un signed char */

/*
** Prototipos de las funciones del compilador
*/
extern wchar_t *info_tarea(int);
extern int      lee_sistema(int);
extern void    *malloc(int);
extern void     free(void *);
extern void    *realloc(void *, int);
extern void    *crea_clase(wchar_t *, int (*func)(), void *, int, int, int);
extern void    *ventana_estandar(void *, void *, wchar_t *, int, int, int);


struct tipo *crea_tipo(int tipo);
void *pide_espacio(struct bloque **primero, struct bloque **ultimo,
                   int bytes);
void salva_contexto(struct bloque **ultimo, struct contexto *resultado);
void restaura_contexto(struct contexto *restaura);
void libera_espacio(struct bloque *primero);
void cancela(void);
void libera_memoria(void);
void reporta_errores(void);
void presentacion(void);
void ISO2wchar_t(wchar_t *destino, char *origen);
int interfaz(void *ventana, int mensaje, int par1, int par2);
void error(char *mensaje);
wchar_t *formatea_numero(wchar_t *destino, int numero);
void abre_salida(void);
void cierra_salida(void);

void analiza(void);
void decl_typedef(int local);
void decl_glb(int clase);
struct sentencia *decl_loc(void);
int p_tipo_1(int estilo);
void califica(int con, int vol);
void tipo_conf(void);
void almacena_conf(void);
void califica_conf(void);
int p_tipo_2(int nombre);
int p_tipo_3(int nombre);
int prototipo(void);
void encadena(int tipo);
int tam_arreglo(void);
void p_estructura(int es_union);
void p_enumerador(void);
int tam_tipo(struct tipo *tipo);
void inicializa(struct tipo *tipo);
int inic(struct tipo *tipo, int dentro);
void nueva_func(struct tipo *tipo, char *n_func, int parentesis);
void tipos_args(void);
void ordena_args(int cuantos);
struct sentencia *sentencia(void);
void punto_y_coma(void);
struct sentencia *p_bloque(void);
struct sentencia *s_if(void);
struct sentencia *s_while(void);
struct sentencia *s_do(void);
struct sentencia *s_for(void);
struct sentencia *s_switch(void);
struct sentencia *s_case(void);
struct sentencia *s_default(void);
struct sentencia *s_goto(void);
struct sentencia *p_etiqueta(void);
int agrega_etiqueta(char *nombre);
struct sentencia *s_return(void);
struct sentencia *s_break(void);
struct sentencia *s_continue(void);
int fin_sentencia(void);
void redefinido(char *nombre);
struct sentencia *nueva_sentencia(enum tipo_sentencia tipo);
void libera_sentencias(struct sentencia *lista);

int calcula_dispersion(char *cadena);
struct nombres *nueva_glb(char *nombre, int id, int clase,
                          struct tipo *tipo, int valor);
struct nombres *nueva_loc(char *nombre, int id, int clase,
                          struct tipo *tipo, int valor);
struct nombres *nueva_loc2(char *nombre, int id, int clase,
                           struct tipo *tipo, int valor);
struct rotulo *nueva_estructura(char *nombre);
struct miembro *nuevo_miembro(struct miembro **lista, char *nombre);
void nuevo_enum(char *nombre, int valor);
struct nombres *busca_glb(char *nombre);
struct nombres *busca_loc(char *nombre);
struct rotulo *busca_estructura(char *nombre);
struct miembro *busca_miembro(struct miembro *lista, char *nombre);
struct enumerador *busca_enum(char *nombre);
void mensaje(char *cad);
int obt_car(void);
void lee_linea(char *base);
void preprocesa(void);
int expr_preproc(void);
int expr_1(void);
int expr_2(void);
int expr_3(void);
int expr_4(void);
int expr_5(void);
int expr_6(void);
int expr_7(void);
int expr_8(void);
int expr_9(void);
int expr_10(void);
int expr_11(void);
int expr_12(void);
int expr_13(void);
int macro_especial(char *nombre);
void nuevo_archivo(char *archivo);
void p_include(void);
void fin_include(void);
void p_line(void);
void primer_paso(void);
void almacena_car(int c);
void pp_espacios(void);
void pp_comillas(void);
void pp_apostrofe(void);
void pp_comentarios(void);
void nueva_macro(void);
void borra_macro(char *nombre);
struct macro *busca_macro(char *nombre);
void hacia_consola(void);
void hacia_archivo(void);
void vacia_buffer(void);
int emite_car(int c);
void emite_nueva_linea(void);
void emite_linea(char *ap);
void emite_texto(char *ap);
void emite_numero(int numero);
int compara_cadenas(char *cad1, char *cad2);
void espacios(void);
int strlen(char *s);
void strcpy(char *destino, char *origen);
void strcat(char *destino, char *origen);
void obt_lex(void);
void p_ident(void);
void p_caracter(int tipo);
void p_cadena(int tipo);
void p_numero(void);
int caracter_literal(void);

int expr_constante(void);
struct tipo *almacena_expresion(int operador_coma);
int nivel0(struct expr *info, int k);
int nivel1(struct expr *info, int k);
int nivel2(struct expr *info, int k);
int nivel3(struct expr *info, int k);
int nivel4(struct expr *info, int k);
int nivel5(struct expr *info, int k);
int nivel6(struct expr *info, int k);
int nivel7(struct expr *info, int k);
int nivel8(struct expr *info, int k);
int nivel9(struct expr *info, int k);
int nivel9eval(int k, struct expr *info);
void nivel9op(struct nodo *izq, int k);
int nivel10(struct expr *info, int k);
int nivel11(struct expr *info, int k);
int nivel12(struct expr *info, int k);
int nivel13(struct expr *info, int nivel);
int nivel13ap(struct expr *info);
int nivel13dir(struct expr *info);
void nivel13inc(struct expr *info);
void nivel13dec(struct expr *info);
void nivel13pinc(struct expr *info);
void nivel13pdec(struct expr *info);
int primaria(struct expr *info, int sin_parentesis);
void req_valorl(void);
void llama_funcion(struct nombres *ap, struct tipo *tipo_funcion);
void carga_valor(struct expr *info);
void dir_var_loc(struct nombres *var);
void dir_var_glb(struct nombres *var);
void dir_func(struct nombres *ap);
int dobla(struct tipo *tipo, struct tipo *tipo2, struct nodo **nodo);
int constante(struct expr *info);
void checa_entero(struct tipo *tipo);
void checa_numerico(struct tipo *tipo);
void checa_entero_o_apuntador(struct tipo *tipo);
void compara_no_cero(struct tipo *tipo);
void compara_cero(struct tipo *tipo);
void convierte_tipo(struct nodo **nodo, struct tipo *tipo_original,
                                        struct tipo *nuevo_tipo, int advertir);
int compara_tipos(struct tipo *tipo1, struct tipo *tipo2);
void checa_comparable(struct tipo *tipo1, struct tipo *tipo2);
int haz_compatible(struct nodo **nodo_izq, struct expr *info_izq,
                   struct nodo **nodo_der, struct expr *info_der);
void crea_nodo(int op, struct nodo *izq, struct nodo *der, int val);
void libera_expr(void);
int es_potencia(int valor);

int var_virtual(int def, int tipo);
void gen_funcion(char *n_func, struct sentencia *lista);
void libera_temporales(void);
void prologo_funcion(void);
void epilogo_funcion(void);
void gen_sentencias(struct sentencia *lista);
void gen_if(struct sentencia *codigo);
void gen_while(struct sentencia *codigo);
void gen_do(struct sentencia *codigo);
void gen_for(struct sentencia *codigo);
void gen_switch(struct sentencia *codigo);
void gen_case(struct sentencia *codigo);
void gen_etiqueta(struct sentencia *codigo);
void gen_break(struct sentencia *codigo);
void gen_return(struct sentencia *codigo);
void gen_expresion(struct sentencia *codigo);
void prueba(int etiq, struct nodo *nodo);
void gen_codigo(int etiq, struct nodo *expr);
void etiqueta(struct nodo *nodo);
int en_registro(struct nodo *nodo);
int es_local(int reg);
int es_constante(struct nodo *nodo);
void gen_oper(int oper, int inmediato, int reg1, int reg2,
              int constreg, int control);
int pedir_reg(int pareja);
void libera_reg(int reg);
void gen_nodo(struct nodo *nodo, int resultado);
void checa_preinc(struct nodo *nodo, int bits);
void checa_posinc(struct nodo *nodo, int bits);
void nodo_preinc(struct nodo **nodo);
void nodo_posinc(struct nodo *nodo);
void gen_nodo1(struct nodo *nodo, int resultado);
void gen_nodo2(struct nodo *nodo, int resultado, int ajuste);
void gen_nodo3(struct nodo *nodo, int resultado);
void gen_nodo4(struct nodo *nodo, int resultado);
void anula_nodos(int reg);
int anula(struct nodo *nodo, int reg);
int existe_nodo(struct nodo *nodo);
int nodo_terminal(struct nodo *nodo);
void carga_reg(int tipo, int reg1, int reg2);
void carga_mem(int tipo, int reg1, int reg2);
void almacena_mem(int tipo, int reg1, int reg2);
void copia_resultado(struct nodo **nodo, int tam);
void prologo(void);
void epilogo(void);
void emite_nombre(char *nombre);
void llamada(struct nombres *ap);
void salto(int etiq);
void salta_expr(struct nodo *expr, int etiq, int forza_not);
void salta_si_falso(int etiq, int reg);
void salta_si_verdadero(int etiq, int reg);
void emite_etiq(int etiq);
void dos_puntos(void);
void def_byte(void);
void def_palabra(void);
void def_espacio(int val);
void def_global(char *nombre);
void compara_y_salta(int valor, int etiqueta);
void vacia_lits(void);
void gen_inst1(char *instruccion, int inmediato, int reg1, int reg2, int valreg);
void gen_inst2(char *instruccion, int inmediato, int reg1, int valreg);
void gen_inst3(char *instruccion, int inmediato, int reg1, int valreg);
void gen_inst4(int reg1, int reg2, char *pars);
void gen_destino(int etiq);
void gen_libre(int tipo);
int tipo_reg(int reg);
int max(int a, int b);
void emite_registro(int reg);
void segmento_cero(void);
void segmento_codigo(void);
