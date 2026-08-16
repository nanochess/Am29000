/*
** Procesador de lenguaje HTML
**
** por Oscar Toledo Gutiérrez
**
** © Copyright Oscar Toledo G.1999
**
** Creación: 03-abr-1999.
** Revisión: 09-abr-1999. Primera versión operativa.
*/

/*
** Errores que puede retornar la torre TCP/IP
*/
#define EWOULDBLOCK       (-35)
#define ECONNABORTED      (-53)
#define ETIMEDOUT         (-60)

/*
** Contexto de trabajo de cada ventana HTML
*/
struct contexto {
  /*
  ** Datos básicos
  */
  void *ventana;                  /* La ventana que le corresponde */
  void *barra_ver;                /* Barra de desplazamiento vertical */
  void *barra_hor;                /* Barra de desplazamiento horizontal */
  wchar_t *titulo;                /* Título de esta ventana (para <frame>) */
  wchar_t *base;                  /* Dirección base para hipertexto */
  struct elemento_generico
    *lista_visual;                /* Lista de elementos ya formateados */
  struct elemento_generico
    *ultimo_visual;               /* Último elemento (para agregar) */
  int lista_valida;               /* Indica si la lista de anclas es válida */
  struct ancla *lista_anclas;     /* Anclas en el documento */

  /*
  ** Variables de análisis de la entrada
  */
  int modo_actual;                /* Modo actual */
  char *buffer;                   /* Buffer de entrada */
  char *buffer_marcador;          /* Buffer para marcadores */
  char *ap_marcador;              /* Ap. en el buffer de marcador */
  char *buffer_caracter;          /* Buffer para caracteres */
  char *ap_caracter;              /* Ap. en el buffer de caracter */
  wchar_t *buffer_formateado;     /* Buffer con el texto formateado */
  wchar_t *ap_formateado;         /* Ap. en el buffer de texto formateado */
  wchar_t *inicio_palabra;        /* Inicio de una palabra */
  int estado_actual;              /* Estado actual */
  int no_formatear;               /* No formatea, solo acumula */
  int tipo_marcador;              /* 0= Inicio. 1= Final */
  int hay_espacio;                /* Indica si hay un espacio por agregar */

  /*
  ** Apariencia del fondo de la ventana
  */
  int color_fondo;                /* Color de fondo de la ventana */
  int color_texto;                /* Color del texto */
  int color_enlaces;              /* Color de los enlaces */
  int color_enlaces_visitados;    /* Color de los enlaces visitados */
  int color_enlace_click;         /* Color de un enlace clickeado */
  int tam_x_imagen;               /* Tamaño X de la imagen de fondo */
  int tam_y_imagen;               /* Tamaño Y de la imagen de fondo */
  int tipo_imagen;                /* Tipo de la imagen del fondo */
  void *imagen_fondo;             /* Imagen de fondo */

  /*
  ** Estado del formateador
  */
  int estado_formateo;            /* Estado del formateo */
  struct elemento_generico
    *primer_elemento;             /* Primer elemento en la línea actual */
  int inicio_x;                   /* Inicio X actual de lo acumulado */
  int x_actual;                   /* Posición X actual */
  int y_actual;                   /* Posición Y actual */
  int x_maximo;                   /* Tamaño X máximo alcanzado */
  int y_maximo;                   /* Tamaño Y máximo de la línea actual */
  int y_inferior;                 /* Tamaño Y inferior máximo */
  int tam_base;                   /* Tamaño base del texto */
  int tam_x_imagen_izq;           /* Tamaño X de la imagen a la izquierda */
  int y_libre_imagen_izq;         /* Primer Y libre si hay imagen a la izq. */
  int tam_y_imagen_der;           /* Tamaño Y de la imagen a la derecha */
  int y_libre_imagen_der;         /* Primer Y libre si hay imagen a la der. */
  struct contexto_html
    *contexto_html;               /* Estado actual del formateador */
};

/*
** Anclas <a name=x>
*/
#define REDONDEO_CHAR  4

struct ancla {
  struct ancla *siguiente;        /* Siguiente ancla */
  int posicion_y;                 /* Posición Y actual */
  char nombre[REDONDEO_CHAR];     /* Nombre del ancla */
};

/*
** Banderas de tipo de letra
*/
#define T_MAQUINA     1
#define T_NEGRITA     2
#define T_ITALICA     4
#define T_MASCARA     7
#define T_SUBRAYADO   8
#define T_TACHADO    16
#define T_HIPER      32

/*
** Alineaciones posibles
*/
#define A_IZQUIERDA   0
#define A_CENTRO      1
#define A_DERECHA     2

/*
** Tipos de lista posibles
*/
#define L_NINGUNA     0
#define L_NUMEROS     1
#define L_LETRAS_MIN  2
#define L_LETRAS_MAY  3
#define L_ROMANOS_MIN 4
#define L_ROMANOS_MAY 5
#define L_BALA        6
#define L_CUADRADO    7
#define L_CIRCULO     8

/*
** Sangrado por nivel de lista
*/
#define PIXELES_LISTA       16 /* ul/ol/dir/menu */
#define PIXELES_DEFINICION  32 /* dl/dt/dd */
#define PIXELES_BLOQUE      32 /* blockquote */

/*
** Contexto de formateo HTML
*/
struct contexto_html {
  struct contexto_html *anterior; /* Apuntador al contexto anterior */
  wchar_t *hiperreferencia;       /* Hiperreferencia <a> actual */
  int en_p;                       /* Indica si esta en <p> */
  int tipo;                       /* Tipo de letra */
  int tam;                        /* Tamaño (1-7) del texto */
  int color;                      /* Color del texto */
  int base_y;                     /* Base Y del texto */
  int inicio_x;                   /* Inicio X actual  */
  int final_x;                    /* Final X actual */
  int alineacion;                 /* Alineación */
  int tipo_lista;                 /* Tipo de lista */
  int nivel_lista;                /* Nivel */
  int extra_lista;                /* Pixeles */
  int cuenta_lista;               /* Cuenta de elementos */
};

/*
** Tipos posibles de elemento
*/
#define E_TEXTO       0
#define E_LINEA       1
#define E_IMAGEN      2
#define E_TABLA       3

struct elemento_generico {
  struct elemento_generico *siguiente;
  char tipo;
};

#define RELLENO_WCHAR 2

struct elemento_texto {
  struct elemento_generico *siguiente;
  char tipo; /* E_TEXTO */
  char tipo_de_letra;
  char tam;
  int x;
  int y;
  int color;
  wchar_t *hiperreferencia;
  wchar_t texto[RELLENO_WCHAR];
};

struct elemento_imagen {
  struct elemento_generico *siguiente;
  char tipo; /* E_IMAGEN */
  char tipo_de_imagen;
  char es_mapeable;  /* Tiene atributo ISMAP */
  int x;
  int y;
  /* !!! Si es GIF animado, descompactar en tiempo real */
  void *imagen;
  int tam_borde;
  int tam_x_imagen;
  int tam_y_imagen;
  int tam_x;
  int tam_y;
  struct mapa *mapa; /* Tiene atributo MAP */
};

struct mapa {
  struct mapa *siguiente;
  wchar_t *hiperreferencia;
  int tipo; /* 0= Circulo, 1= Rectángulo, 2= Polígono */
  int coordenadas[1];
};

struct elemento_linea {
  struct elemento_generico *siguiente;
  char tipo; /* E_LINEA */
  char sombreado;
  unsigned short x;
  int y;
  unsigned short tam_x;
  unsigned short tam_y;
};

struct elemento_tabla {
  struct elemento_generico *siguiente;
  char tipo; /* E_TABLA */
  unsigned short x;
  int y;
  unsigned short tam_x;
  unsigned short tam_borde;
  int tam_y;
};

/*
** Mapa para ciertos caracteres Unicode que están disponibles.
*/
#define TOTAL_UNICODE      23

struct {
  int unicode;
  int pdfdoc;
} mapa_unicode[TOTAL_UNICODE] = {
  8226, 128, /* bull   € */  8224, 129, /* dagger  */
  8225, 130, /* Dagger ‚ */  8230, 131, /* hellip ƒ */
  8212, 132, /* mdash  „ */  8211, 133, /* ndash  … */
   402, 134, /* fnof   † */  8260, 135, /* frasl  ‡ */
  8249, 136, /* lsaquo ˆ */  8250, 137, /* rsaquo ‰ */
  8722, 138, /* minus  Š */  8222, 140, /* bdquo  Œ */
  8220, 141, /* ldquo   */  8221, 142, /* rdquo  Ž */
  8216, 143, /* lsquo   */  8217, 144, /* rsquo   */
  8218, 145, /* sbquo  ‘ */  8482, 146, /* trade  ’ */
   338, 150, /* OElig  – */   352, 151, /* Scaron — */
   376, 152, /* Yuml   ˜ */   339, 156, /* oelig  œ */
   353, 157, /* scaron  */
};

/*
** Nombres de caracteres HTML
** Las mayúsculas y minúsculas importan.
*/
#define TOTAL_CARACTERES  123

struct {
  char *nombre;
  int valor;
} nombres_caracteres[TOTAL_CARACTERES] = {
  "AElig",   198,  "Aacute",  193,  "Acirc",   194,
  "Agrave",  192,  "Aring",   197,  "Atilde",  195,
  "Auml",    196,  "Ccedil",  199,  "Dagger", 8225,
  "ETH",     208,  "Eacute",  201,  "Ecirc",   202,
  "Egrave",  200,  "Euml",    203,  "Iacute",  205,
  "Icirc",   206,  "Igrave",  204,  "Iuml",    207,
  "Ntilde",  209,  "OElig",   338,  "Oacute",  211,
  "Ocirc",   212,  "Ograve",  210,  "Otilde",  213,
  "Ouml",    214,  "Oslash",  216,  "Scaron",  352,
  "THORN",   222,  "Uacute",  218,  "Ucirc",   219,
  "Ugrave",  217,  "Uuml",    220,  "Yacute",  221,
  "Yuml",    376,  "aacute",  225,  "acirc",   226,
  "acute",   180,  "aelig",   230,  "agrave",  224,
  "amp",      38,  "aring",   229,  "atilde",  227,
  "auml",    228,  "bdquo",  8222,  "brvbar",  166,
  "bull",   8226,  "ccedil",  231,  "cedil",   184,
  "cent",    162,  "copy",    169,  "curren",  164,
  "dagger", 8224,  "deg",     176,  "divide",  247,
  "eacute",  233,  "ecirc",   234,  "egrave",  232,
  "eth",     240,  "euml",    235,  "fnof",    402,
  "frac12",  189,  "frac14",  188,  "frac34",  190,
  "frasl",  8260,  "gt",       62,  "hellip", 8230,
  "iacute",  237,  "icirc",   238,  "iexcl",   161,
  "igrave",  236,  "iquest",  191,  "iuml",    239,
  "laquo",   171,  "ldquo",  8220,  "lsaquo", 8249,
  "lsquo",  8216,  "lt",       60,  "macr",    175,
  "mdash",  8212,  "micro",   181,  "middot",  183,
  "minus",  8722,  "nbsp",    160,  "ndash",  8211,
  "not",     172,  "ntilde",  241,  "oacute",  243,
  "ocirc",   244,  "oelig",   339,  "ograve",  242,
  "ordf",    170,  "ordm",    186,  "oslash",  248,
  "otilde",  245,  "ouml",    246,  "para",    182,
  "plusmn",  177,  "pound",   163,  "quot",     34,
  "raquo",   187,  "rdquo",  8221,  "reg",     174,
  "rsaquo", 8250,  "rsquo",  8217,  "sbquo",  8218,
  "scaron",  353,  "sect",    167,  "shy",     173,
  "sup1",    185,  "sup2",    178,  "sup3",    179,
  "szlig",   223,  "thorn",   254,  "times",   215,
  "trade",  8482,  "uacute",  250,  "ucirc",   251,
  "ugrave",  249,  "uml",     168,  "uuml",    252,
  "yacute",  253,  "yen",     165,  "yuml",    255,
};

/*
** Marcadores HTML estándar.
** Ordenados para una busqueda binaria.
*/
#define TOTAL_MARCADORES  71

struct {
  char *marcador;
  int (*funcion)();
} marcadores[TOTAL_MARCADORES] = {
  "!doctype",   html_sin_operacion,
  "a",          html_sin_operacion, /* !!! */
  "address",    html_direccion,
  "applet",     html_sin_operacion, /* !!! */
  "area",       html_sin_operacion, /* !!! MAP */
  "b",          html_tipo_negrita,
  "base",       html_sin_operacion, /* !!! */
  "basefont",   html_tipo_base,
  "big",        html_agranda,
  "blockquote", html_bloque,
  "body",       html_cuerpo_documento,
  "br",         html_sin_operacion, /* !!! */
  "caption",    html_sin_operacion, /* !!! TABLE */
  "center",     html_centro,
  "cite",       html_tipo_italica,
  "code",       html_tipo_maquina,
  "dd",         html_elemento_definicion,
  "dfn",        html_tipo_italica,
  "dir",        html_lista_sin_numerar,
  "div",        html_division,
  "dl",         html_lista_definicion,
  "dt",         html_elemento_termino,
  "em",         html_tipo_italica,
  "font",       html_tipo,
  "form",       html_sin_operacion, /* !!! FORM */
  "frame",      html_sin_operacion, /* !!! FRAME */
  "frameset",   html_sin_operacion, /* !!! FRAME */
  "h1",         html_encabezado,
  "h2",         html_encabezado,
  "h3",         html_encabezado,
  "h4",         html_encabezado,
  "h5",         html_encabezado,
  "h6",         html_encabezado,
  "head",       html_sin_operacion,
  "hr",         html_sin_operacion, /* !!! */
  "i",          html_tipo_italica,
  "img",        html_sin_operacion, /* !!! */
  "input",      html_sin_operacion, /* !!! FORM */
  "isindex",    html_sin_operacion, /* !!! */
  "kbd",        html_tipo_maquina,
  "li",         html_elemento_lista,
  "link",       html_sin_operacion,
  "listing",    html_sin_operacion, /* !!! */
  "map",        html_sin_operacion, /* !!! MAP */
  "menu",       html_lista_sin_numerar,
  "meta",       html_sin_operacion, /* !!! */
  "ol",         html_lista_numerada,
  "option",     html_sin_operacion, /* !!! FORM */
  "p",          html_parrafo,
  "plaintext",  html_sin_operacion, /* !!! */
  "pre",        html_sin_operacion, /* !!! */
  "samp",       html_tipo_maquina,
  "script",     html_sin_operacion, /* !!! */
  "select",     html_sin_operacion, /* !!! FORM */
  "small",      html_reduce,
  "strike",     html_tachado,
  "strong",     html_tipo_negrita,
  "style",      html_sin_operacion, /* !!! */
  "sub",        html_subindice,
  "sup",        html_supraindice,
  "table",      html_sin_operacion, /* !!! TABLE */
  "td",         html_sin_operacion, /* !!! TABLE */
  "textarea",   html_sin_operacion, /* !!! FORM */
  "th",         html_sin_operacion, /* !!! TABLE */
  "title",      html_titulo,
  "tr",         html_sin_operacion, /* !!! TABLE */
  "tt",         html_tipo_maquina,
  "u",          html_subrayado,
  "ul",         html_lista_sin_numerar,
  "var",        html_tipo_maquina,
  "xmp",        html_sin_operacion, /* !!! */
};

/*
** Nombres de colores estándar.
** Las mayúsculas y minúsculas dan igual.
*/
struct {
  char *nombre;
  int color;
} colores_estandar[16] = {
  "black",  0x000000,  "navy",   0x000080,
  "green",  0x008000,  "teal",   0x008080,
  "maroon", 0x800000,  "olive",  0x808000,
  "silver", 0xc0c0c0,  "gray",   0x808080,
  "blue",   0x0000ff,  "lime",   0x00ff00,
  "aqua",   0x00ffff,  "red",    0xff0000,
  "yellow", 0xffff00,  "white",  0xffffff,
};

/*
** Tamaño del buffer de entrada
*/
#define TAM_BUFFER_HTML  1024

/*
** Máquina de estados para el procesamiento de SGML
** Arreglo de funciones que retornan apuntadores a char
*/
char *(*analisis_html[])() = {
  html_func_texto,     /* Estado 0 - Texto de documento */
  html_func_m1,        /* Estado 1 - Inicio de marcador */
  html_func_m2,        /* Estado 2 - Marcador */
  html_func_m3,        /* Estado 3 - Espacios */
  html_func_m4,        /* Estado 4 - Parametro */
  html_func_m5,        /* Estado 5 - Espacios */
  html_func_m6,        /* Estado 6 - Valor */
  html_func_m7,        /* Estado 7 - Valor entre comillas */
  html_func_m8,        /* Estado 8 - Valor entre apóstrofes */
  html_func_m9,        /* Estado 9 - Valor genérico */
  html_func_caracter,  /* Caracter especial en texto */
  html_func_caracter,  /* Caracter especial en estado 7 */
  html_func_caracter,  /* Caracter especial en estado 8 */
  html_func_caracter,  /* Caracter especial en estado 9 */
};

/*
** Inicia el contexto de trabajo HTML
*/
int inicia_html(struct contexto *contexto, int creacion)
{
  struct contexto_html *html;
  void *ventana = contexto;

  contexto->estado_formateo = 0;
  contexto->contexto_html = NULL;
  contexto->ventana = contexto;
  contexto->modo_actual = 0;
  contexto->no_formatear = 0;
  contexto->estado_actual = 0;
  contexto->hay_espacio = 0;
  contexto->primer_elemento = NULL;
  contexto->inicio_x = 0;
  contexto->x_actual = 0;
  contexto->y_actual = 0;
  contexto->x_maximo = 0;
  contexto->y_maximo = 0;
  contexto->y_inferior = 0;
  contexto->titulo = NULL;
  contexto->base = NULL;
  contexto->color_fondo = 0xffffff;
  contexto->color_texto = 0x000000;
  contexto->color_enlaces = 0x0000ff;
  contexto->color_enlaces_visitados = 0xff00ff;
  contexto->color_enlace_click = 0x00ff00;
  contexto->tam_x_imagen = 0;
  contexto->tam_y_imagen = 0;
  contexto->tipo_imagen = 0;
  contexto->imagen_fondo = NULL;
  contexto->tam_base = 2;
  contexto->tam_x_imagen_izq = 0;
  contexto->y_libre_imagen_izq = 0;
  contexto->tam_y_imagen_der = 0;
  contexto->y_libre_imagen_der = 0;
  if (creacion) {
    contexto->contexto_html = NULL;
  } else {
    contexto->buffer = malloc(TAM_BUFFER_HTML * 3 +
      TAM_BUFFER_HTML * sizeof(wchar_t));
    if (contexto->buffer == NULL)
      return 1;
    contexto->buffer_caracter = contexto->buffer + TAM_BUFFER_HTML * 1;
    contexto->buffer_marcador = contexto->buffer + TAM_BUFFER_HTML * 2;
    contexto->buffer_formateado = (wchar_t *)
      (contexto->buffer + TAM_BUFFER_HTML * 3);
    contexto->ap_formateado = contexto->buffer_formateado;
    contexto->inicio_palabra = NULL;
    contexto->contexto_html = malloc(sizeof(struct contexto_html));
    if (contexto->contexto_html == NULL) {
      free(contexto->buffer);
      return 1;
    }
    html = contexto->contexto_html;
    html->anterior = NULL;
    html->hiperreferencia = NULL;
    html->en_p = 0;
    html->tipo = 0;
    html->tam = contexto->tam_base;
    html->color = contexto->color_texto;
    html->base_y = 0;
    html->inicio_x = 8;
    html->final_x = leer_variable(ventana, V_TAMX) - 8;
    html->alineacion = A_IZQUIERDA;
    html->tipo_lista = L_NINGUNA;
    html->extra_lista = 0;
    html->nivel_lista = 0;
    html->cuenta_lista = 0;
  }
  return 0;
}

#ifdef DEPURACION_HTML
int archivo_depuracion;
#endif

/*
** Procesa un documento HTML
*/
int procesa_html(struct contexto *contexto, int archivo, int cache)
{
  char *base, *avance;
  int bytes, escrito;
  void *ventana;

#ifdef DEPURACION_HTML
  archivo_depuracion = open(L"x:/Sistema/Temporal/Depuración HTML", 1);
#endif
  /*
  ** Prepara el formateador HTML
  */
  ventana = contexto;
  elimina_lista_visual(contexto);
  if (inicia_html(contexto, 0)) {
    /* !!! Indicar error */
    return 1;
  }
  contexto->y_actual += 16;  /* Margen estético */
  while (1) {
    while (1) {
      bytes = read(archivo, contexto->buffer, TAM_BUFFER_HTML);
      if (cache >= 0) {
        escrito = write(cache, contexto->buffer, bytes);
        if (escrito != bytes) {
          /* !!! Indicar error */
          termina_html(contexto);
          return 1;
        }
      }
      if (bytes >= 0)
        break;
      if (bytes == EWOULDBLOCK) {
        multitarea();
        continue;
      }
      /* !!! Indicar error */
      termina_html(contexto);
      return 1;
    }
    if (bytes == 0) {  /* No hay más que procesar */
      break;
    }
    base = contexto->buffer;
    while (bytes) {
      avance =
        (*analisis_html[contexto->estado_actual])
          (base, bytes, contexto);
      bytes -= (avance - base);
      base = avance;
    }
  }
  corte_de_linea(contexto);
#ifdef DEPURACION_HTML
  close(archivo_depuracion);
#endif
  termina_html(contexto);
  return 0;
}

/*
** Crea un nuevo contexto HTML
*/
void crea_nuevo_contexto(struct contexto *contexto)
{
  struct contexto_html *html;

  html = malloc(sizeof(struct contexto_html));
  if (html == NULL) {
    /* !!! Avisar que no hay memoria */
    return;
  }
  memcpy(html, contexto->contexto_html, sizeof(struct contexto_html));
  html->anterior = contexto->contexto_html;
  html->en_p = 0;
  contexto->contexto_html = html;
}

/*
** Recupera el contexto HTML anterior
*/
void recupera_contexto(struct contexto *contexto)
{
  struct contexto_html *html = contexto->contexto_html;

  if (html->anterior != NULL)
    libera_contexto_html(contexto);
}


/*
** Libera un contexto HTML
*/
void libera_contexto_html(struct contexto *contexto)
{
  struct contexto_html *html = contexto->contexto_html;

  contexto->contexto_html = html->anterior;
  free(html);
}

/*
** Termina de utilizar el formateador HTML
*/
void termina_html(struct contexto *contexto)
{
  while (contexto->contexto_html != NULL)
    libera_contexto_html(contexto);
  free(contexto->buffer);
}

/*
** Elimina una lista de elementos visuales
*/
void elimina_lista_visual(struct contexto *contexto)
{
  struct elemento_generico *elemento, *temp;
  struct elemento_texto *texto;
  struct elemento_linea *linea;
  struct elemento_imagen *imagen;
  struct elemento_tabla *tabla;

  elemento = contexto->lista_visual;
  while (elemento != NULL) {
    switch (elemento->tipo) {
      case E_TEXTO:
        texto = elemento;
        free(texto->hiperreferencia);
        break;
      case E_LINEA:
        linea = elemento;
        /* !!! */
        break;
      case E_IMAGEN:
        imagen = elemento;
        /* !!! */
        break;
      case E_TABLA:
        tabla = elemento;
        /* !!! */
        break;
    }
    temp = elemento->siguiente;
    free(elemento);
    elemento = temp;
  }
  contexto->lista_visual = NULL;
  contexto->ultimo_visual = NULL;
}

/*
** Procesamiento de la entrada
*/

/*
** Procesamiento de texto
**
** Si hay_espacio es 0, significa que esta acumulando letras
** Si hay_espacio es 1, significa que el último caracter era un espacio.
** Si hay_espacio es 2, significa que debe ignorar todos los
** espacios, porque acaba de procesar un marcador de inicio.
*/
char *html_func_texto(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (*ap == ' ' || *ap == '\r' || *ap == '\n') {
      if (contexto->hay_espacio == 0) { /* Acumula una palabra */
        if (contexto->no_formatear == 0)
          formatea_palabra(contexto);
        contexto->hay_espacio = 1;
      }
    } else if (*ap == '<') { /* Inicio de marcador */
      contexto->estado_actual = 1;
      return ++ap;
    } else {
      if (contexto->inicio_palabra == NULL)
        contexto->inicio_palabra = contexto->ap_formateado;
      if (contexto->hay_espacio == 1) { /* Inserta espacio */
        if (contexto->ap_formateado - contexto->buffer_formateado < TAM_BUFFER_HTML - 2)
          if (contexto->ap_formateado == contexto->buffer_formateado
           || *(contexto->ap_formateado - 1) != ' ')
            *contexto->ap_formateado++ = ' ';
      }
      contexto->hay_espacio = 0;
      if (*ap == '&') {
        contexto->estado_actual = 10;
        contexto->ap_caracter = contexto->buffer_caracter;
        return ++ap;
      } else {
        if (contexto->ap_formateado - contexto->buffer_formateado < TAM_BUFFER_HTML - 1)
          *contexto->ap_formateado++ = *ap;
      }
    }
    ap++;
    bytes--;
  }
  return ap;
}

/*
** Procesamiento de marcador
** 1. Detecta el tipo de marcador
*/
char *html_func_m1(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    contexto->estado_actual = 2;
    contexto->ap_marcador = contexto->buffer_marcador;
    if (*ap == '/') {
      contexto->tipo_marcador = 1;
      return ++ap;
    } else {
      contexto->tipo_marcador = 0;
      return ap;
    }
  }
  return ap;
}

/*
** 2. Absorbe el marcador
*/
char *html_func_m2(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (isspace(*ap) || *ap == '>') { /* Un espacio o final termina */
      *contexto->ap_marcador++ = 0;
      contexto->estado_actual = 3;
      return ap;
    } else {
      if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 2)
        *contexto->ap_marcador++ = tolower(*ap);
    }
    ap++;
    bytes--;
  }
  return ap;
}

/*
** 3. Absorbe espacios inutiles
*/
char *html_func_m3(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (isspace(*ap)) {
      ap++;
      bytes--;
    } else {
      contexto->estado_actual = 4;
      return ap;
    }
  }
  return ap;
}

/*
** 4. Absorbe posibles parametros
*/
char *html_func_m4(char *ap, int bytes, struct contexto *contexto)
{
  char *buffer;
  int min, max, c, v;
  char *parametros[32];
  char *valores[32];
  int total_parametros;

  while (bytes) {
    c = *ap; /* Evade defecto en CC */
    if (c == '>') { /* Final de marcador */
      /*
      ** Termina adecuadamente un nombre de parametro
      */
      *contexto->ap_marcador++ = 0;
      *contexto->ap_marcador++ = 0;
      *contexto->ap_marcador++ = 0;

      /*
      ** Vuelve a procesamiento de texto, instruye para
      ** que elimine espacios después de un marcador inicial
      */
      contexto->estado_actual = 0;
      if (contexto->tipo_marcador == 0) {
        if (contexto->hay_espacio == 1) {
          contexto->inicio_palabra = contexto->ap_formateado;
          if (contexto->ap_formateado - contexto->buffer_formateado < TAM_BUFFER_HTML - 2) {
            if (contexto->ap_formateado == contexto->buffer_formateado
             || *(contexto->ap_formateado - 1) != ' ')
              *contexto->ap_formateado++ = ' ';
          }
        }
        contexto->hay_espacio = 2;
      } else {
        contexto->hay_espacio = 0;
      }

      /*
      ** Busca todos los parametros
      */
      buffer = contexto->buffer_marcador;
      total_parametros = 0;
#ifdef DEPURACION_HTML
      write(archivo_depuracion, "Marcador: ", 10);
      while (*buffer)
        write(archivo_depuracion, buffer++, 1);
      buffer++;
      write(archivo_depuracion, " - Parametros:", 14);
      while (*buffer) {
        write(archivo_depuracion, " ", 1);
        parametros[total_parametros] = buffer; /* Anota el parametro */
        valores[total_parametros] = NULL; /* Aún no sabe si hay valor */
        while (*buffer)
          write(archivo_depuracion, buffer++, 1);
        buffer++;
        if (*buffer) {
          write(archivo_depuracion, "=", 1);
          valores[total_parametros] = buffer; /* Anota el valor */
        }
        while (*buffer)
          write(archivo_depuracion, buffer++, 1);
        buffer++;
        total_parametros++;
      }
      write(archivo_depuracion, "\r\n", 2);
#else
      while (*buffer++) ;
      while (*buffer) {
        parametros[total_parametros] = buffer; /* Anota el parametro */
        valores[total_parametros] = NULL; /* Aún no sabe si hay valor */
        while (*buffer++) ;
        if (*buffer)
          valores[total_parametros] = buffer; /* Anota el valor */
        while (*buffer++) ;
        total_parametros++;
      }
#endif

      /*
      ** Busca el marcador en la tabla, si no lo localiza
      ** entonces lo ignora
      */
      min = 0;
      max = TOTAL_MARCADORES - 1;
      while (min <= max) {
        c = (min + max) / 2;
        v = strcmp(contexto->buffer_marcador, marcadores[c].marcador);
        if (v == 0) {
          (*marcadores[c].funcion)(contexto, total_parametros, parametros, valores);
          break;
        } else if (v == -1) {
          max = c - 1;
        } else {
          min = c + 1;
        }
      }
      return ++ap;
    } else if (c == '=') { /* Valor para parametro */
      *contexto->ap_marcador++ = 0; /* Termina nombre */
      contexto->estado_actual = 6;
      return ++ap;
    } else if (isspace(c)) { /* Espacio */
      *contexto->ap_marcador++ = 0; /* Termina nombre */
      contexto->estado_actual = 5;
      return ap;
    } else { /* Absorbe nombre de parametro */
      if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 5)
        *contexto->ap_marcador++ = tolower(c);
      ap++;
      bytes--;
    }
  }
  return ap;
}

/*
** 5. Absorbe espacios después de parametro y detecta valor
*/
char *html_func_m5(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (isspace(*ap)) {
      ap++;
      bytes--;
    } else if (*ap == '=') {
      contexto->estado_actual = 6;
      return ++ap;
    } else {
      *contexto->ap_marcador++ = 0; /* Sin valor */
      contexto->estado_actual = 4;
      return ap;
    }
  }
  return ap;
}

/*
** 6. Absorbe espacios después de signo =
*/
char *html_func_m6(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (isspace(*ap)) {
      ap++;
      bytes--;
    } else if (*ap == '"') {
      contexto->estado_actual = 7;
      return ++ap;
    } else if (*ap == '\'') {
      contexto->estado_actual = 8;
      return ++ap;
    } else {
      contexto->estado_actual = 9;
      return ap;
    }
  }
  return ap;
}

/*
** 7. Absorbe un valor entrecomillado
*/
char *html_func_m7(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (*ap == '"') {
      contexto->estado_actual = 3;
      *contexto->ap_marcador++ = 0;
      return ++ap;
    } else if (*ap == '&') {
      contexto->estado_actual = 11;
      contexto->ap_caracter = contexto->buffer_caracter;
      return ++ap;
    } else {
      if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 5)
        *contexto->ap_marcador++ = *ap;
    }
    ap++;
    bytes--;
  }
  return ap;
}

/*
** 8. Absorbe un valor entre apóstrofes
*/
char *html_func_m8(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (*ap == '\'') {
      contexto->estado_actual = 3;
      *contexto->ap_marcador++ = 0;
      return ++ap;
    } else if (*ap == '&') {
      contexto->estado_actual = 12;
      contexto->ap_caracter = contexto->buffer_caracter;
      return ++ap;
    } else {
      if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 5)
        *contexto->ap_marcador++ = *ap;
    }
    ap++;
    bytes--;
  }
  return ap;
}

/*
** 9. Absorbe un valor común
*/
char *html_func_m9(char *ap, int bytes, struct contexto *contexto)
{
  while (bytes) {
    if (*ap == '>' || isspace(*ap)) {
      contexto->estado_actual = 3;
      *contexto->ap_marcador++ = 0;
      return ap;
    } else if (*ap == '&') {
      contexto->estado_actual = 13;
      contexto->ap_caracter = contexto->buffer_caracter;
      return ++ap;
    } else {
      if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 5)
        *contexto->ap_marcador++ = *ap;
    }
    ap++;
    bytes--;
  }
  return ap;
}

/*
** Procesamiento de caracteres especiales
*/
char *html_func_caracter(char *ap, int bytes, struct contexto *contexto)
{
  int min, max, c, v;
  int caracter;
  int cuenta;
  char *ap1;

  while (bytes) {
    if (*ap == ';') { /* Procesa el caracter */
      *contexto->ap_caracter = 0;
      ap1 = contexto->buffer_caracter;
      caracter = 0;

      /*
      ** Si empieza con # es un número
      */
      if (*ap1 == '#') {
        ap1++;
        if (*ap1 == 'x' || *ap1 == 'X') { /* Hexadecimal */
          ap1++;
          while (1) {
            c = *ap1;
            if (c >= 'a' && c <= 'z')
              c -= ' ';
            if (c < '0')
              break;
            if (c > 'F')
              break;
            if (c > '9') {
              c -= 7;
              if (c <= '9')
                break;
            }
            c -= '0';
            caracter = (caracter << 4) | c;
            ap1++;
          }
        } else { /* Decimal */
          while (*ap1 >= '0' && *ap1 <= '9')
            caracter = caracter * 10 + (*ap1++ - '0');
        }
      } else {  /* Si no, es un nombre de caracter */
        min = 0;
        max = TOTAL_CARACTERES - 1;
        while (min <= max) {
          c = (min + max) / 2;
          v = strcmp(ap1, nombres_caracteres[c].nombre);
          if (v == 0) {
            caracter = nombres_caracteres[c].valor;
            break;
          } else if (v == -1) {
            max = c - 1;
          } else {
            min = c + 1;
          }
        }
      }

      /*
      ** Si es un caracter Unicode, buscar en el mapa
      */
      if (caracter >= 256) {
        for (cuenta = 0; cuenta < TOTAL_UNICODE; cuenta++) {
          if (caracter == mapa_unicode[cuenta].unicode) {
            caracter = mapa_unicode[cuenta].pdfdoc;
            break;
          }
        }
        if (cuenta == TOTAL_UNICODE)
          caracter = 0;
      }

      /*
      ** No se encontro el caracter, mostrar signo de interrogación
      */
      if (caracter == 0)
        caracter = '?';

      /*
      ** Agrega en el lugar correcto
      */
      if (contexto->estado_actual >= 11) { /* Caracter en marcador */
        if (contexto->ap_marcador - contexto->buffer_marcador < TAM_BUFFER_HTML - 1)
          *contexto->ap_marcador++ = caracter;
        contexto->estado_actual -= 4;
      } else { /* Caracter en texto */
        if (contexto->ap_formateado - contexto->buffer_formateado < TAM_BUFFER_HTML - 2)
          *contexto->ap_formateado++ = caracter;
        contexto->estado_actual = 0;
      }
      return ++ap;
    } else { /* Acumula caracteres */
      if (contexto->ap_caracter - contexto->buffer_caracter < TAM_BUFFER_HTML - 1)
        *contexto->ap_caracter++ = *ap;
      ap++;
      bytes--;
    }
  }
  return ap;
}

/*
** Obtiene el ancho de una letra en pixeles (100 puntos por pulgada)
*/
int ancho_letra(struct contexto *contexto, int letra)
{
  if (letra > 255)
    letra = 32;
  return (anchos[contexto->contexto_html->tipo & T_MASCARA][letra] *
         escalas[contexto->contexto_html->tam] + 65535) / 65536;
}

/*
** Formatea una palabra más
*/
void formatea_palabra(struct contexto *contexto)
{
  int tam;
  int tam_extra;
  wchar_t *ap = contexto->inicio_palabra;
  wchar_t *ap1;

  /*
  ** Checa si hay palabra
  */
  *contexto->ap_formateado = '\0';
  if (*contexto->buffer_formateado == '\0')
    return;
  if (ap == NULL)
    return;

  /*
  ** Cálcula el espacio que puede ocupar un espacio inicial
  */
  if (*ap == ' ') {
    tam_extra = ancho_letra(contexto, *ap);
    ap++;
  } else {
    tam_extra = 0;
  }

  /*
  ** Cálcula el espacio que ocupa la palabra
  */
  tam = 0;
  while (*ap)
    tam += ancho_letra(contexto, *ap++);

  /*
  ** Checa si cabe, dejamos pasar una línea que no cabe de
  ** ninguna forma.
  */
  if (checa_espacio(contexto, tam, tam_extra) || contexto->x_actual == 0) {
    /*
    ** Cabe a la perfección, avanzar x_actual
    */
    contexto->x_actual += tam + tam_extra;
  } else {
    /*
    ** Romper la línea para insertar los nuevos datos
    */
    ap = contexto->ap_formateado = contexto->inicio_palabra;
    if (*ap != ' ') {
      ap1 = ap;
      while (*ap1)
        ap1++;
      do {
        *(ap1 + 1) = *ap1;
      } while (ap1-- != ap);
    }
    vaciar_acumulado(contexto);
    iniciar_linea(contexto);
    ap++;
    ap1 = contexto->buffer_formateado;
    while (*ap)
      *ap1++ = *ap++;
    contexto->ap_formateado = ap1;
    contexto->x_actual += tam;
  }
  contexto->inicio_palabra = NULL;
}

/*
** Checa si cabe un elemento
*/
int checa_espacio(struct contexto *contexto, int tam_real, int tam_extra)
{
  struct contexto_html *html = contexto->contexto_html;
  int x_disponible = html->final_x - html->inicio_x;

  if (contexto->x_actual + tam_real + tam_extra <= x_disponible)
    return 1;
  return 0;
}

/*
** Vacía todo lo que esta acumulado
*/
void vaciar_acumulado(struct contexto *contexto)
{
  struct contexto_html *html = contexto->contexto_html;
  wchar_t *hiperreferencia;
  int extra;
  struct elemento_texto *texto;
  int altura;

  *contexto->ap_formateado = '\0';
  if (*contexto->buffer_formateado == '\0') /* No hay nada */
    return;
  hiperreferencia = NULL;
  if (html->hiperreferencia) {
    hiperreferencia = malloc((wcslen(html->hiperreferencia) + 1)
      * sizeof(wchar_t));
    if (hiperreferencia == NULL) {
      /* !!! Avisar que no hay memoria */
      return;
    }
    wcscpy(hiperreferencia, html->hiperreferencia);
  }
  extra = ((contexto->ap_formateado + 1) - contexto->buffer_formateado) *
    sizeof(wchar_t) - RELLENO_WCHAR;
  texto = malloc(sizeof(struct elemento_texto) + extra);
  if (texto == NULL) {
    /* !!! Avisar no hay memoria */
    return;
  }
  texto->siguiente = NULL;
  texto->tipo = E_TEXTO;
  texto->tipo_de_letra = html->tipo;
  texto->tam = html->tam;
  texto->x = contexto->inicio_x;
  texto->y = html->base_y;
  texto->color = html->color;
  texto->hiperreferencia = hiperreferencia;
  wcscpy(texto->texto, contexto->buffer_formateado);
  contexto->ultimo_visual->siguiente = texto;
  contexto->ultimo_visual = texto;
  if (contexto->lista_visual == NULL)
    contexto->lista_visual = texto;
  if (contexto->primer_elemento == NULL)
    contexto->primer_elemento = texto;
  altura = alturas[html->tam] - html->base_y;
  if (altura > contexto->y_maximo)
    contexto->y_maximo = altura;
  altura = html->base_y + 2;
  if (altura > contexto->y_inferior)
    contexto->y_inferior = altura;
  contexto->inicio_x = contexto->x_actual;
}

/*
** Efectua la alineación de la línea actual, luego empieza
** una nueva línea
*/
void iniciar_linea(struct contexto *contexto)
{
  struct contexto_html *html = contexto->contexto_html;
  struct elemento_generico *elemento;
  struct elemento_texto *texto;
  struct elemento_imagen *imagen;
  int x_extra, y_extra;

  /*
  ** Efectua la alineación de los elementos
  */
  switch (html->alineacion) {
    case A_IZQUIERDA:
      x_extra = html->inicio_x;
      break;
    case A_CENTRO:
      x_extra = html->inicio_x +
        ((html->final_x - html->inicio_x) - contexto->x_actual) / 2;
      break;
    case A_DERECHA:
      x_extra = html->final_x - contexto->x_actual;
      break;
  }
  if (contexto->x_actual + x_extra > contexto->x_maximo)
    contexto->x_maximo = contexto->x_actual + x_extra;
  y_extra = contexto->y_maximo + contexto->y_actual;
  elemento = contexto->primer_elemento;
  while (elemento != NULL) {
    switch (elemento->tipo) {
      case E_TEXTO:  /* Texto */
        texto = elemento;
        texto->x += x_extra;
        texto->y += y_extra;
        break;
      case E_IMAGEN: /* Imagen */
        imagen = elemento;
        imagen->x += x_extra;
        imagen->y += y_extra;
        /* !!! */
        break;
    }
    elemento = elemento->siguiente;
  }
  contexto->y_actual += contexto->y_maximo + contexto->y_inferior;
  contexto->inicio_x = 0;
  contexto->x_actual = 0;
  contexto->y_maximo = 0;
  contexto->primer_elemento = NULL;
}

/*
** Crea un corte de línea (solo si es necesario)
*/
void corte_de_linea(struct contexto *contexto)
{
  formatea_palabra(contexto);
  if (contexto->x_actual) {
    vaciar_acumulado(contexto);
    iniciar_linea(contexto);
  }
  contexto->ap_formateado = contexto->buffer_formateado;
}

/*
** Crea un corte de párrafo (solo si es necesario)
*/
void corte_de_parrafo(struct contexto *contexto)
{
  if (contexto->ap_formateado - 1 == contexto->buffer_formateado
   && *contexto->buffer_formateado == ' ')
    contexto->ap_formateado--;
  formatea_palabra(contexto);
  if (contexto->x_actual) {
    vaciar_acumulado(contexto);
    iniciar_linea(contexto);
    contexto->y_actual += 8;
  }
  contexto->ap_formateado = contexto->buffer_formateado;
}

/*
** Vacia el texto formateado
*/
void vaciar_texto(struct contexto *contexto)
{
  formatea_palabra(contexto);
  vaciar_acumulado(contexto);
  contexto->ap_formateado = contexto->buffer_formateado;
}

/*
** PROCESAMIENTO DE MARCADORES
*/

/*
** Para ignorar marcadores
*/
void html_sin_operacion(struct contexto *contexto)
{
}

/*
** Procesa el marcador <title>
*/
void html_titulo(struct contexto *contexto, int total_parametros,
  char *parametros[], char *valores[])
{
  void *ventana;
  wchar_t *ap;

  if (contexto->tipo_marcador) { /* Final */
    contexto->no_formatear = 0;
    *contexto->ap_formateado = 0;
    ventana = (void *) leer_variable(ventana_interfaz, V_MADRE);
    ventana = (void *) leer_variable(ventana, V_TITULO);
    cambia_texto(ventana, contexto->buffer_formateado);
    mensaje_urgente(ventana, M_PINTAR, 0, -1);
    ap = contexto->buffer_formateado;
    contexto->ap_formateado = ap;
    contexto->inicio_palabra = ap;
  } else {
    if (contexto->hay_espacio == 1)
      contexto->hay_espacio = 0;
    contexto->no_formatear = 1;
    ap = contexto->buffer_formateado;
    wcscpy(ap, NOMBRE_CONFIGURACION);
    while (*ap)
      ap++;
    *ap++ = ' ';
    *ap++ = '-';
    *ap++ = ' ';
    contexto->ap_formateado = ap;
  }
}

/*
** Procesa un indicador de alineación
*/
void html_alineacion(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html = contexto->contexto_html;
  int a;

  a = html_parametro(total, parametros, "align");
  if (a >= 0) {
    minusculas(valores[a]);
    if (strcmp(valores[a], "left") == 0)
      html->alineacion = A_IZQUIERDA;
    else if (strcmp(valores[a], "right") == 0)
      html->alineacion = A_DERECHA;
    else if (strcmp(valores[a], "center") == 0)
      html->alineacion = A_CENTRO;
  }
}

/*
** Busca un parametro
*/
int html_parametro(int total, char *parametros[], char *parametro)
{
  int a;

  for (a = 0; a < total; a++) {
    if (strcmp(parametros[a], parametro) == 0)
      return a;
  }
  return -1;
}

/*
** Convierte una cadena a minúsculas
*/
void minusculas(char *ap)
{
  while (*ap) {
    *ap = tolower(*ap);
    ap++;
  }
}

/*
** Cierra un marcador de párrafo <p>
*/
void html_cerrar_p(struct contexto *contexto)
{
  struct contexto_html *html = contexto->contexto_html;

  if (html->en_p)
    recupera_contexto(contexto);
}

/*
** Procesa un valor de color
*/
void html_color(char *valor, int *color)
{
  int a;
  int c;

  minusculas(valor);
  if (*valor == '#') {
    valor++;
    c = 0;
    while (*valor) {
      a = *valor++ - '0';
      if (a > 9)
        a -= 0x27;
      c = (c << 4) | a;
    }
    *color = c;
  } else {
    for (a = 0; a < 16; a++) {
      if (strcmp(valor, colores_estandar[a].nombre) == 0) {
        *color = colores_estandar[a].color;
        return;
      }
    }
  }
}

/*
** Procesa el marcador <body>
*/
void html_cuerpo_documento(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  int a;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    /* Ignora */
  } else {
    a = html_parametro(total, parametros, "bgcolor");
    if (a >= 0)
      html_color(valores[a], &contexto->color_fondo);
    a = html_parametro(total, parametros, "text");
    if (a >= 0)
      html_color(valores[a], &contexto->color_texto);
    contexto->contexto_html->color = contexto->color_texto;
    a = html_parametro(total, parametros, "link");
    if (a >= 0)
      html_color(valores[a], &contexto->color_enlaces);
    a = html_parametro(total, parametros, "vlink");
    if (a >= 0)
      html_color(valores[a], &contexto->color_enlaces_visitados);
    a = html_parametro(total, parametros, "alink");
    if (a >= 0)
      html_color(valores[a], &contexto->color_enlace_click);
    a = html_parametro(total, parametros, "background");
    if (a >= 0) {
      /* !!! Implementar imagen de fondo */
    }
  }
}

/*
** Procesa los marcadores <h1 - h2 - h3 - h4 - h5 - h6>
*/
void html_encabezado(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo = (html->tipo & ~T_MASCARA) | T_NEGRITA;
    html->tam = 6 - (contexto->buffer_marcador[1] - '1');
    html_alineacion(total, parametros, valores);
  }
}

/*
** Procesa el marcador <address>
*/
void html_direccion(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo = (html->tipo & ~T_MASCARA) | T_ITALICA;
    html->tam = 2;
  }
}

/*
** Procesa el marcador <p>
*/
void html_parrafo(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  if (contexto->tipo_marcador) { /* Final */
    /* Ignora */
  } else {
    corte_de_parrafo(contexto);
    html_cerrar_p(contexto);
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->en_p = 1;
    html_alineacion(total, parametros, valores);
  }
}

/*
** Procesa los marcadores <ul> <menu> <dir>
*/
void html_lista_sin_numerar(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;
  int a;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->inicio_x += PIXELES_LISTA;
    html->nivel_lista++;
    a = html_parametro(total, parametros, "type");
    if (a >= 0) {
      minusculas(valores[a]);
      if (strcmp(valores[a], "disc") == 0)
        html->tipo_lista = L_BALA;
      else if (strcmp(valores[a], "square") == 0)
        html->tipo_lista = L_CUADRADO;
      else if (strcmp(valores[a], "circle") == 0)
        html->tipo_lista = L_CIRCULO;
      else
        html->tipo_lista = L_BALA;
    } else {
      if (html->nivel_lista == 1)
        html->tipo_lista = L_BALA;
      else if (html->nivel_lista == 2)
        html->tipo_lista = L_CUADRADO;
      else
        html->tipo_lista = L_CIRCULO;
    }
    html->cuenta_lista = 1;
  }
}

/*
** Procesa el marcador <ol>
*/
void html_lista_numerada(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;
  int a;
  char *valor;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->inicio_x += PIXELES_LISTA;
    html->nivel_lista++;
    a = html_parametro(total, parametros, "type");
    if (a >= 0) {
      valor = valores[a];
      switch (*valor) {
        default:
        case '1':
          html->tipo_lista = L_NUMEROS;
          break;
        case 'a':
          html->tipo_lista = L_LETRAS_MIN;
          break;
        case 'A':
          html->tipo_lista = L_LETRAS_MAY;
          break;
        case 'i':
          html->tipo_lista = L_ROMANOS_MIN;
          break;
        case 'I':
          html->tipo_lista = L_ROMANOS_MAY;
          break;
      }
    } else {
      html->tipo_lista = L_NUMEROS;
    }
    a = html_parametro(total, parametros, "start");
    if (a >= 0) {
      html->cuenta_lista = atoi(valores[a]);
    } else {
      html->cuenta_lista = 1;
    }
  }
}

/*
** Rutina para generación de números romanos
**
** Basada en una rutina de INPUT Sinclair #17 (1987)
** «Manejando los Números Romanos».
*/
wchar_t *html_romanos(int valor, wchar_t *destino, int suma)
{
  static char *romanos[36] = {
    "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX",
    "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC",
    "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM",
    "M", "MM", "MMM", "", "", "", "", "", "",
  };
  int c[4];
  int a;
  char *ap;

  c[0] = (valor / 1000) % 10;
  c[1] = (valor /  100) % 10;
  c[2] = (valor /   10) % 10;
  c[3] = (valor       ) % 10;

  for (a = 0; a < 4; a++) {
    if (c[a] == 0)
      continue;
    ap = romanos[a * 9 + c[a] - 1];
    while (*ap)
      *destino++ = *ap++ + suma;
  }
  return destino;
}

/*
** Procesa el marcador <li>
*/
void html_elemento_lista(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_linea(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    /* Ignorar */
  } else {
    html = contexto->contexto_html;
    switch (html->tipo_lista) {
      case L_NINGUNA:
        break;
      case L_NUMEROS:
        contexto->ap_formateado = entero_wchar(html->cuenta_lista,
          contexto->ap_formateado);
        *contexto->ap_formateado++ = '.';
        contexto->hay_espacio = 1;
        break;
      case L_LETRAS_MIN:
        *contexto->ap_formateado++ = 96 + html->cuenta_lista;
        *contexto->ap_formateado++ = '.';
        contexto->hay_espacio = 1;
        break;
      case L_LETRAS_MAY:
        *contexto->ap_formateado++ = 64 + html->cuenta_lista;
        *contexto->ap_formateado++ = '.';
        contexto->hay_espacio = 1;
        break;
      case L_ROMANOS_MIN:
        contexto->ap_formateado = html_romanos(html->cuenta_lista,
          contexto->ap_formateado, 0);
        *contexto->ap_formateado++ = '.';
        contexto->hay_espacio = 1;
        break;
      case L_ROMANOS_MAY:
        contexto->ap_formateado = html_romanos(html->cuenta_lista,
          contexto->ap_formateado, 32);
        *contexto->ap_formateado++ = '.';
        contexto->hay_espacio = 1;
        break;
      case L_BALA:
      case L_CUADRADO:
      case L_CIRCULO:
        *contexto->ap_formateado++ = 128; /* € */
        contexto->hay_espacio = 1;
        break;
    }
    html->cuenta_lista++;
  }
}

/*
** Procesa el marcador <dl>
*/
void html_lista_definicion(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    html = contexto->contexto_html;
    if (html->anterior != NULL
     && html->anterior->nivel_lista == html->nivel_lista
     && html->anterior->inicio_x == html->inicio_x + PIXELES_DEFINICION)
      recupera_contexto(contexto);
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->inicio_x += PIXELES_DEFINICION;
    html->nivel_lista++;
  }
}

/*
** Procesa el marcador <dt>
*/
void html_elemento_termino(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_linea(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    /* Ignora */
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->inicio_x -= PIXELES_DEFINICION;
  }
}

/*
** Procesa el marcador <dd>
*/
void html_elemento_definicion(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_linea(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    /* Ignora */
  } else {
    html = contexto->contexto_html;
    if (html->anterior != NULL
     && html->anterior->nivel_lista == html->nivel_lista
     && html->anterior->inicio_x == html->inicio_x + PIXELES_DEFINICION)
      recupera_contexto(contexto);
  }
}

/*
** Procesa el marcador <center>
*/
void html_centro(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->alineacion = A_CENTRO;
  }
}

/*
** Procesa el marcador <div>
*/
void html_division(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html_alineacion(total, parametros, valores);
  }
}

/*
** Procesa el marcador <blockquote>
*/
void html_bloque(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  corte_de_parrafo(contexto);
  html_cerrar_p(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->inicio_x += PIXELES_BLOQUE;
    html->final_x -= PIXELES_BLOQUE;
  }
}

/*
** Pone el texto en italica
*/
void html_tipo_italica(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    html = contexto->contexto_html;
    contexto->x_actual -= (200 * escalas[html->tam] + 65535) / 65536;
    contexto->inicio_x = contexto->x_actual;
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo |= T_ITALICA;
    if (contexto->x_actual) {
      contexto->x_actual += (200 * escalas[html->tam] + 65535) / 65536;
      contexto->inicio_x = contexto->x_actual;
    }
  }
}

/*
** Pone el texto en negrita
*/
void html_tipo_negrita(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo |= T_NEGRITA;
  }
}

/*
** Pone el texto en tipo de máquina de escribir
*/
void html_tipo_maquina(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo = (html->tipo & ~T_MASCARA) | T_MAQUINA;
  }
}

/*
** Pone el texto con tachado
*/
void html_tachado(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo |= T_TACHADO;
  }
}

/*
** Pone el texto con subrayado
*/
void html_subrayado(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->tipo |= T_SUBRAYADO;
  }
}

/*
** Pone el texto en un tipo reducido
*/
void html_reduce(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    if (html->tam)
      html->tam--;
  }
}

/*
** Pone el texto en un tipo agrandado
*/
void html_agranda(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    if (html->tam != 6)
      html->tam++;
  }
}

/*
** Pone el texto en subindice
*/
void html_subindice(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->base_y += alturas[html->tam] / 3;
  }
}

/*
** Pone el texto en supraindice
*/
void html_supraindice(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    html->base_y -= alturas[html->tam] / 3;
  }
}

/*
** Pone el tamaño y color del texto
*/
void html_tipo(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;
  int a;
  char *ap;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    recupera_contexto(contexto);
  } else {
    crea_nuevo_contexto(contexto);
    html = contexto->contexto_html;
    a = html_parametro(total, parametros, "color");
    if (a >= 0)
      html_color(valores[a], &html->color);
    a = html_parametro(total, parametros, "size");
    if (a >= 0) {
      ap = valores[a];
      if (*ap == '+') {
        a = contexto->tam_base + atoi(ap + 1);
        if (a < 0)
          a = 0;
        if (a > 6)
          a = 6;
        html->tam = a;
      } else if (*ap == '-') {
        a = contexto->tam_base - atoi(ap + 1);
        if (a < 0)
          a = 0;
        if (a > 6)
          a = 6;
        html->tam = a;
      } else {
        a = atoi(ap);
        if (a < 1)
          a = 0;
        if (a > 7)
          a = 7;
        html->tam = a - 1;
      }
    }
  }
}

/*
** Selecciona el tamaño base del tipo de letra
*/
void html_tipo_base(struct contexto *contexto, int total,
  char *parametros[], char *valores[])
{
  struct contexto_html *html;
  int a;

  vaciar_texto(contexto);
  if (contexto->tipo_marcador) { /* Final */
    /* Ignora */
  } else {
    a = html_parametro(total, parametros, "size");
    if (a >= 0) {
      a = atoi(valores[a]);
      if (a < 1)
        a = 1;
      if (a > 7)
        a = 7;
      a--;
      contexto->tam_base = a;
      html = contexto->contexto_html;
      while (html != NULL) {
        html->tam = a;
        html = html->anterior;
      }
    }
  }
}

