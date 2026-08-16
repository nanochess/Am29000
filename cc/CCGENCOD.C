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

