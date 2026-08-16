/*
** Compilador de C para G11.
** Interfaz con el usuario.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 04-jun-1995.
** Revisión: 26-jul-1995. Ahora ap_mac se inicializa a 1.
** Revisión: 27-jul-1995. Se modifica p_include() para que sólo
**                        acepte la sintaxis estandard.
** Revisión: 10-ago-1995. Predefine los tipos estandares.
** Revisión: 12-ago-1995. Soporte para #include anidado.
** Revisión: 22-ago-1995. Inicialización de lista_estruct.
** Revisión: 23-ago-1995. Inicialización de ultima_estruct.
** Revisión: 24-ago-1995. Inicialización de lista_enum y ultimo_enum.
** Revisión: 07-sep-1995. Nueva función. inicializa().
** Revisión: 22-nov-1995. Inicialización de const_definidas, y
**                        inicialización de pos_globales a 3.
** Revisión: 08-may-1998. Inicialización de funcion a NULL.
** Revisión: 02-jul-1998. Soporte para el sistema de ventanas.
** Revisión: 15-jul-1998. Muestra el primer error encontrado
**                        unicamente. (no decia nada).
** Revisión: 10-ago-1998. Integración con Fénix C, utiliza los parametros
**                        proporcionados.
*/

/*
** !!! Debe soportar "Advertencias", que son pequeños detalles que no
**     impiden que la compilación prosiga.
** !!! Todos los errores y advertencias deben ser acumulados a una pequeña
**     ventana dependiente del editor (Fénix C), para que el usuario pueda
**     tenerlos a la vista.
*/

/*
** El compilador comienza su ejecución aquí.
*/
void main(void)
{
  wchar_t *nombre_tarea;
  char *ap;
  int tarea;

  nombre_tarea = info_tarea(tarea = lee_sistema(0));
  ap = camino_tarea;
  while (*nombre_tarea && *nombre_tarea != 0x0001)
    *ap++ = *nombre_tarea++;
  *ap = 0;
  if (*nombre_tarea++ == 0)
    return;
  ap = nombre_archivo;
  while (*nombre_tarea && *nombre_tarea != 0x0001)
    *ap++ = *nombre_tarea++;
  *ap = 0;
  if (*nombre_tarea == 0)
    return;
  ap = opciones;
  while (*ap++ = *nombre_tarea++);

  inicializa_todo();             /* Inicializa todo */
  presentacion();                /* Presentacion */
  nuevo_archivo(nombre_archivo); /* Primer archivo a procesar */
  if (archivo_actual != NULL) {
    abre_salida();               /* Prepara el archivo de salida */
    prologo();                   /* Emite el prologo */
    obt_lex();                   /* Obtiene un componente léxico */
    analiza();                   /* Efectua la compilación */
    if (nivel)
      error("Falta llave de cierre");
    epilogo();                   /* Emite el epilogo */
    cierra_salida();             /* Cierra la salida */
    reporta_errores();           /* Reporta errores detectados */
    libera_memoria();
    if (!errores)
      inicia_tarea("Ensamblador", nombre_salida);
  }
}

/*
** Inicializa todo.
*/
inicializa_todo()
{
  entrada = -1;
  comienzo_funcion = 0;   /* Ninguna función aún */
  dentro_funcion = NO;
  archivo_actual = NULL;  /* Archivo actual, ninguno */
  ultimo_bucle = NULL;    /* Limpia la cola de bucles */
  pila =                  /* Apuntador de pila */
  errores =               /* No hay errores */
  eof =                   /* No se ha alcanzado el fin del archivo */
  desvio_salida =         /* No se ha desviado la salida */
  nivel =                 /* No hay bloques abiertos */
  comienzo_funcion =      /* La función actual empezó en la linea 0 */
  lineas_totales =        /* No se ha leído ni una línea */
  dentro_funcion =        /* No esta dentro de una función */
  sig_etiq =              /* Inicia números de etiquetas */
  nivel_if =              /* No esta dentro de un #if... */
  nivel_incl =            /* No esta dentro de un #include */
  evadir_nivel = 0;       /* No esta evadiendo ningun texto de la entrada */
  ultimo_nodo = NULL;     /* Ultimo nodo usado del arbol */
  dentro_pp = NO;         /* No esta dentro del preprocesador */
  funcion_actual = NULL;  /* Ninguna función aún */
  funcion = NULL;         /* La función no ha sido compilada aún */
  lista_case = NULL;      /* No hay lista de cases */
  ultimo_case = NULL;
  adv_cs = SI;            /* Advertencia conversiones sospechosas */
  adv_ansi = SI;          /* Advertencia ANSI estricto en apuntadores */
  prog_grande = NO;       /* No genera codigo grande */
                          /* Prepara los tipos predefinidos */
  primer_tipo = NULL;
  ultimo_tipo = NULL;
  t_achar = crea_tipo(APUNTADOR);     /* Apuntador a char */
  t_uchar = crea_tipo(UCHAR);         /* unsigned char */
  t_achar->sig = t_uchar;
  t_func = crea_tipo(FUNCION);        /* Función que retorna int */
  t_sint = crea_tipo(SINT);           /* signed int */
  t_func->sig = t_sint;
  t_sshort = crea_tipo(SSHORT);       /* signed short */
  t_schar = crea_tipo(SCHAR);         /* signed char */
  t_awchar = crea_tipo(APUNTADOR);    /* Apuntador a wchar_t */
  t_ushort = crea_tipo(USHORT);       /* unsigned short */
  t_awchar->sig = t_ushort;
  t_uint = crea_tipo(UINT);           /* unsigned int */
  t_float = crea_tipo(FLOAT);         /* float */
  t_double = crea_tipo(DOUBLE);       /* double o long double */
  t_void = crea_tipo(VOID);           /* void */
}

/*
** Crea una cadena de tipos
*/
struct tipo *crea_tipo(int tipo)
{
  struct tipo *nuevo;

  nuevo = pide_espacio(&primer_tipo, &ultimo_tipo, sizeof(struct tipo));
  nuevo->sig = NULL;
  nuevo->tipo = tipo;
  nuevo->num_pars = 128;
  if (tipo == ARREGLO)
    nuevo->especial.tam = 0;
  else if (tipo == STRUCT)
    nuevo->especial.est = NULL;
  else if (tipo == FUNCION)
    nuevo->especial.proto = NULL;
  return nuevo;
}

void *pide_espacio(struct bloque **primero, struct bloque **ultimo,
                   int bytes)
{
  void *apuntador;
  struct bloque *nuevo;

  if (*ultimo == NULL) {
    if ((*primero = *ultimo = malloc(sizeof(struct bloque))) == NULL) {
      error("No hay memoria");
      cancela();
    }
    (*primero)->pos = 0;
    (*primero)->sig = NULL;
    (*primero)->ant = NULL;
  }
  while (1) {
    if ((*ultimo)->pos + bytes <= TAM_BLOQUE) {
      apuntador = (*ultimo)->datos + (*ultimo)->pos;
      (*ultimo)->pos += (bytes + 3) & ~3;
      return apuntador;
    }
    if ((*ultimo)->pos == 0) {
      error("Mal funcionamiento interno");
      cancela();
    }
    nuevo = malloc(sizeof(struct bloque));
    if (nuevo == NULL) {
      error("No hay memoria");
      cancela();
    }
    (*ultimo)->sig = nuevo;
    nuevo->ant = *ultimo;
    nuevo->sig = NULL;
    nuevo->pos = 0;
    *ultimo = nuevo;
  }
}

void salva_contexto(struct bloque **ultimo, struct contexto *resultado)
{
  resultado->variable = ultimo;
  resultado->bloque = *ultimo;
  resultado->posicion = (*ultimo)->pos;
}

void restaura_contexto(struct contexto *restaura)
{
  struct bloque *lista, *temp;

  lista = restaura->bloque->sig;
  while (lista != NULL) {
    temp = lista->sig;
    free(lista);
    lista = temp;
  }
  restaura->bloque->pos = restaura->posicion;
  restaura->bloque->sig = NULL;
  *(restaura->variable) = restaura->bloque;
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
** Cancela la compilación.
*/
void cancela(void)
{
  while (archivo_actual != NULL)
    fin_include();
  if (entrada > 0)
    fclose(entrada);
  if (funcion != NULL)
    libera_sentencias(funcion);
  libera_memoria();
  cierra_salida();
  hacia_consola();
  mensaje("Compilación cancelada.");
  emite_nueva_linea();
  exit(1);
}

/*
** Libera la memoria ocupada.
*/
void libera_memoria(void)
{
  struct nombres *ap1, *ap2;
  struct rotulo *ap3, *ap4;
  struct miembro *ap5, *ap6;
  struct enumerador *ap7, *ap8;
  struct macro *ap9, *ap10;
  int a;

  for (a = 0; a < NUM_PRIMO; a++) {
    ap1 = tabla[a];
    while (ap1 != NULL) {
      ap2 = ap1->sig;
      free(ap1);
      ap1 = ap2;
    }
    ap3 = tabla_estruct[a];
    while (ap3 != NULL) {
      ap5 = ap3->lista;
      while (ap5 != NULL) {
        ap6 = ap5->sig;
        free(ap5);
        ap5 = ap6;
      }
      ap4 = ap3->sig;
      free(ap3);
      ap3 = ap4;
    }
    ap7 = tabla_enum[a];
    while (ap7 != NULL) {
      ap8 = ap7->sig;
      free(ap7);
      ap7 = ap8;
    }
    ap9 = macros[a];
    while (ap9 != NULL) {
      ap10 = ap9->sig;
      free(ap9->definicion);
      free(ap9);
      ap9 = ap10;
    }
  }
  ap1 = locales;
  while (ap1 != NULL) {
    ap2 = ap1->sig;
    free(ap1);
    ap1 = ap2;
  }
  libera_sentencias(funcion);
  libera_expr();
  libera_espacio(primer_tipo);
}

/*
** Reporta los errores
*/
void reporta_errores(void)
{/*
  emite_nueva_linea();
  emite_texto("Hubo ");
  emite_numero(errores);
  emite_texto(" errores en la compilación.");
  emite_nueva_linea();*/
}

/*
** Presentación.
*/
void presentacion(void)
{
  void *clase;
  void *raiz;
  wchar_t *texto = L"Compilador de C";

  raiz = (void *) lee_sistema(3);
  clase = crea_clase(texto, interfaz, NULL, 64, 0, 0);
  if (clase == NULL)
    aviso_error(1);
  ventana = ventana_estandar(clase, raiz, texto, 256, 128, 0x0401);
  if (ventana == NULL)
    aviso_error(1);
  multitarea();
}

void ISO2wchar_t(wchar_t *destino, char *origen)
{
  while (*destino++ = *origen++) ;
}

wchar_t texto[512];

int interfaz(void *ventana, int mensaje, int par1, int par2)
{
  wchar_t *ap;

  if (mensaje == 0x02) {
    sel_color(ventana, lee_sistema(0x3d));
    rellena(ventana, par1 & 0xffff, (par1 >> 16) & 0xffff,
                     (par2 - par1) & 0xffff, ((par2 - par1) >> 16) & 0xffff);
    sel_color(ventana, lee_sistema(0x3e));
    sel_tipo(ventana, lee_sistema(0x9c));

    ilustra_texto(ventana, L"Compilando...", 4, 48);
    sel_tipo(ventana, lee_sistema(0x9e));
    if (archivo_actual != NULL) {
      ISO2wchar_t(texto, archivo_actual->nombre_real);
      ilustra_texto(ventana, texto, 4, 64);
      ap = formatea_numero(texto, archivo_actual->linea_real);
      ISO2wchar_t(ap, " líneas compiladas");
      ilustra_texto(ventana, texto, 4, 88);
    }
    ap = formatea_numero(texto, lineas_totales);
    ISO2wchar_t(ap, " líneas en total");
    ilustra_texto(ventana, texto, 4, 104);
  } else if (mensaje == 0x22) {
    cancela();
  }
  return 1;
}

void error(char *mensaje)
{
  wchar_t texto[200], *ap;

  if (errores++)
    return;
  sel_color(ventana, lee_sistema(0x3e));
  sel_tipo(ventana, lee_sistema(0x9e));
  ISO2wchar_t(texto, "Línea ");
  ap = texto + 6;
  ap = formatea_numero(texto, archivo_actual->linea_real);
  *ap++ = ':';
  *ap++ = ' ';
  ISO2wchar_t(ap, mensaje);
  ilustra_texto(ventana, texto, 4, 124);
}

wchar_t *formatea_numero(wchar_t *destino, int numero)
{
  if (numero >= 10)
    destino = formatea_numero(destino, numero / 10);
  *destino = (numero % 10) + '0';
  return destino + 1;
}

/*
** Obtiene el nombre del archivo de salida.
*/
void abre_salida(void)
{
  char *ap, *ap2;

  strcpy(linea_m, nombre_archivo);
  ap = linea_m + strlen(linea_m);
  ap2 = ap;
  while (*ap2 != '/' && *ap2 != '.')
    --ap2;
  if (*ap2 == '.')
    ap = ap2;
  *ap++ = '.';
  *ap++ = 'a';
  *ap++ = 0;
  strcpy(nombre_salida, linea_m);
  strcpy(nombre_salida + strlen(nombre_salida), opciones);
  salida = fopen(linea_m, "w");  /* Crea el archivo de salida */
  if (salida == 0)
    error("No se pudo crear archivo de salida");
  ap_buf_retrasado = buf_retrasado;
  total_lineas = 0;
}

/*
** Cierra el archivo de salida.
*/
void cierra_salida(void)
{
  hacia_archivo();
  if (salida >= 0)
    fclose(salida);     /* Si esta abierto, cerrarlo */
  salida = -1;          /* Marcar como cerrado */
}
