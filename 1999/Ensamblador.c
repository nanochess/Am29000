/*
** Ensamblador G11
**
** por Oscar Toledo Gutiérrez
**
** (c) Copyright Oscar Toledo G.1998.
**
** Creación: 13-may-1998.
** Revisión: 10-ago-1998. Se traslada al sistema de ventanas y se interfaza
**                        con Fénix C.
** Revisión: 13-ago-1998. Se agregan las directivas .ascii .asciil .hword
** Revisión: 15-ago-1998. Corrección de un defecto en el procesamiento de
**                        números hexadecimales.
** Revisión: 01-oct-1998. Se agrega la directiva .equ
** Revisión: 03-oct-1998. Permite etiquetas y instrucciones en la misma línea.
** Revisión: 08-oct-1998. Permite .align a tamaño.
** Revisión: 06-nov-1998. Nuevas directivas .bss y .text
** Revisión: 11-nov-1998. Corrección de defectos en procesamiento bss. La tabla
**                        de instrucciones ahora es un inicializador.
** Revisión: 18-nov-1998. Optimización con tablas de isdigit y tolower, también
**                        en ensamblador de strcpy, strlen, strcmp y strcat.
** Revisión: 19-nov-1998. Aceleración de la lectura de la entrada.
** Revisión: 02-dic-1998. Acumulación de las etiquetas y indefinidos en bloques,
**                        acelera el ensamblaje y reduce los requerimientos de
**                        memoria. 1811 líneas.
** Revisión: 12-dic-1998. Permite programas gigantescos gracias a una
**                        extensión para CALL y JMP. 1858 líneas.
*/

#define NULL           ((void *) 0)
#define FILE           int
#define SEEK_SET       0

#asm
.global _strlen
_strlen:
add gr96,lr2,0
_strlen1:
load 0,&14,gr97,gr96
exbyte gr97,gr97,0
cpeq gr98,gr97,0
jmpf gr98,_strlen1
add gr96,gr96,1
sub gr96,gr96,1
jmpi lr0
sub gr96,gr96,lr2

.global _strcat
_strcat:
load 0,&14,gr96,lr2
exbyte gr96,gr96,0
cpeq gr96,gr96,0
jmpf gr96,_strcat
add lr2,lr2,1
sub lr2,lr2,1
.global _strcpy
_strcpy:
load 0,&14,gr96,lr3
exbyte gr96,gr96,0
load 0,&14,gr97,lr2
inbyte gr97,gr97,gr96
store 0,4,gr97,lr2
add lr2,lr2,1
cpeq gr96,gr96,0
jmpf gr96,_strcpy
add lr3,lr3,1
jmpi lr0
nop

.global _strcmp
_strcmp:
load 0,&14,gr96,lr2
exbyte gr96,gr96,0
load 0,&14,gr97,lr3
exbyte gr97,gr97,0
cpeq gr98,gr96,gr97
jmpt gr98,_strcmp1
cplt gr98,gr96,gr97
jmpti gr98,lr0
const gr96,-1
jmpi lr0
const gr96,1
_strcmp1:
cpeq gr98,gr96,0
jmpti gr98,lr0
add lr2,lr2,1
jmp _strcmp
add lr3,lr3,1
#endasm

char _ctype[256] = {
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
  0x09, 0x09, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x40, 0x40, 0x40, 0x40, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
};

#define isalpha(x)  (_ctype[x] & 6)
#define isalnum(x)  (_ctype[x] & 7)
#define iscntrl(x)  (_ctype[x] & 48)
#define isdigit(x)  (_ctype[x] & 1)
#define isgraph(x)  (_ctype[x] & 71)
#define islower(x)  (_ctype[x] & 4)
#define isprint(x)  (_ctype[x] & 175)
#define ispunct(x)  (_ctype[x] & 64)
#define isspace(x)  (_ctype[x] & 160)
#define isupper(x)  (_ctype[x] & 2)
#define isxdigit(x) (_ctype[x] & 8)

char _tolower[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
  0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

#define tolower(x) (_tolower[x])

/*
** Este programa supone que int es de 32 bits.
*/

#define TAM_BLOQUE    8192  /* Tamaño asignado para bloques */

#define TAM_ARCHIVO    512  /* Tamaño máximo de un nombre de archivo */

#define TAM_LINEA      256  /* Tamaño máximo de la línea de entrada */

#define TAM_BUFFER    1024  /* Tamaño del buffer de entrada */

#define TAM_INFO       136  /* Tamaño de la información sobre las */
                            /* instrucciones */
#define TAM_TABLA      257  /* Tamaño de la tabla de dispersión. */
                            /* Un número primo es mejor. */
#define TAM_R          256  /* Total de reubicaciones en cada bloque */

#define TAM_INT          4  /* Tamaño en bytes de un entero */

struct etiqueta {           /* Estructura para almacenar etiquetas */
  struct etiqueta *sig;     /* Siguiente etiqueta */
  int tipo;                 /* Tipo de esta etiqueta */
                            /* 0 = Dirección en .text */
                            /* 1 = Dirección en .bss (reubicada al final) */
                            /* 2 = Valor */
  int valor;                /* Valor de esta etiqueta */
  char nombre[TAM_INT];     /* Nombre de la etiqueta */
};

struct indefinido {         /* Estructura para almacenar indefinidos */
  struct indefinido *sig;   /* Siguiente indefinido */
  int tipo;                 /* Tipo de indefinido */
                            /* 0 = .word */
                            /* 1 = Salto */
                            /* 2 = CONST */
                            /* 3 = CONSTH o CONSTHZ */
                            /* El número de línea esta codificado en los */
                            /* bits 30-3 */
  int posicion;             /* Posición para la corrección */
  char expresion[TAM_INT];  /* Expresión por evaluar */
};

struct reubicar {           /* Estructura para informar reubicación */
  struct reubicar *sig;     /* Siguiente reubicación */
  int sig_espacio;          /* Indica siguiente espacio en bloque */
  int posicion[TAM_R];      /* Los dos bits inferiores indican tipo de */
                            /* ajuste:                                 */
                            /*                                         */
                            /* 0= Apuntador a CONST y CONSTH           */
                            /* 1= Apuntador a .word                    */
                            /*                                         */
};

struct bloque {             /* Estructura para asignación de memoria */
  struct bloque *sig;       /* Siguiente bloque */
  int pos;                  /* Siguiente posición disponible */
  char datos[TAM_BLOQUE];   /* Datos contenidos */
};

/*
** Tipos de instrucción:
**  0 = Nada
**  1 =    RC     RA   RB/I
**  2 =    VN     RA   RB/I
**  3 = I17..I10  RA  I9..I2
**  4 =     x     RA    RB
**  5 =    RC     RA    FS
**  6 =    RC      x   RB/I
**  7 =    RC     RA UI RND FD FS
**  8 =    RC     RA    RB
**  9 = FUNC ACN  RA    RB
** 10 =    RC     RA     x
** 11 = I17..I10   x  I9..I2
** 12 =     x      x    RB
** 13 = CE CNTL   RA   RB/I
** 14 =    RC      x  FMT ACN
** 15 =     x     RA  FMT ACN
** 16 =    RC     SA     x
** 17 =     x     SA    RB
** 18 = I17..I10  SA  I9..I2
** 19 = I17..I10  RA  I9..I2   CONST
** 20 = I17..I10  RA  I9..I2   CONSTH
*/

struct {
  char *nemonico;              /* Nemonico de la instrucción */
  int codigo;                  /* Codigo máquina */
  int tipo;                    /* Tipo de instrucción */
} instrucciones[TAM_INFO] = {  /* Instrucciones del ensamblador */
  ".align",            0, 64,
  ".ascii",            0, 69,
  ".asciil",           0, 70,
  ".bss",              0, 73,
  ".byte",             0, 65,
  ".equ",              0, 72,
  ".global",           0, 66,
  ".hword",            0, 71,
  ".space",            0, 67,
  ".text",             0, 74,
  ".word",             0, 68,
  "add",      0x14000000,  1,
  "addc",     0x1c000000,  1,
  "addcs",    0x18000000,  1,
  "addcu",    0x1a000000,  1,
  "adds",     0x10000000,  1,
  "addu",     0x12000000,  1,
  "and",      0x90000000,  1,
  "andn",     0x9c000000,  1,
  "aseq",     0x70000000,  2,
  "asge",     0x5c000000,  2,
  "asgeu",    0x5e000000,  2,
  "asgt",     0x58000000,  2,
  "asgtu",    0x5a000000,  2,
  "asle",     0x54000000,  2,
  "asleu",    0x56000000,  2,
  "aslt",     0x50000000,  2,
  "asltu",    0x52000000,  2,
  "asneq",    0x72000000,  2,
  "call",     0xa8000000,  3,
  "calli",    0xc8000000,  4,
  "class",    0xe6000000,  5,
  "clz",      0x08000000,  6,
  "const",    0x03000000, 19,
  "consth",   0x02000000, 20,
  "consthz",  0x05000000, 20,
  "convert",  0xe4000000,  7,
  "cpbyte",   0x2e000000,  1,
  "cpeq",     0x60000000,  1,
  "cpge",     0x4c000000,  1,
  "cpgeu",    0x4e000000,  1,
  "cpgt",     0x48000000,  1,
  "cpgtu",    0x4a000000,  1,
  "cple",     0x44000000,  1,
  "cpleu",    0x46000000,  1,
  "cplt",     0x40000000,  1,
  "cpltu",    0x42000000,  1,
  "cpneq",    0x62000000,  1,
  "dadd",     0xf1000000,  8,
  "ddiv",     0xf7000000,  8,
  "deq",      0xeb000000,  8,
  "dge",      0xef000000,  8,
  "dgt",      0xed000000,  8,
  "div",      0x6a000000,  1,
  "div0",     0x68000000,  6,
  "divide",   0xe1000000,  8,
  "dividu",   0xe3000000,  8,
  "divl",     0x6c000000,  1,
  "divrem",   0x6e000000,  1,
  "dmac",     0xd9000000,  9,
  "dmsm",     0xdb000000,  8,
  "dmul",     0xf5000000,  8,
  "dsub",     0xf3000000,  8,
  "emulate",  0xd7000000,  2,
  "exbyte",   0x0a000000,  1,
  "exhw",     0x7c000000,  1,
  "exhws",    0x7e000000, 10,
  "extract",  0x7a000000,  1,
  "fadd",     0xf0000000,  8,
  "fdiv",     0xf6000000,  8,
  "fdmul",    0xf9000000,  8,
  "feq",      0xea000000,  8,
  "fge",      0xee000000,  8,
  "fgt",      0xec000000,  8,
  "fmac",     0xd8000000,  9,
  "fmsm",     0xda000000,  8,
  "fmul",     0xf4000000,  8,
  "fsub",     0xf2000000,  8,
  "halt",     0x89000000,  0,
  "inbyte",   0x0c000000,  1,
  "inhw",     0x78000000,  1,
  "inv",      0x9f000000,  0,
  "iret",     0x89000000,  0,
  "iretinv",  0x8c000000,  0,
  "jmp",      0xa0000000, 11,
  "jmpf",     0xa4000000,  3,
  "jmpfdec",  0xb4000000,  3,
  "jmpfi",    0xc4000000,  4,
  "jmpi",     0xc0000000, 12,
  "jmpt",     0xac000000,  3,
  "jmpti",    0xcc000000,  4,
  "load",     0x16000000, 13,
  "loadl",    0x06000000, 13,
  "loadm",    0x36000000, 13,
  "loadset",  0x26000000, 13,
  "mfacc",    0xe9000100, 14,
  "mfsr",     0xc6000000, 16,
  "mftlb",    0xb6000000, 10,
  "mtacc",    0xe8010000, 15,
  "mtsr",     0xce000000, 17,
  "mtsrim",   0x04000000, 18,
  "mttlb",    0xbe000000,  4,
  "mul",      0x64000000,  1,
  "mull",     0x66000000,  1,
  "multiplu", 0xe2000000,  8,
  "multiply", 0xe0000000,  8,
  "multm",    0xde000000,  8,
  "multmu",   0xdf000000,  8,
  "mulu",     0x74000000,  1,
  "nand",     0x9a000000,  1,
  "nop",      0x70400101,  0,
  "nor",      0x98000000,  1,
  "or",       0x92000000,  1,
  "orn",      0xaa000000,  1,
  "setip",    0x9e000000,  8,
  "sll",      0x80000000,  1,
  "sqrt",     0xe5000000,  5,
  "sra",      0x86000000,  1,
  "srl",      0x82000000,  1,
  "store",    0x1e000000, 13,
  "storel",   0x0e000000, 13,
  "storem",   0x3e000000, 13,
  "sub",      0x24000000,  1,
  "subc",     0x2c000000,  1,
  "subcs",    0x28000000,  1,
  "subcu",    0x2a000000,  1,
  "subr",     0x34000000,  1,
  "subrc",    0x3c000000,  1,
  "subrcs",   0x38000000,  1,
  "subrcu",   0x3a000000,  1,
  "subrs",    0x30000000,  1,
  "subru",    0x32000000,  1,
  "subs",     0x20000000,  1,
  "subu",     0x22000000,  1,
  "xnor",     0x96000000,  1,
  "xor",      0x94000000,  1,
};

struct bloque *binicio, *bfinal;    /* Memoria ocupada para etiquetas */
struct etiqueta *tabla[TAM_TABLA];  /* Tabla de dispersión para etiquetas */
struct etiqueta *etiqueta_reciente; /* Etiqueta más recientemente definida */
struct indefinido *lista_indef;     /* Lista de indefinidos */
struct indefinido *ultimo_indef;    /* Indefinido más reciente */
struct reubicar *lista_reubica;     /* Lista de reubicaciones */
struct reubicar *ultimo_bloque;     /* Ultimo bloque de reubicaciones */

int total_instrucciones;    /* Total de instrucciones definidas */
int codigo;                 /* Codigo de la instrucción actual */
int pos_actual;             /* Posición en bytes */

int segmento_actual;        /* Segmento actual */
int posicion_salvada;       /* Posición en bytes salvada */
int final_codigo;           /* Posición del final de codigo */

char *pos_buffer;                   /* Posición en buffer */
char buffer_entrada[TAM_BUFFER + 1];/* Buffer de entrada */

char archivo_entrada[TAM_ARCHIVO];  /* Archivo de entrada */
char archivo_libreria[TAM_ARCHIVO]; /* Archivo de libreria */
char archivo_salida[TAM_ARCHIVO];   /* Archivo de salida */
char archivo_actual[TAM_ARCHIVO];   /* Nombre de archivo actual */
char linea_entrada[TAM_LINEA];      /* Una línea de la entrada */
int pos_linea;                      /* Posición en la línea */
char componente[TAM_LINEA];         /* Un componente de la entrada */
char etiqueta[TAM_LINEA];           /* Un etiqueta en expresión */
int total_ensamblado;       /* Total de líneas ensambladas */
int total_reubicacion;      /* Total de reubicaciones */
int ultima_reubicacion;     /* Última posición de reubicación */
int tipo_indef;             /* Tipo de indefinido */
int pos_indef;              /* Posición de indefinido */
char *pos_expr;             /* Posición en expresión */
int opciones;               /* Opciones de ensamblaje */
int tam_pila_local;         /* Tamaño de la pila local */
int tam_pila_global;        /* Tamaño de la pila global */
int errores;                /* Total de errores localizados */
void *ventana;              /* Ventana del ensamblador */
int total_todo;             /* Total de líneas de todos los archivos */

int entrada;                /* Archivo de entrada */
int salida;                 /* Archivo de salida */

typedef unsigned short wchar_t;

/*
** !!! Debe soportar "Advertencias", que son pequeños detalles que no
**     impiden que el ensamblaje prosiga.
** !!! Todos los errores y advertencias deben ser acumulados a una pequeña
**     ventana dependiente del editor (Fénix C), para que el usuario pueda
**     tenerlos a la vista.
** !!! Debe permitir que multiples archivos se puedan ensamblar, anulando las
**     etiquetas que no son globales, hara falta un nuevo programa que se
**     encargará de mantener los proyectos y compilar lo necesario. (algo así
**     como un Makefile)
** !!! Debe permitir enlace con una libreria y sacar exactamente los módulos
**     necesarios, hara falta un nuevo programa para mantener la libreria, y
**     quizas un nuevo programa para efectuar el enlace. (linking)
** !!! En un sistema de ventanas a veces es útil agregar archivos al final del
**     programa (icono de programa, gráficas, datos, presentaciones, etc.) que
**     no se cargan más que cuando son necesarios, puede hacer falta un nuevo
**     programa para el mantenimiento de "resources" o incluirlo con el
**     ensamblador o enlazador.
*/

extern wchar_t *info_tarea(int);
extern void *malloc(int);
extern void free(void *);

/*
** Este ensamblador genera codigo con información para reubicación.
*/
main()
{
  wchar_t *nombre_tarea;
  unsigned char *ap, *ap2;

  nombre_tarea = info_tarea(lee_sistema(0));
  while (*nombre_tarea && *nombre_tarea != 0x0001)
    ++nombre_tarea;
  if (*nombre_tarea++ == 0)
    return;
  ap = archivo_entrada;
  ap2 = archivo_salida;
  while (*nombre_tarea && *nombre_tarea != 0x0001) {
    *ap++ = *nombre_tarea;
    *ap2++ = *nombre_tarea++;
  }
  *ap = 0;
  *(ap2 - 2) = 0;
  if (*nombre_tarea++ == 0)
    return;
  ap = archivo_libreria;
  while (*nombre_tarea && *nombre_tarea != 0x0001)
    *ap++ = *nombre_tarea++;
  *ap = 0;
  if (*nombre_tarea++ == 0)
    return;
  opciones = *nombre_tarea++ - '0';
  nombre_tarea++;
  while (*nombre_tarea >= '0' && *nombre_tarea <= '9')
    tam_pila_local = tam_pila_local * 10 + (*nombre_tarea++ - '0');
  nombre_tarea++;
  while (*nombre_tarea >= '0' && *nombre_tarea <= '9')
    tam_pila_global = tam_pila_global * 10 + (*nombre_tarea++ - '0');

  salida = fopen(archivo_salida, "w+1");
  if (salida) {
    presentacion();
    total_instrucciones = 136;
    encabezado();
    ensambla(archivo_entrada);
    ensambla(archivo_libreria);
    genera_reubicacion();
    actualiza_indefinidos();
    fclose(salida);
    libera_espacio(binicio);
    if ((opciones & 1) == 0 && errores == 0)
      remove(archivo_entrada);
    if ((opciones & 2)      && errores == 0)
      inicia_tarea(archivo_salida, NULL);
  }
}

/*
** Presentación.
*/
presentacion()
{
  void *clase;
  void *raiz;
  wchar_t texto[32];

  raiz = (void *) lee_sistema(3);
  ISO2wchar_t(texto, "Ensamblador");
  clase = (void *) crea_clase(texto, interfaz, NULL, 64, 0, 0);
  if (clase == NULL)
    aviso_error(1);
  ventana = (void *) ventana_estandar(clase, raiz, texto, 256, 128, 0x0401);
  if (ventana == NULL)
    aviso_error(1);
  multitarea();
}

ISO2wchar_t(destino, origen)
  wchar_t *destino;
  unsigned char *origen;
{
  while (*destino++ = *origen++) ;
}

wchar_t *formatea_numero(destino, numero)
  wchar_t *destino;
  int numero;
{
  if (numero >= 10)
    destino = formatea_numero(destino, numero / 10);
  *destino = (numero % 10) + '0';
  return destino + 1;
}

interfaz(ventana, mensaje, par1, par2)
  char *ventana;
  int mensaje, par1, par2;
{
  struct reubicar *lista, *temp;

  if (mensaje == 0x02) {
    wchar_t texto[TAM_LINEA], *ap;

    sel_color(ventana, lee_sistema(0x3d));
    rellena(ventana, par1 & 0xffff, (par1 >> 16) & 0xffff,
                     (par2 - par1) & 0xffff, ((par2 - par1) >> 16) & 0xffff);
    sel_color(ventana, lee_sistema(0x3e));
    sel_tipo(ventana, lee_sistema(0x9c));
    ISO2wchar_t(texto, "Ensamblando...");
    ilustra_texto(ventana, texto, 4, 48);
    sel_tipo(ventana, lee_sistema(0x9e));
    ISO2wchar_t(texto, archivo_actual);
    ilustra_texto(ventana, texto, 4, 64);
    ap = formatea_numero(texto, total_ensamblado);
    ISO2wchar_t(ap, " líneas ensambladas");
    ilustra_texto(ventana, texto, 4, 88);
    ap = formatea_numero(texto, total_todo);
    ISO2wchar_t(ap, " líneas en total");
    ilustra_texto(ventana, texto, 4, 104);
  } else if (mensaje == 0x22) {
    lista = lista_reubica;
    while (lista != NULL) {
      temp = lista->sig;
      free(lista);
      lista = temp;
    }
    fclose(salida);
    fclose(entrada);
    libera_espacio(binicio);
    exit(1);
  }
  return 1;
}

error(mensaje)
  unsigned char *mensaje;
{
  wchar_t texto[200], *ap;

  if (errores++)
    return;
  sel_color(ventana, lee_sistema(0x3e));
  sel_tipo(ventana, lee_sistema(0x9e));
  ISO2wchar_t(texto, "Línea ");
  ap = texto + 6;
  ap = formatea_numero(texto, total_ensamblado);
  *ap++ = ':';
  *ap++ = ' ';
  ISO2wchar_t(ap, mensaje);
  ilustra_texto(ventana, texto, 4, 124);
}

void *pide_espacio(struct bloque **primero, struct bloque **ultimo,
                   int bytes)
{
  void *apuntador;
  struct bloque *nuevo;

  if (*ultimo == NULL) {
    if ((*primero = *ultimo = malloc(sizeof(struct bloque))) == NULL)
      return NULL;
    (*primero)->pos = 0;
    (*primero)->sig = NULL;
  }
  while (1) {
    if ((*ultimo)->pos + bytes <= TAM_BLOQUE) {
      apuntador = (*ultimo)->datos + (*ultimo)->pos;
      (*ultimo)->pos += (bytes + 3) & ~3;
      return apuntador;
    }
    nuevo = malloc(sizeof(struct bloque));
    if (nuevo == NULL)
      return NULL;
    (*ultimo)->sig = nuevo;
    nuevo->sig = NULL;
    nuevo->pos = 0;
    *ultimo = nuevo;
  }
}

void libera_espacio(struct bloque *primero)
{
  struct bloque *sig;

  while (primero != NULL) {
    sig = primero->sig;
    free(primero);
    primero = sig;
  }
}

/*
** Separa un componente de la entrada.
*/
char *separa_componente()
{
  char *ap1 = linea_entrada + pos_linea;
  char *ap2 = componente;

  while (isspace(*ap1))
    ap1++;
  if (*ap1 == ';') {
    *ap2 = 0;
    return ap2;
  }
  while (*ap1 && !isspace(*ap1) && *ap1 != ',')
    *ap2++ = *ap1++;
  *ap2 = 0;
  while (isspace(*ap1))
    ap1++;
  pos_linea = ap1 - linea_entrada;
  if (ap2 != componente)
    return ap2 - 1;
  else
    return ap2;
}

/*
** Busca una etiqueta
*/
struct etiqueta *busca_etiqueta(etiqueta)
  char *etiqueta;
{
  struct etiqueta *lista = tabla[calcula_dispersion(etiqueta)];

  while (lista != NULL) {
    if (strcmp(etiqueta, lista->nombre) == 0)
      return lista;
    lista = lista->sig;
  }
  return NULL;
}

/*
** Muestra total de líneas ensambladas.
*/
muestra_lineas()
{
  mensaje_urgente(ventana, 2, 0x00480000, 0x006bffff);
  multitarea();
}

/*
** Define una etiqueta
*/
define_etiqueta()
{
  int lista;
  struct etiqueta *nueva;

  if (busca_etiqueta(componente) != NULL) {
    error("Etiqueta redefinida");
    return;
  }
  nueva = pide_espacio(&binicio, &bfinal, sizeof(struct etiqueta) + strlen(componente) - (TAM_INT - 1));
  if (nueva == NULL) {
    error("No hay memoria para etiqueta");
    return;
  }
  lista = calcula_dispersion(componente);
  nueva->sig = tabla[lista];
  nueva->tipo = segmento_actual;
  nueva->valor = pos_actual;
  strcpy(nueva->nombre, componente);
  tabla[lista] = nueva;
  etiqueta_reciente = nueva;
}

/*
** Busca una instrucción o directiva.
*/
busca_instruccion()
{
  int min, max, actual, resultado;
  char *ap;

  ap = componente;
  while (*ap = tolower(*ap))
    ap++;
  min = 0;                           /* Efectua una busqueda binaria */
  max = total_instrucciones - 1;
  while (min <= max) {
    actual = (min + max) >> 1;
    resultado = strcmp(instrucciones[actual].nemonico, componente);
    if (resultado == -1)
      min = actual + 1;
    else if (resultado == 1)
      max = actual - 1;
    else
      break;
  }
  if (min <= max) {
    if (instrucciones[actual].tipo >= 64) {  /* Es una directiva */
      codigo = 0;
      procesa_directiva(instrucciones[actual].tipo);
    } else {                                 /* Es una instrucción */
      if (segmento_actual == 1)
        error("Instrucción en bss");
      codigo = instrucciones[actual].codigo;
      procesa_instruccion(instrucciones[actual].tipo, &codigo);
      escribe_palabra(codigo);
    }
  } else {
    if (*componente == '.')
      error("Directiva desconocida");
    else
      error("Instrucción desconocida");
  }
}

/*
** Procesa una directiva
*/
procesa_directiva(tipo)
  int tipo;
{
  int a;
  char *ap;

  if (tipo == 64) {             /* .align */
    separa_componente();
    if (*componente)
      a = procesa_expr2(0, -1);
    else
      a = 4;
    if (segmento_actual == 1)
      pos_actual = ((pos_actual + a - 1) / a) * a;
    else {
      while (pos_actual % a)
        escribe_byte(0);
    }
    return;
  }
  if (tipo == 66) {             /* .global */
    return;
  }
  if (tipo == 67) {             /* .space */
    a = procesa_expr(0, -1);
    if (segmento_actual == 1)
      pos_actual += a;
    else {
      while (a--)
        escribe_byte(0);
    }
    return;
  }
  if (tipo == 73) {             /* .bss */
    if (segmento_actual == 0) {
      a = pos_actual;
      pos_actual = posicion_salvada;
      posicion_salvada = a;
      segmento_actual = 1;
    }
    return;
  }
  if (tipo == 74) {             /* .text */
    if (segmento_actual == 1) {
      a = pos_actual;
      pos_actual = posicion_salvada;
      posicion_salvada = a;
      segmento_actual = 0;
    }
    return;
  }
  if (segmento_actual)
    error("Directiva incorrecta en bss");
  if (tipo == 65) {             /* .byte */
    while (1) {
      a = procesa_expr(0, -1);
      escribe_byte(a);
      if (linea_entrada[pos_linea] != ',') {
        checa_nada();
        return;
      }
      pos_linea++;
    }
  }
  if (tipo == 68) {             /* .word */
    while (1) {
      a = procesa_expr(0, pos_actual);
      escribe_palabra(a);
      if (linea_entrada[pos_linea] != ',') {
        checa_nada();
        return;
      }
      pos_linea++;
    }
  }
  if (tipo == 69) {             /* .ascii */
    ap = linea_entrada + pos_linea;
    if (*ap++ == '"') {
      while (*ap && *ap != '"')
        escribe_byte(*ap++);
    } else
      error("Faltan comillas");
    return;
  }
  if (tipo == 70) {             /* .asciil */
    ap = linea_entrada + pos_linea;
    if (*ap++ == '"') {
      while (*ap && *ap != '"') {
        escribe_byte(*ap++);
        escribe_byte(0);
      }
    } else
      error("Faltan comillas");
    return;
  }
  if (tipo == 71) {             /* .hword */
    while (1) {
      a = procesa_expr(0, pos_actual);
      escribe_byte(a);
      escribe_byte(a >> 8);
      if (linea_entrada[pos_linea] != ',') {
        checa_nada();
        return;
      }
      pos_linea++;
    }
  }
  if (tipo == 72) {             /* .equ */
    a = procesa_expr(0, -1);
    if (etiqueta_reciente != NULL) {
      etiqueta_reciente->tipo = 2;
      etiqueta_reciente->valor = a;
    }
    return;
  }
  error("Error interno");
}

/*
** Procesa una instrucción
*/
procesa_instruccion(tipo, codigo)
  int tipo, codigo;
{
  switch (tipo) {
    case 0 : checa_nada();
             return;
    case 1 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             checa_nada();
             return;
    case 2 : procesa_vn(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             checa_nada();
             return;
    case 3 : procesa_ra(codigo);
             checa_coma();
    case 11: procesa_dir(codigo);
             return;
    case 5 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_fs(codigo);
             checa_nada();
             return;
    case 6 : procesa_rc(codigo);
             checa_coma();
             procesa_rbi(codigo);
             checa_nada();
             return;
    case 7 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_ui(codigo);
             checa_coma();
             procesa_rnd(codigo);
             checa_coma();
             procesa_fd(codigo);
             checa_coma();
             procesa_fs(codigo);
             checa_nada();
             return;
    case 8 : procesa_rc(codigo);
             checa_coma();
    case 4 : procesa_ra(codigo);
             checa_coma();
    case 12: procesa_rb(codigo);
             checa_nada();
             return;
    case 9 : procesa_funcacn(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rb(codigo);
             checa_nada();
             return;
    case 10: procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_nada();
             return;
    case 13: procesa_ce(codigo);
             checa_coma();
             procesa_cntl(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             checa_nada();
             return;
    case 14: procesa_rc(codigo);
             checa_coma();
             procesa_fmt(codigo);
             checa_coma();
             procesa_acn(codigo);
             checa_nada();
             return;
    case 15: procesa_ra(codigo);
             checa_coma();
             procesa_fmt(codigo);
             checa_coma();
             procesa_acn(codigo);
             checa_nada();
             return;
    case 16: procesa_rc(codigo);
             checa_coma();
             procesa_sa(codigo);
             checa_nada();
             return;
    case 17: procesa_sa(codigo);
             checa_coma();
             procesa_rb(codigo);
             checa_nada();
             return;
    case 18: procesa_sa(codigo);
             checa_coma();
             procesa_const(codigo, 0);
             checa_nada();
             return;
    case 19: procesa_ra(codigo);
             checa_coma();
             procesa_const(codigo, 1);
             checa_nada();
             return;
    case 20: procesa_ra(codigo);
             checa_coma();
             procesa_const(codigo, 2);
             checa_nada();
             return;
    default: error("Error interno");
             return;
  }
}

checa_coma()
{
  if (linea_entrada[pos_linea] != ',')
    error("Falta coma");
  else
    pos_linea++;
}

checa_nada()
{
  if (linea_entrada[pos_linea])
    error("Caracteres ignorados");
}

int checa_registro(que_hacer)
  int que_hacer;
{
  int a, num = -1;
  char *ap;

  separa_componente();
  a = tolower(componente[0]);
  if (a == 'g' || a == 'l') {
    if (tolower(componente[1]) == 'r') {
      if (isdigit(componente[2])) {
        ap = componente + 2;
        num = 0;
        while (*ap) {
          if (isdigit(*ap))
            num = (num * 10) + (*ap++ - '0');
          else {
            num = -1;
            break;
          }
        }
        if (num > 127)
          num = -1;
        if (a == 'l' && num >= 0)
          num += 128;
      }
    }
  }
  if (que_hacer == 0 && num == -1)
    error("Se requiere registro");
  return num;
}

procesa_rc(codigo)
  int *codigo;
{
  int a;

  if ((a = checa_registro(0)) >= 0)
    *codigo |= a << 16;
}

procesa_ra(codigo)
  int *codigo;
{
  int a;

  if ((a = checa_registro(0)) >= 0)
    *codigo |= a << 8;
}

procesa_rb(codigo)
  int *codigo;
{
  int a;

  if ((a = checa_registro(0)) >= 0)
    *codigo |= a;
}

procesa_rbi(codigo)
  int *codigo;
{
  int a;

  if ((a = checa_registro(1)) >= 0)
    *codigo |= a;
  else {
    a = procesa_expr2(0, -1);
    if (a < 0 || a > 255)
      error("Constante fuera de rango");
    *codigo |= 0x01000000 | (a & 255);
  }
}

procesa_vn(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 255)
    error("Constante fuera de rango");
  *codigo |= (a & 255) << 16;
}

procesa_ui(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 1) {
    error("Solo se acepta 0 o 1 en UI");
    return;
  }
  *codigo |= a << 7;
}

procesa_rnd(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 7) {
    error("Solo se acepta 0-7 en RND");
    return;
  }
  *codigo |= a << 4;
}

procesa_fd(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 3) {
    error("Solo se acepta 0-3 en FD");
    return;
  }
  *codigo |= a << 2;
}

procesa_fs(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 3) {
    error("Solo se acepta 0-3 en FS");
    return;
  }
  *codigo |= a;
}

procesa_ce(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 1) {
    error("Solo se acepta 0 ó 1 en CE");
    return;
  }
  *codigo |= a << 23;
}

procesa_cntl(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 127) {
    error("Solo se acepta 0-127 en CNTL");
    return;
  }
  *codigo |= a << 16;
}

procesa_funcacn(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 15)
    error("Solo se acepta 0-15 en FUNC");
  else
    *codigo |= a << 18;
  checa_coma();
  a = procesa_expr(0, -1);
  if (a < 0 || a > 3)
    error("Solo se acepta 0-3 en ACN");
  else
    *codigo |= a << 16;
}

procesa_fmt(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 3)
    error("Solo se acepta 0-3 en FMT");
  else
    *codigo |= a << 2;
}

procesa_acn(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
  if (a < 0 || a > 3)
    error("Solo se acepta 0-3 en ACN");
  else
    *codigo |= a;
}

procesa_sa(codigo)
  int *codigo;
{
  int reg;
  char *ap;

  separa_componente();
  ap = componente;
  while (*ap = tolower(*ap))
    ap++;
  if (strcmp("vab", componente) == 0)
    reg = 0;
  else if (strcmp("q", componente) == 0)
    reg = 131;
  else if (strcmp("ops", componente) == 0)
    reg = 1;
  else if (strcmp("cps", componente) == 0)
    reg = 2;
  else if (strcmp("cfg", componente) == 0)
    reg = 3;
  else if (strcmp("cha", componente) == 0)
    reg = 4;
  else if (strcmp("chd", componente) == 0)
    reg = 5;
  else if (strcmp("chc", componente) == 0)
    reg = 6;
  else if (strcmp("rbp", componente) == 0)
    reg = 7;
  else if (strcmp("tmc", componente) == 0)
    reg = 8;
  else if (strcmp("tmr", componente) == 0)
    reg = 9;
  else if (strcmp("pc0", componente) == 0)
    reg = 10;
  else if (strcmp("pc1", componente) == 0)
    reg = 11;
  else if (strcmp("pc2", componente) == 0)
    reg = 12;
  else if (strcmp("mmu", componente) == 0)
    reg = 13;
  else if (strcmp("lru", componente) == 0)
    reg = 14;
  else if (strcmp("rsn", componente) == 0)
    reg = 15;
  else if (strcmp("rma0", componente) == 0)
    reg = 16;
  else if (strcmp("rmc0", componente) == 0)
    reg = 17;
  else if (strcmp("rma1", componente) == 0)
    reg = 18;
  else if (strcmp("rmc1", componente) == 0)
    reg = 19;
  else if (strcmp("spc0", componente) == 0)
    reg = 20;
  else if (strcmp("spc1", componente) == 0)
    reg = 21;
  else if (strcmp("spc2", componente) == 0)
    reg = 22;
  else if (strcmp("iba0", componente) == 0)
    reg = 23;
  else if (strcmp("ibc0", componente) == 0)
    reg = 24;
  else if (strcmp("iba1", componente) == 0)
    reg = 25;
  else if (strcmp("ibc1", componente) == 0)
    reg = 26;
  else if (strcmp("ipc", componente) == 0)
    reg = 128;
  else if (strcmp("ipa", componente) == 0)
    reg = 129;
  else if (strcmp("ipb", componente) == 0)
    reg = 130;
  else if (strcmp("alu", componente) == 0)
    reg = 132;
  else if (strcmp("bp", componente) == 0)
    reg = 133;
  else if (strcmp("fc", componente) == 0)
    reg = 134;
  else if (strcmp("cr", componente) == 0)
    reg = 135;
  else if (strcmp("fpe", componente) == 0)
    reg = 160;
  else if (strcmp("inte", componente) == 0)
    reg = 161;
  else if (strcmp("fps", componente) == 0)
    reg = 162;
  else if (strcmp("exop", componente) == 0)
    reg = 164;
  else
    error("Registro especial desconocido");
  *codigo |= reg << 8;
}

procesa_const(codigo, clase)
  int *codigo, clase;
{
  int a;
  struct indefinido *b = ultimo_indef;

  a = procesa_expr((clase >= 2) ? 3 : 2, pos_actual);
  if (b != ultimo_indef)
    return;
  if (clase == 1 && a >= -65536 && a <= -1) {
    *codigo &= ~ 0x02000000;
    a += 65536;
  }
  if (clase == 2 || clase == 3)
    a >>= 16;
  a &= 0xffff;
  *codigo |= (a & 0xff);
  *codigo |= ((a >> 8) & 0xff) << 16;
}

procesa_dir(codigo)
  int *codigo;
{
  int a;
  struct indefinido *b = ultimo_indef;

  a = procesa_expr(1, pos_actual);
  if (b != ultimo_indef) {
    if (linea_entrada[pos_linea]) {
      b = ultimo_indef;
      checa_coma();
      a = procesa_expr(1, pos_actual);
      if (b != ultimo_indef)
        return;
    } else
      return;
  }
  a = (a - pos_actual) >> 2;
  if (a < -32768 || a > 32767) { /* Procesa llamada extendida */
    if (linea_entrada[pos_linea]) {
      checa_coma();
      a = procesa_expr(1, pos_actual);
      if (b != ultimo_indef)
        return;
      a = (a - pos_actual) >> 2;
    }
  }
  if (a < -32768 || a > 32767)
    error("Llamada fuera de rango");
  a &= 0xffff;
  *codigo |= (a & 0xff);
  *codigo |= ((a >> 8) & 0xff) << 16;
}

procesa_expr(tipo, indef)
  int tipo, indef;
{
  separa_componente();
  return procesa_expr2(tipo, indef);
}

procesa_expr2(tipo, indef)
  int tipo, indef;
{
  int definido = 1;
  int valor;
  struct indefinido *nuevo;

  tipo_indef = tipo;
  pos_indef = indef;
  pos_expr = componente;
  valor = procesa_n1(&definido);
  if (*pos_expr)
    error("Caracteres extras en expresión");
  if (definido)
    return valor;
  else {
    nuevo = pide_espacio(&binicio, &bfinal, sizeof(struct indefinido) +
                   strlen(componente) - (TAM_INT - 1));
    if (nuevo == NULL)
      error("No hay espacio para etiqueta indefinida");
    else {
      nuevo->sig = NULL;
      nuevo->tipo = tipo_indef | (total_ensamblado << 3);
      nuevo->posicion = pos_indef;
      strcpy(nuevo->expresion, componente);
      if (ultimo_indef != NULL)
        ultimo_indef->sig = nuevo;
      if (lista_indef == NULL)
        lista_indef = nuevo;
      ultimo_indef = nuevo;
    }
    return 0;
  }
}

procesa_n1(definido)
  int *definido;
{
  int valor;

  valor = procesa_n2(definido);
  if (hay("|"))
    valor |= procesa_n2(definido);
  return valor;
}

procesa_n2(definido)
  int *definido;
{
  int valor;

  valor = procesa_n3(definido);
  if (hay("^"))
    valor ^= procesa_n3(definido);
  return valor;
}

procesa_n3(definido)
  int *definido;
{
  int valor;

  valor = procesa_n4(definido);
  if (hay("&"))
    valor &= procesa_n4(definido);
  return valor;
}

procesa_n4(definido)
  int *definido;
{
  int valor;

  valor = procesa_n5(definido);
  if (hay("<<"))
    valor <<= procesa_n5(definido);
  else if (hay(">>"))
    valor >>= procesa_n5(definido);
  return valor;
}

procesa_n5(definido)
  int *definido;
{
  int valor;

  valor = procesa_n6(definido);
  if (hay("+"))
    valor += procesa_n6(definido);
  else if (hay("-"))
    valor -= procesa_n6(definido);
  return valor;
}

procesa_n6(definido)
  int *definido;
{
  int valor;

  valor = procesa_n7(definido);
  if (hay("*"))
    valor *= procesa_n7(definido);
  else if (hay("/"))
    valor /= procesa_n7(definido);
  else if (hay("%"))
    valor %= procesa_n7(definido);
  return valor;
}

procesa_n7(definido)
  int *definido;
{
  int valor, negar = 0;
  char *ap;
  struct etiqueta *temp;
  struct reubicar *bloque;

  if (hay("-"))
    negar = 1;
  if (*pos_expr == '&')
    valor = procesa_hex();
  else if (isdigit(*pos_expr))
    valor = procesa_num();
  else if (isalpha(*pos_expr) || *pos_expr == '_') {
    ap = etiqueta;
    while (isalnum(*pos_expr) || *pos_expr == '_')
      *ap++ = *pos_expr++;
    *ap = 0;
    temp = busca_etiqueta(etiqueta);
    if (temp == NULL) {
      if (pos_indef == -1)
        error("Se requiere constante");
      else if (pos_indef == -2) {
        ap = etiqueta;
        strcat(ap, " indefinido");
        error(etiqueta);
      } else
        *definido = 0;
    } else {
      valor = temp->valor;
      if (temp->tipo == 1 && final_codigo)
        valor += final_codigo;
      else if (temp->tipo == 1)
        *definido = 0;
    }
    if ((pos_indef >= 0 && (tipo_indef == 0 || tipo_indef == 2))
     && (temp == NULL || temp->tipo < 2) && (ultima_reubicacion != pos_actual)) {
      ultima_reubicacion = pos_actual;
      if (lista_reubica == NULL || ultimo_bloque->sig_espacio == TAM_R) {
        bloque = malloc(sizeof(struct reubicar));
        if (bloque == NULL)
          error("No hay memoria para lista de reubicación");
        else {
          bloque->sig = NULL;
          bloque->sig_espacio = 0;
          if (lista_reubica == NULL)
            lista_reubica = bloque;
          if (ultimo_bloque != NULL)
            ultimo_bloque->sig = bloque;
          ultimo_bloque = bloque;
        }
      }
      if (ultimo_bloque != NULL && ultimo_bloque->sig_espacio < TAM_R) {
        ultimo_bloque->posicion[ultimo_bloque->sig_espacio++] =
          pos_actual | (tipo_indef == 0 ? 1 : 0);
        total_reubicacion++;
      }
    }
  } else
    error("Expresión inválida");
  if (negar)
    valor = -valor;
  return valor;
}

procesa_hex()
{
  int valor = 0, car;

  pos_expr++;
  while (1) {
    car = tolower(*pos_expr);
    if (car >= '0' && car <= '9')
      valor = (valor << 4) + car - 0x30;
    else if (car >= 'a' && car <= 'f')
      valor = (valor << 4) + car - 0x57;
    else
      break;
    pos_expr++;
  }
  return valor;
}

procesa_num()
{
  int valor = 0;

  while (isdigit(*pos_expr))
    valor = (valor * 10) + (*pos_expr++ - '0');
  return valor;
}

hay(operador)
  char *operador;
{
  char *ap1 = pos_expr;

  while (*operador)
    if (*ap1++ != *operador++)
      return 0;
  pos_expr = ap1;
  return 1;
}

/*
** Lee una palabra y deja la posición como estaba.
*/
int lee_palabra(void)
{
  int valor;

  fread(salida, &valor, 4);
  fseek(salida, -4, 1);
  return valor;
}

/*
** Escribe una palabra en orden bajo-alto
*/
escribe_palabra(valor)
  int valor;
{
  fwrite(salida, &valor, 4);
  pos_actual += 4;
}

/*
** Escribe un byte
*/
escribe_byte(valor)
  int valor;
{
  fputc(valor, salida);
  pos_actual++;
}

/*
** Obtiene otra linea de la entrada.
*/
lee_linea()
{
  int k;

  while (1) {
    pos_linea = 0;
    while (1) {
      if (k = *pos_buffer++) {
        if (k == '\n' || pos_linea >= (TAM_LINEA - 1))
          break;
        if (k == ';') {
          while (1) {
            while (*pos_buffer && *pos_buffer != '\n')
              pos_buffer++;
            if (*pos_buffer == '\n')
              break;
            k = fread(entrada, buffer_entrada, TAM_BUFFER);
            if (k == 0) {
              buffer_entrada[0] = '\n';
              buffer_entrada[1] = 0;
            } else {
              buffer_entrada[k] = 0;
            }
            pos_buffer = buffer_entrada;
          }
        } else if (k != '\r')
          linea_entrada[pos_linea++] = k;
      } else {
        k = fread(entrada, buffer_entrada, TAM_BUFFER);
        if (k == 0) {
          k = -1;
          break;
        } else {
          buffer_entrada[k] = 0;
        }
        pos_buffer = buffer_entrada;
      }
    }
    total_todo++;
    total_ensamblado++;             /* Se ha leido una línea más */
    linea_entrada[pos_linea] = 0;   /* Agrega un caracter nulo */
    if (k < 0) {
      fclose(entrada);
      entrada = 0;
    }
    if (pos_linea || entrada == 0) {
      pos_linea = 0;
      return;
    }
  }
}

/*
** Cálcula la función de dispersión PJW
*/
int calcula_dispersion(etiqueta)
  char *etiqueta;
{
  int val = 0, temp;

  while (*etiqueta) {
    val = (val << 4) + *etiqueta++;
    if (temp = val & 0xf0000000) {
      val ^= (temp >> 24);
      val &= ~0xf0000000;
    }
  }
  return val % TAM_TABLA;
}

genera_reubicacion()
{
  struct reubicar *lista, *temp;
  int total;
  int pos_anterior;
  int pos_final;
  int a;

  while (pos_actual & 3)     /* Alineación de .bss a palabra */
    escribe_byte(0);
  if (segmento_actual == 1) {
    a = pos_actual;
    pos_actual = posicion_salvada;
    posicion_salvada = a;
    segmento_actual = 0;
  }
  posicion_salvada = (posicion_salvada + 3) & ~3;
  final_codigo = pos_actual;
  pos_anterior = pos_actual;
  lista = lista_reubica;
  while (lista != NULL) {
    for (total = 0; total < lista->sig_espacio; total++)
      escribe_palabra(lista->posicion[total]);
    temp = lista->sig;
    free(lista);
    lista = temp;
  }
  pos_final = pos_actual;
  fseek(salida, 8, SEEK_SET);
  escribe_palabra(pos_anterior);
  escribe_palabra(posicion_salvada);
  fseek(salida, 40, SEEK_SET);
  escribe_palabra(pos_final);
  escribe_palabra(total_reubicacion);
}

actualiza_indefinidos()
{
  struct indefinido *lista, *temp;
  int val;

  lista = lista_indef;
  while (lista != NULL) {
    strcpy(componente, lista->expresion);
    total_ensamblado = lista->tipo >> 3;
    val = procesa_expr2(0, -2);
    fseek(salida, lista->posicion, SEEK_SET);
    switch (lista->tipo & 7) {
      case 0:  escribe_palabra(val);
               break;
      case 1:  val = (val - lista->posicion) >> 2;
               if (val < -32768 || val > 32767) {
                 if (lista->sig == NULL
                  || lista->sig->posicion != lista->posicion)
                   error("Salto demasiado largo");
               } else {
                 if (lista->sig != NULL
                  && lista->sig->posicion == lista->posicion)
                   lista = lista->sig;
               }
      case 2:  codigo = lee_palabra() & 0xff00ff00;
               codigo |= (val & 255);
               codigo |= ((val >> 8) & 255) << 16;
               escribe_palabra(codigo);
               break;
      case 3:  val >>= 16;
               codigo = lee_palabra() & 0xff00ff00;
               codigo |= (val & 255);
               codigo |= ((val >> 8) & 255) << 16;
               escribe_palabra(codigo);
               break;
      default: error("Error interno del ensamblador");
               break;
    }
    lista = lista->sig;
  }
  lista_indef = NULL;
}

/*
** Encabezado de archivo ejecutable extendido
*/
encabezado()
{
  pos_actual = 0;
  segmento_actual = 0;
  posicion_salvada = 0;
  final_codigo = 0;
  ultima_reubicacion = -1;

  escribe_palabra(0xA0000010); /* JMP */
  escribe_palabra(0x70406060); /* NOP especial */
  escribe_palabra(0x00000000); /* Tamaño del codigo (text) */
  escribe_palabra(0x00000000); /* Tamaño de datos extras (bss) */
  escribe_palabra(tam_pila_local);  /* Tamaño de pila local */
  escribe_palabra(tam_pila_global); /* Tamaño de pila global */
  escribe_palabra(0x00000000);
  escribe_palabra(0x00000000);

  escribe_palabra(0x50524F47); /* "PROG" */
  escribe_palabra(0x07CE0512); /* 1998-may-18 */
  escribe_palabra(0x00000000); /* Offset de datos en el archivo */
  escribe_palabra(0x00000000); /* Total de reubicaciones */
  escribe_palabra(0x00000000); /* Información reservada */
  escribe_palabra(0x00000000); /* Información reservada */
  escribe_palabra(0x00000000); /* Información reservada */
  escribe_palabra(0x00000000); /* Información reservada */
}

/*
** Efectua el ensamblaje.
*/
ensambla(archivo)
  char *archivo;
{
  char *ap;

  total_ensamblado = 0;
  strcpy(archivo_actual, archivo);
  envia_mensaje(ventana, 2, 0x00320000, 0x0042ffff);
  entrada = fopen(archivo, "r");
  pos_buffer = buffer_entrada;
  *pos_buffer = 0;
  while (entrada) {
    lee_linea();
    if (entrada == 0)
      break;
    ap = separa_componente();
    while (*ap == ':') {
      *ap = 0;
      define_etiqueta();
      ap = separa_componente();
    }
    if (componente[0])
      busca_instruccion();
    if ((total_ensamblado & 127) == 0)
      muestra_lineas();
  }
  muestra_lineas();
}
