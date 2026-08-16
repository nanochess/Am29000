/*
** Ensamblador G11
**
** por Oscar Toledo Gutiérrez
**
** (c) Copyright Oscar Toledo G.1998.
**
** Creación: 13 de mayo de 1998.
*/

#include <stdio.h>

#define puts(a)  fputs(a, stdout)

/*
** Este programa supone que int es de 32 bits.
*/

#define TAM_LINEA      512  /* Máximo tamaño de la línea de entrada */

#define TAM_INFO       140  /* Tamaño de la información sobre las */
                            /* instrucciones */
#define TAM_TABLA      211  /* Tamaño de la tabla de dispersión. */
                            /* Un número primo es mejor. */
#define TAM_R          256  /* Total de reubicaciones en cada bloque */

struct etiqueta {           /* Estructura para almacenar etiquetas */
  struct etiqueta *sig;     /* Siguiente etiqueta */
  int tipo;                 /* Tipo de esta etiqueta */
                            /* 0 = Dirección */
                            /* 1 = Valor */
  int valor;                /* Valor de esta etiqueta */
  char nombre[1];           /* Nombre de la etiqueta */
};

struct indefinido {         /* Estructura para almacenar indefinidos */
  struct indefinido *sig;   /* Siguiente indefinido */
  int tipo;                 /* Tipo de indefinido */
                            /* 0 = .word */
                            /* 1 = Salto */
                            /* 2 = CONST */
                            /* 3 = CONSTH o CONSTHZ */
  int linea;                /* En que línea del codigo fuente */
  int codigo;               /* Codigo de la instrucción */
  int posicion;             /* Posición para la corrección */
  char expresion[1];        /* Expresión por evaluar */
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

struct {
  char *nemonico;
  int codigo;
  int tipo;
  int procesador;
} instrucciones[TAM_INFO];  /* Instrucciones del ensamblador */

struct etiqueta *tabla[TAM_TABLA];  /* Tabla de dispersión para etiquetas */
struct indefinido *lista_indef;     /* Lista de indefinidos */
struct indefinido *ultimo_indef;    /* Indefinido más reciente */
struct reubicar *lista_reubica;     /* Lista de reubicaciones */
struct reubicar *ultimo_bloque;     /* Ultimo bloque de reubicaciones */

int total_instrucciones;    /* Total de instrucciones definidas */
int codigo;                 /* Codigo de la instrucción actual */
int pos_actual;             /* Posición en bytes */

char archivo_entrada[TAM_LINEA];    /* Archivo de entrada */
char archivo_salida[TAM_LINEA];     /* Archivo de salida */
char linea_entrada[TAM_LINEA];      /* Una línea de la entrada */
int pos_linea;                      /* Posición en la línea */
char componente[TAM_LINEA];         /* Un componente de la entrada */
char etiqueta[TAM_LINEA];           /* Un etiqueta en expresión */
int total_ensamblado;       /* Total de líneas ensambladas */
int total_reubicacion;      /* Total de reubicaciones */
int tipo_indef;             /* Tipo de indefinido */
int pos_indef;              /* Posición de indefinido */
char *pos_expr;             /* Posición en expresión */

FILE *entrada;              /* Archivo de entrada */
FILE *salida;               /* Archivo de salida */

/*
** Nuestro primer problema es que no sabemos en donde va a caer
** el codigo destino, así que tenemos que generar información para
** reubicación (básicamente las variables globales y algunos apuntadores
** a funciones).
*/
main()
{
  puts("\n");
  color(15);
  puts("Ensamblador G11  (c) Copyright Oscar Toledo G.1998\n");
  while (1) {
    puts("\n");
    color(10);
    puts("¿ Archivo de entrada ? ");
    gets(archivo_entrada);
    if (*archivo_entrada == 0) {
      color(14);
      puts("\nEnsamblador cancelado.\n");
      color(7);
      exit(1);
    }
    entrada = fopen(archivo_entrada, "r");
    if (entrada == NULL) {
      color(14);
      puts("\nNo se puede abrir el archivo de entrada\n");
      color(7);
      continue;
    }
    break;
  }
  while (1) {
    puts("\n");
    puts("¿ Archivo de salida ? ");
    gets(archivo_salida);
    if (*archivo_salida == 0) {
      fclose(entrada);
      color(14);
      puts("\nEnsamblador cancelado.\n");
      color(7);
      exit(1);
    }
    salida = fopen(archivo_salida, "wb");
    if (salida == NULL) {
      color(14);
      puts("\nNo se puede abrir el archivo de salida\n");
      color(7);
      continue;
    }
    break;
  }
  puts("\n");
  color(14);
  prepara_info();
  ensambla();
  genera_reubicacion();
  actualiza_indefinidos();
  fclose(salida);
  libera_etiquetas();
  color(11);
  puts("\rFin de ensamblaje.         \n");
  color(7);
}

/*
** Selecciona un color
*/
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

/*
** Tipos predefinidos:
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
prepara_info()
{
  total_instrucciones = 0;
  almacena_info(".align",            0, 64, 0);
  almacena_info(".byte",             0, 65, 0);
  almacena_info(".global",           0, 66, 0);
  almacena_info(".space",            0, 67, 0);
  almacena_info(".word",             0, 68, 0);
  almacena_info("add",      0x14000000,  1, 0);
  almacena_info("addc",     0x1c000000,  1, 0);
  almacena_info("addcs",    0x18000000,  1, 0);
  almacena_info("addcu",    0x1a000000,  1, 0);
  almacena_info("adds",     0x10000000,  1, 0);
  almacena_info("addu",     0x12000000,  1, 0);
  almacena_info("and",      0x90000000,  1, 0);
  almacena_info("andn",     0x9c000000,  1, 0);
  almacena_info("aseq",     0x70000000,  2, 0);
  almacena_info("asge",     0x5c000000,  2, 0);
  almacena_info("asgeu",    0x5e000000,  2, 0);
  almacena_info("asgt",     0x58000000,  2, 0);
  almacena_info("asgtu",    0x5a000000,  2, 0);
  almacena_info("asle",     0x54000000,  2, 0);
  almacena_info("asleu",    0x56000000,  2, 0);
  almacena_info("aslt",     0x50000000,  2, 0);
  almacena_info("asltu",    0x52000000,  2, 0);
  almacena_info("asneq",    0x72000000,  2, 0);
  almacena_info("call",     0xa8000000,  3, 0);
  almacena_info("calli",    0xc8000000,  4, 0);
  almacena_info("class",    0xe6000000,  5, 0);
  almacena_info("clz",      0x08000000,  6, 0);
  almacena_info("const",    0x03000000, 19, 0);
  almacena_info("consth",   0x02000000, 20, 0);
  almacena_info("consthz",  0x05000000, 20, 5);
  almacena_info("convert",  0xe4000000,  7, 0);
  almacena_info("cpbyte",   0x2e000000,  1, 0);
  almacena_info("cpeq",     0x60000000,  1, 0);
  almacena_info("cpge",     0x4c000000,  1, 0);
  almacena_info("cpgeu",    0x4e000000,  1, 0);
  almacena_info("cpgt",     0x48000000,  1, 0);
  almacena_info("cpgtu",    0x4a000000,  1, 0);
  almacena_info("cple",     0x44000000,  1, 0);
  almacena_info("cpleu",    0x46000000,  1, 0);
  almacena_info("cplt",     0x40000000,  1, 0);
  almacena_info("cpltu",    0x42000000,  1, 0);
  almacena_info("cpneq",    0x62000000,  1, 0);
  almacena_info("dadd",     0xf1000000,  8, 0);
  almacena_info("ddiv",     0xf7000000,  8, 0);
  almacena_info("deq",      0xeb000000,  8, 0);
  almacena_info("dge",      0xef000000,  8, 0);
  almacena_info("dgt",      0xed000000,  8, 0);
  almacena_info("div",      0x6a000000,  1, 0);
  almacena_info("div0",     0x68000000,  6, 0);
  almacena_info("divide",   0xe1000000,  8, 0);
  almacena_info("dividu",   0xe3000000,  8, 0);
  almacena_info("divl",     0x6c000000,  1, 0);
  almacena_info("divrem",   0x6e000000,  1, 0);
  almacena_info("dmac",     0xd9000000,  9, 5);
  almacena_info("dmsm",     0xdb000000,  8, 5);
  almacena_info("dmul",     0xf5000000,  8, 0);
  almacena_info("dsub",     0xf3000000,  8, 0);
  almacena_info("emulate",  0xd7000000,  2, 0);
  almacena_info("exbyte",   0x0a000000,  1, 0);
  almacena_info("exhw",     0x7c000000,  1, 0);
  almacena_info("exhws",    0x7e000000, 10, 0);
  almacena_info("extract",  0x7a000000,  1, 0);
  almacena_info("fadd",     0xf0000000,  8, 0);
  almacena_info("fdiv",     0xf6000000,  8, 0);
  almacena_info("fdmul",    0xf9000000,  8, 0);
  almacena_info("feq",      0xea000000,  8, 0);
  almacena_info("fge",      0xee000000,  8, 0);
  almacena_info("fgt",      0xec000000,  8, 0);
  almacena_info("fmac",     0xd8000000,  9, 5);
  almacena_info("fmsm",     0xda000000,  8, 5);
  almacena_info("fmul",     0xf4000000,  8, 0);
  almacena_info("fsub",     0xf2000000,  8, 0);
  almacena_info("halt",     0x89000000,  0, 0);
  almacena_info("inbyte",   0x0c000000,  1, 0);
  almacena_info("inhw",     0x78000000,  1, 0);
  almacena_info("inv",      0x9f000000,  0, 0);
  almacena_info("iret",     0x89000000,  0, 0);
  almacena_info("iretinv",  0x8c000000,  0, 0);
  almacena_info("jmp",      0xa0000000, 11, 0);
  almacena_info("jmpf",     0xa4000000,  3, 0);
  almacena_info("jmpfdec",  0xb4000000,  3, 0);
  almacena_info("jmpfi",    0xc4000000,  4, 0);
  almacena_info("jmpi",     0xc0000000, 12, 0);
  almacena_info("jmpt",     0xac000000,  3, 0);
  almacena_info("jmpti",    0xcc000000,  4, 0);
  almacena_info("load",     0x16000000, 13, 0);
  almacena_info("loadl",    0x06000000, 13, 0);
  almacena_info("loadm",    0x36000000, 13, 0);
  almacena_info("loadset",  0x26000000, 13, 0);
  almacena_info("mfacc",    0xe9000100, 14, 5);
  almacena_info("mfsr",     0xc6000000, 16, 0);
  almacena_info("mftlb",    0xb6000000, 10, 0);
  almacena_info("mtacc",    0xe8010000, 15, 5);
  almacena_info("mtsr",     0xce000000, 17, 0);
  almacena_info("mtsrim",   0x04000000, 18, 0);
  almacena_info("mttlb",    0xbe000000,  4, 0);
  almacena_info("mul",      0x64000000,  1, 0);
  almacena_info("mull",     0x66000000,  1, 0);
  almacena_info("multiplu", 0xe2000000,  8, 0);
  almacena_info("multiply", 0xe0000000,  8, 0);
  almacena_info("multm",    0xde000000,  8, 0);
  almacena_info("multmu",   0xdf000000,  8, 0);
  almacena_info("mulu",     0x74000000,  1, 0);
  almacena_info("nand",     0x9a000000,  1, 0);
  almacena_info("nop",      0x70400101,  0, 0);
  almacena_info("nor",      0x98000000,  1, 0);
  almacena_info("or",       0x92000000,  1, 0);
  almacena_info("orn",      0xaa000000,  1, 5);
  almacena_info("setip",    0x9e000000,  8, 0);
  almacena_info("sll",      0x80000000,  1, 0);
  almacena_info("sqrt",     0xe5000000,  5, 0);
  almacena_info("sra",      0x86000000,  1, 0);
  almacena_info("srl",      0x82000000,  1, 0);
  almacena_info("store",    0x1e000000, 13, 0);
  almacena_info("storel",   0x0e000000, 13, 0);
  almacena_info("storem",   0x3e000000, 13, 0);
  almacena_info("sub",      0x24000000,  1, 0);
  almacena_info("subc",     0x2c000000,  1, 0);
  almacena_info("subcs",    0x28000000,  1, 0);
  almacena_info("subcu",    0x2a000000,  1, 0);
  almacena_info("subr",     0x34000000,  1, 0);
  almacena_info("subrc",    0x3c000000,  1, 0);
  almacena_info("subrcs",   0x38000000,  1, 0);
  almacena_info("subrcu",   0x3a000000,  1, 0);
  almacena_info("subrs",    0x30000000,  1, 0);
  almacena_info("subru",    0x32000000,  1, 0);
  almacena_info("subs",     0x20000000,  1, 0);
  almacena_info("subu",     0x22000000,  1, 0);
  almacena_info("xnor",     0x96000000,  1, 0);
  almacena_info("xor",      0x94000000,  1, 0);
}
                                         
/*
** Almacena información acerca de una instrucción
*/
almacena_info(nemonico, codigo, tipo, procesador)
  char *nemonico;
  int codigo, tipo, procesador;
{
  instrucciones[total_instrucciones].nemonico = nemonico;
  instrucciones[total_instrucciones].codigo = codigo;
  instrucciones[total_instrucciones].tipo = tipo;
  instrucciones[total_instrucciones].procesador = procesador;
  total_instrucciones++;
}

/*
** Separa un componente de la entrada.
*/
char *separa_componente()
{
  char *ap1 = linea_entrada + pos_linea;
  char *ap2 = componente;

  while (*ap1 == ' ' || *ap1 == '\t') ++ap1;
  while (*ap1 && *ap1 != ' ' && *ap1 != '\t' && *ap1 != ',')
    *ap2++ = *ap1++;
  *ap2 = 0;
  while (*ap1 == ' ' || *ap1 == '\t') ++ap1;
  pos_linea = ap1 - linea_entrada;
  return ap2 - 1;
}

/*
** Busca una etiqueta
*/
struct etiqueta *busca_etiqueta(etiqueta)
  char *etiqueta;
{
  int cubeta = calcula_dispersion(etiqueta);
  struct etiqueta *lista = tabla[cubeta];
  struct etiqueta *anterior;

  while (lista != NULL) {
    if (strcmp(etiqueta, lista->nombre) == 0)
      return lista;
    anterior = lista;
    lista = lista->sig;
  }
  return NULL;
}

/*
** Muestra total de líneas ensambladas.
*/
muestra_lineas()
{
  putchar('\r');
  numero(total_ensamblado);
  puts(" líneas ensambladas.");
  fflush(stdout);
}

/*
** Define una etiqueta
*/
define_etiqueta()
{
  int cubeta;
  struct etiqueta *nueva;

  if (busca_etiqueta(componente) != NULL) {
    error("Etiqueta redefinida");
    return;
  }
  nueva = malloc(sizeof(struct etiqueta) + strlen(componente));
  if (nueva == NULL) {
    error("No hay memoria para etiqueta");
    return;
  }
  cubeta = calcula_dispersion(componente);
  nueva->sig = tabla[cubeta];
  nueva->tipo = 0;
  nueva->valor = pos_actual;
  strcpy(nueva->nombre, componente);
  tabla[cubeta] = nueva;
}

/*
** Cálcula el tamaño de una cadena de texto
*/
int strlen(a)
  char *a;
{
  char *b = a;

  while (*b) b++;
  return b - a;
}

/*
** Copia una cadena de texto
*/
strcpy(b, a)
  char *b, *a;
{
  while (*b++ = *a++);
}

/*
** Agrega una cadena de texto
*/
strcat(b, a)
  char *b, *a;
{
  while (*b) b++;
  strcpy(b, a);
}

/*
** Compara dos cadenas
*/
strcmp(a, b)
  char *a, *b;
{
  while (1) {
    if (*a == *b) {
      if (*a++ == 0)
        return 0;
      b++;
      continue;
    }
    if (*a < *b)
      return -1;
    else
      return 1;
  }
}

/*
** Compara dos cadenas
*/
strcmp2(a, b)
  char *a, *b;
{
  while (1) {
    if (*a == tolower(*b)) {
      if (*a++ == 0)
        return 0;
      b++;
      continue;
    }
    if (*a < tolower(*b))
      return -1;
    else
      return 1;
  }
}

/*
** Traslada una letra a minúsculas
*/
int tolower(letra)
  int letra;
{
  if (letra < 'A' || letra > 'Z')
    return letra;
  else
    return letra + 32;
}

/*
** Checa si el caracter es un número.
*/
int isdigit(car)
  int car;
{
  return (car >= '0') && (car <= '9');
}

/*
** Busca una instrucción o directiva.
*/
busca_instruccion()
{
  int min, max, actual, resultado;

  min = 0;
  max = total_instrucciones - 1;
  while (min <= max) {
    actual = (min + max) >> 1;
    resultado = strcmp2(instrucciones[actual].nemonico, componente);
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
      codigo = instrucciones[actual].codigo;
      procesa_instruccion(instrucciones[actual].tipo, &codigo);
      checa_nada();
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

  if (tipo == 64) {             /* .align */
    while (pos_actual & 3)
      escribe_byte(0);
    return;
  }
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
  if (tipo == 66) {             /* .global */
  }
  if (tipo == 67) {             /* .space */
    a = procesa_expr(0, -1);
    while (a--)
      escribe_byte(0);
    return;
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
}

/*
** Procesa una instrucción
*/
procesa_instruccion(tipo, codigo)
  int tipo, codigo;
{
  switch (tipo) {
    case 0 : return;
    case 1 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             return;
    case 2 : procesa_vn(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             return;
    case 3 : procesa_ra(codigo);
             checa_coma();
             procesa_dir(codigo);
             return;
    case 4 : procesa_ra(codigo);
             checa_coma();
             procesa_rb(codigo);
             return;
    case 5 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_fs(codigo);
             return;
    case 6 : procesa_rc(codigo);
             checa_coma();
             procesa_rbi(codigo);
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
             return;
    case 8 : procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rb(codigo);
             return;
    case 9 : procesa_funcacn(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rb(codigo);
             return;
    case 10: procesa_rc(codigo);
             checa_coma();
             procesa_ra(codigo);
             return;
    case 11: procesa_dir(codigo);
             return;
    case 12: procesa_rb(codigo);
             return;
    case 13: procesa_ce(codigo);
             checa_coma();
             procesa_cntl(codigo);
             checa_coma();
             procesa_ra(codigo);
             checa_coma();
             procesa_rbi(codigo);
             return;
    case 14: procesa_rc(codigo);
             checa_coma();
             procesa_fmt(codigo);
             checa_coma();
             procesa_acn(codigo);
             return;
    case 15: procesa_ra(codigo);
             checa_coma();
             procesa_fmt(codigo);
             checa_coma();
             procesa_acn(codigo);
             return;
    case 16: procesa_rc(codigo);
             checa_coma();
             procesa_sa(codigo);
             return;
    case 17: procesa_sa(codigo);
             checa_coma();
             procesa_rb(codigo);
             return;
    case 18: procesa_sa(codigo);
             checa_coma();
             procesa_const(codigo, 0);
             return;
    case 19: procesa_ra(codigo);
             checa_coma();
             procesa_const(codigo, 1);
             return;
    case 20: procesa_ra(codigo);
             checa_coma();
             procesa_const(codigo, 2);
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
    *codigo |= 0x01000000 | (a & 255);
  }
}

procesa_vn(codigo)
  int *codigo;
{
  int a;

  a = procesa_expr(0, -1);
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

  separa_componente();
  if (strcmp2("vab", componente) == 0)
    reg = 0;
  else if (strcmp2("ops", componente) == 0)
    reg = 1;
  else if (strcmp2("cps", componente) == 0)
    reg = 2;
  else if (strcmp2("cfg", componente) == 0)
    reg = 3;
  else if (strcmp2("cha", componente) == 0)
    reg = 4;
  else if (strcmp2("chd", componente) == 0)
    reg = 5;
  else if (strcmp2("chc", componente) == 0)
    reg = 6;
  else if (strcmp2("rbp", componente) == 0)
    reg = 7;
  else if (strcmp2("tmc", componente) == 0)
    reg = 8;
  else if (strcmp2("tmr", componente) == 0)
    reg = 9;
  else if (strcmp2("pc0", componente) == 0)
    reg = 10;
  else if (strcmp2("pc1", componente) == 0)
    reg = 11;
  else if (strcmp2("pc2", componente) == 0)
    reg = 12;
  else if (strcmp2("mmu", componente) == 0)
    reg = 13;
  else if (strcmp2("lru", componente) == 0)
    reg = 14;
  else if (strcmp2("rsn", componente) == 0)
    reg = 15;
  else if (strcmp2("rma0", componente) == 0)
    reg = 16;
  else if (strcmp2("rmc0", componente) == 0)
    reg = 17;
  else if (strcmp2("rma1", componente) == 0)
    reg = 18;
  else if (strcmp2("rmc1", componente) == 0)
    reg = 19;
  else if (strcmp2("spc0", componente) == 0)
    reg = 20;
  else if (strcmp2("spc1", componente) == 0)
    reg = 21;
  else if (strcmp2("spc2", componente) == 0)
    reg = 22;
  else if (strcmp2("iba0", componente) == 0)
    reg = 23;
  else if (strcmp2("ibc0", componente) == 0)
    reg = 24;
  else if (strcmp2("iba1", componente) == 0)
    reg = 25;
  else if (strcmp2("ibc1", componente) == 0)
    reg = 26;
  else if (strcmp2("ipc", componente) == 0)
    reg = 128;
  else if (strcmp2("ipa", componente) == 0)
    reg = 129;
  else if (strcmp2("ipb", componente) == 0)
    reg = 130;
  else if (strcmp2("q", componente) == 0)
    reg = 131;
  else if (strcmp2("alu", componente) == 0)
    reg = 132;
  else if (strcmp2("bp", componente) == 0)
    reg = 133;
  else if (strcmp2("fc", componente) == 0)
    reg = 134;
  else if (strcmp2("cr", componente) == 0)
    reg = 135;
  else if (strcmp2("fpe", componente) == 0)
    reg = 160;
  else if (strcmp2("inte", componente) == 0)
    reg = 161;
  else if (strcmp2("fps", componente) == 0)
    reg = 162;
  else if (strcmp2("exop", componente) == 0)
    reg = 164;
  else {
    error("Registro especial desconocido");
  }
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
  if (b != ultimo_indef)
    return;
  a = ((a - pos_actual) >> 2) & 0xffff;
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
  salta_espacios();
  if (*pos_expr)
    error("Caracteres extras en expresión");
  if (definido)
    return valor;
  else {
    nuevo = malloc(sizeof(struct indefinido) + strlen(componente));
    if (nuevo == NULL)
      error("No hay espacio para etiqueta indefinida");
    else {
      nuevo->sig = NULL;
      nuevo->codigo = codigo;
      nuevo->tipo = tipo_indef;
      nuevo->posicion = pos_indef;
      nuevo->linea = total_ensamblado;
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
  salta_espacios();
  if (*pos_expr >= '0' && *pos_expr <= '9')
    valor = procesa_num();
  else if ((tolower(*pos_expr) >= 'a' && tolower(*pos_expr) <= 'z')
        || *pos_expr == '_') {
    ap = etiqueta;
    while ((tolower(*pos_expr) >= 'a' && tolower(*pos_expr) <= 'z')
        || (*pos_expr >= '0' && *pos_expr <= '9')
        || (*pos_expr == '_'))
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
    }
    if (pos_indef != -1 && (tipo_indef == 0 || tipo_indef == 2)) {
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

procesa_num()
{
  int valor = 0;

  while (*pos_expr >= '0' && *pos_expr <= '9')
    valor = (valor * 10) + (*pos_expr++ - '0');
  return valor;
}

hay(operador)
  char *operador;
{
  char *ap1 = pos_expr;

  salta_espacios();
  while (*operador)
    if (*ap1++ != *operador++)
      return 0;
  pos_expr = ap1;
  return 1;
}

salta_espacios()
{
  while (*pos_expr == ' ')
    pos_expr++;
}

/*
** Escribe una palabra
*/
escribe_palabra(valor)
  int valor;
{
  escribe_byte(valor >> 24);
  escribe_byte(valor >> 16);
  escribe_byte(valor >> 8);
  escribe_byte(valor);
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
  int k, en_comilla, ignorar;

  while (1) {
    en_comilla = 0;
    ignorar = 0;
    pos_linea = 0;
    while ((k = fgetc(entrada)) > 0) {
      if (k == '"')
        en_comilla ^= 1;
      if (k == '\n' || pos_linea >= (TAM_LINEA - 1))
        break;
      if (!en_comilla && k == ';')
        ignorar = 1;
      if (!ignorar)
        linea_entrada[pos_linea++] = k;
    }
    total_ensamblado++;             /* Se ha leido una línea más */
    if (en_comilla)
      error("Faltan comillas");
    linea_entrada[pos_linea] = 0;   /* Agrega un caracter nulo */
    if (k < 0) {
      fclose(entrada);
      entrada = NULL;
    }
    if (pos_linea || entrada == NULL) {
      pos_linea = 0;
      return;
    }
  }
}

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
    total_ensamblado = lista->linea;
    val = procesa_expr2(0, -2);
    fseek(salida, lista->posicion, SEEK_SET);
    switch (lista->tipo) {
      case 0:  escribe_palabra(val);
               break;
      case 1:  val = (val - lista->posicion) >> 2;
               if (val < -32768 || val > 32767)
                 error("Salto demasiado largo");
      case 2:  codigo = lista->codigo;
               codigo |= (val & 255);
               codigo |= ((val >> 8) & 255) << 16;
               escribe_palabra(codigo);
               break;
      case 3:  val >>= 16;
               codigo = lista->codigo;
               codigo |= (val & 255);
               codigo |= ((val >> 8) & 255) << 16;
               escribe_palabra(codigo);
               break;
      default: error("Error interno del ensamblador");
               break;
    }
    temp = lista->sig;
    free(lista);
    lista = temp;
  }
}

libera_etiquetas()
{
  int a;
  struct etiqueta *lista, *temp;

  for (a = 0; a < TAM_TABLA; a++) {
    lista = tabla[a];
    while (lista != NULL) {
      temp = lista->sig;
      free(lista);
      lista = temp;
    }
  }
}

error(mensaje)
  char *mensaje;
{
  puts(mensaje);
  puts(" en la linea ");
  numero(total_ensamblado);
  puts("\n");
}

/*
** Saca un número decimal en la salida.
*/
numero(num)
  int num;
{
  if (num < 0) {
    putchar('-');
    if (num < -9)
      numero(-(num / 10));
    putchar(-(num % 10) + '0');
  } else {
    if (num > 9)
      numero(num / 10);
    putchar((num % 10) + '0');
  }
}

/*
** Efectua el ensamblaje.
*/
ensambla()
{
  char *ap;

  pos_actual = 0;
  total_ensamblado = 0;

  escribe_palabra(0xA0000010); /* JMP */
  escribe_palabra(0x70406060); /* NOP especial */
  escribe_palabra(0x00000000); /* Aquí se pondra el tamaño del codigo */
  escribe_palabra(0x00000000); /* Tamaño para datos extras */
  escribe_palabra(0x00002000); /* 8 KB. de pila local */
  escribe_palabra(0x00002000); /* 8 KB. de pila global */
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

  while (1) {
    lee_linea();
    if (entrada == NULL)
      break;
    ap = separa_componente();
    if (*ap == ':') {
      *ap = 0;
      define_etiqueta();
    } else
      busca_instruccion();
    if ((total_ensamblado & 63) == 0)
      muestra_lineas();
  }
  muestra_lineas();
  putchar('\n');
}
