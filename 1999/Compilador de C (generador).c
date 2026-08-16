/*
** Compilador de C para G11.
** Generador de Codigo.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 03-jun-1995.
** Revisión: 07-sep-1998. Rediseño completo para que optimice subexpresiones
**                        comunes.
** Revisión: 12-sep-1998. Soporte para N_CCHAR
** Revisión: 12-oct-1998. La conversión de double a int ya no redondea, ANSI lo
**                        requiere así.
** Revisión: 28-oct-1998. Corrección de error en generación de subexpresiones.
** Revisión: 30-oct-1998. Corrección de error en generación de terminales.
** Revisión: 04-nov-1998. Desactivación de parte de anula_nodos para evitar
**                        generaciones de codigo incorrecto. Hace falta un
**                        rediseño completo.
** Revisión: 14-nov-1998. Corrección de tremendo error en generación de codigo
**                        para continue en do.
** Revisión: 24-nov-1998. Soporte para programas grandes.
** Revisión: 09-dic-1998. Corrección de defecto al generar codigo para copia de
**                        estructuras.
** Revisión: 12-dic-1998. Revisión de la generación de llamadas para funciones
**                        para permitir programas gigantescos.
*/

/*
** !!! Nuevo N_NUMF para números float, N_NUMPF cargaría directamente los
**     dos registros en vez de accesar los literales.
*/

/*
** Crea una variable virtual, después se asigna a registro o memoria.
*/
int var_virtual(int def, int tipo)
{
  int numero = variables_virtuales;

  if (variables_virtuales >= MAX_VIRTUALES) {
    error("Demasiadas variables");
    cancela();
  }
  virtuales[variables_virtuales++] = def;   /* Estado actual */
  virtuales[variables_virtuales++] = 0;     /* ¿Se pide su dirección? */
  virtuales[variables_virtuales++] = tipo;  /* Tipo de variable */
                                            /* 0= Int, 1= Double */
  return numero;
}

/*
** Genera el codigo para una función.
*/
void gen_funcion(char *n_func, struct sentencia *lista)
{
  int a;

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
  if (prog_grande) {
    for (a = 0; a < total_funciones; a++) {
      emite_etiq(funciones[a].etiqueta);
      dos_puntos();
      emite_nueva_linea();
      emite_texto("const gr121,");
      emite_nombre(funciones[a].func->nombre);
      emite_nueva_linea();
      emite_texto("consth gr121,");
      emite_nombre(funciones[a].func->nombre);
      emite_nueva_linea();
      emite_linea("jmpi gr121");
      emite_linea("nop");
    }
  }
}

/*
** Libera los registros temporales (gr98-gr111)
*/
void libera_temporales(void)
{
  int a;

  ultimo_temporal = 2;
  temporales[0] = 1;    /* Mantenemos ocupados gr96 y gr97 */
  exp_temp[0] = NULL;
  temporales[1] = 1;
  exp_temp[1] = NULL;
  for (a = 2; a < TEMP; a++) {
    temporales[a] = 0;
    exp_temp[a] = NULL;
  }
  pila_temporal = 0;    /* Nada temporal en pila de memoria */
}

/*
** Prologo de función:
**
**  o Asigna las variables virtuales a los registros o a la memoria.
**  o Asigna el espacio requerido.
**  o Copia los argumentos de la entrada (si es requerido)
*/
void prologo_funcion(void)
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
        case 0:              /* Asignar registros gr116 - gr119 */
          virtuales[variable] = ((virtuales[variable] >> 2) + 116) << 2;
          break;
        case 2:              /* Los parametros siguen en locales */
          virtuales[variable] &= ~3;
          break;
      }
      variable += 3;
    }
  } else {                   /* Pedimos espacio en la pila de registros */
    variable = 0;
    while (variable < variables_virtuales) {
      switch (virtuales[variable] & 3) {
        case 0:              /* Asignar registros locales */
          if (total_regs >= 0)
            virtuales[variable] = ((virtuales[variable] >> 2) +
                                    total_regs + 130) << 2;
          else
            virtuales[variable] = ((virtuales[variable] >> 2) + 130) << 2;
          break;
        case 2:              /* Los parametros ya tienen sus posiciones */
          virtuales[variable] &= ~3;
          break;
      }
      variable += 3;
    }
    if (total_regs >= 0)
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

void epilogo_funcion(void)
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
void gen_sentencias(struct sentencia *lista)
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
                       cancela();
    }
    ultima_sentencia = codigo->tipo;
    codigo = codigo->sig;
  }
}

void gen_if(struct sentencia *codigo)
{
  struct nodo *expr;
  int etiq1, etiq2;
  struct nodo *c1[TEMP];
  int c, s;

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
  etiq1 = ++sig_etiq;
  prueba(etiq1, expr);                     /* Checa la expresión */
  for (c = 0; c < TEMP; c++)
    c1[c] = exp_temp[c];
  gen_sentencias(codigo->def.t_if.lista1);
  if (codigo->def.t_if.lista2 != NULL) {   /* ¿ Es IF..ELSE ? */
    if ((ultima_sentencia != t_return) &&
        (ultima_sentencia != t_break) &&
        (ultima_sentencia != t_continue) &&
        (ultima_sentencia != t_goto))
      salto(etiq2 = ++sig_etiq);  /* Salta alrededor del codigo de else */
    else
      etiq2 = 0;
    gen_destino(etiq1);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = c1[c];
    gen_sentencias(codigo->def.t_if.lista2); /* Codigo ELSE */
    gen_destino(etiq2);
  } else {
    gen_destino(etiq1);
  }
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
}

void gen_while(struct sentencia *codigo)
{
  struct nodo *expr;
  int c;

  expr = codigo->def.t_while.expresion;
  if (expr->oper == N_CONST && expr->esp == 0)
    return;
  gen_destino(codigo->def.t_while.etiqueta_continue); /* Etiqueta del bucle */
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  if (expr->oper == N_CONST) {
    gen_sentencias(codigo->def.t_while.lista);
  } else if (codigo->def.t_while.lista != NULL) {
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
    gen_destino(codigo->def.t_while.etiqueta_break);  /* Etiqueta de salida */
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
}

void gen_do(struct sentencia *codigo)
{
  struct nodo *expr;
  int c;

  expr = codigo->def.t_while.expresion;
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  gen_destino(c = ++sig_etiq);
  gen_sentencias(codigo->def.t_while.lista);       /* Procesa una sentencia */
  gen_destino(codigo->def.t_while.etiqueta_continue); /* Etiqueta del bucle */
  if (expr->oper != N_CONST)
    prueba(codigo->def.t_while.etiqueta_break, expr); /* Checa la expresión */
  if (expr->oper != N_CONST || (expr->oper == N_CONST && expr->esp != 0))
    salto(c);
  gen_destino(codigo->def.t_while.etiqueta_break);    /* Etiqueta de salida */
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
}

void gen_for(struct sentencia *codigo)
{
  struct nodo *expr, *c1[TEMP];
  int etiq = ++sig_etiq;
  int c;

  usa_expr = NO;
  etiqueta(codigo->def.t_for.expresion1);
  gen_codigo(0, codigo->def.t_for.expresion1);
  expr = codigo->def.t_for.expresion2;
  gen_destino(etiq);
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  if (expr != NULL) {
    if (expr->oper == N_CONST && expr->esp == 0)
      return;
    if (expr->oper != N_CONST)
      prueba(codigo->def.t_for.etiqueta_break, expr); /* Checa la expresión */
  }
  gen_sentencias(codigo->def.t_for.lista);
  usa_expr = NO;
  gen_destino(codigo->def.t_for.etiqueta_continue);
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  etiqueta(codigo->def.t_for.expresion3);
  gen_codigo(0, codigo->def.t_for.expresion3);
  salto(etiq);
  gen_destino(codigo->def.t_for.etiqueta_break);
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
}

void gen_switch(struct sentencia *codigo)
{
  int c, etiq_default = 0;
  struct sentencia *analisis;

  usa_expr = SI;
  etiqueta(codigo->def.t_while.expresion);
  gen_codigo(0, codigo->def.t_while.expresion);
  analisis = codigo->def.t_while.lista;
  while (analisis != NULL) {
    if (analisis->tipo == t_case)
      compara_y_salta(analisis->def.t_case.constante,
                      analisis->def.t_case.etiqueta);
    else if (analisis->tipo == t_default)
      etiq_default = analisis->def.t_break.etiqueta;
    analisis = analisis->sig;
  }
  if (etiq_default)
    salto(etiq_default);
  else
    salto(codigo->def.t_while.etiqueta_break);
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  gen_sentencias(codigo->def.t_while.lista);
  gen_destino(codigo->def.t_while.etiqueta_break);
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
}

void gen_case(struct sentencia *codigo)
{
  int c;

  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  gen_destino(codigo->def.t_case.etiqueta);
}

void gen_etiqueta(struct sentencia *codigo)
{
  int c;

  for (c = 0; c < TEMP; c++)
    exp_temp[c] = NULL;
  gen_destino(codigo->def.t_break.etiqueta);
}

void gen_break(struct sentencia *codigo)
{
  salto(codigo->def.t_break.etiqueta);
}

void gen_return(struct sentencia *codigo)
{
  int c;
  struct nodo *c1[TEMP];

  for (c = 0; c < TEMP; c++)
    c1[c] = exp_temp[c];
  usa_expr = SI;
  if (codigo->def.t_return.informacion)
    copia_resultado(&codigo->def.t_return.expresion,
                     codigo->def.t_return.informacion);
  etiqueta(codigo->def.t_return.expresion);
  gen_codigo(0, codigo->def.t_return.expresion);
  epilogo_funcion();
  for (c = 0; c < TEMP; c++)
    exp_temp[c] = c1[c];
}

void gen_expresion(struct sentencia *codigo)
{
  usa_expr = NO;
  etiqueta(codigo->def.t_expresion.expresion);
  gen_codigo(0, codigo->def.t_expresion.expresion);
}

/*
** Prueba si la expresión es cero y salta.
*/
void prueba(int etiq, struct nodo *nodo)
{
  usa_expr = SI;
  if (nodo->oper == N_NOT)
    if (nodo->izq->oper == N_NOT)
      nodo = nodo->izq->izq;
  etiqueta(nodo);
  gen_codigo(etiq, nodo);
}

/*
** Genera codigo para un árbol de expresiones
*/
void gen_codigo(int etiq, struct nodo *expr)
{
  int reg;

  if (expr == NULL)
    return;
  if (etiq)
    salta_expr(expr, etiq, NO);
  else
    gen_nodo(expr, usa_expr ? 96 : 0);
}

/*
** Etiqueta un árbol.
**
** Cada nodo es etiquetado con el número de registros
** que requiere para evaluarse.
*/
void etiqueta(struct nodo *nodo)
{
  int min, max, op;
  struct nodo *temp;

  if (nodo == NULL || nodo->regs)
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
    if ((struct nodo *) nodo->esp != NULL)
      etiqueta((struct nodo *) nodo->esp);
  if ((op == N_FUNCI) || (op == N_FUNC) ||
      (op == N_ANDB) || (op == N_ORB) ||
      (op == N_COMA)) {
    nodo->regs = 1000;
  } else if (op == N_TRI) {
    temp = (struct nodo *) nodo->esp;
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
int en_registro(struct nodo *nodo)
{
  if (nodo->oper != N_DIR)
    return 0;
  if (virtuales[nodo->esp] & 3)
    return 0;
  return virtuales[nodo->esp + 2] + 1;
}

/*
** Averigua si el registro es una variable local
*/
int es_local(int reg)
{
  int tipo;

  tipo = tipo_reg(reg);
  return (tipo >= 2 && tipo <= 3);
}

/*
** Obtiene el registro de un nodo
*/
#define registro(nodo) (virtuales[nodo->esp] >> 2)

/*
** Averigua si hay una constante pequeña
*/
int es_constante(struct nodo *nodo)
{
  if (nodo->oper != N_CONST)
    return 0;
  return (nodo->esp >= 0 && nodo->esp <= 255);
}

/*
** Codigo para cada operador binario, y algunos unarios.
*/
void gen_oper(int oper, int inmediato, int reg1, int reg2,
              int constreg, int control)
{
  int reg;

  switch (oper) {
    case N_OR      :
    case N_AOR     : gen_inst1("or", inmediato, reg1, reg2, constreg);
                     break;
    case N_XOR     :
    case N_AXOR    : gen_inst1("xor", inmediato, reg1, reg2, constreg);
                     break;
    case N_AND     :
    case N_AAND    : if (constreg >= -256 && constreg < 0)
                       gen_inst1("andn", inmediato, reg1, reg2, ~constreg);
                     else
                       gen_inst1("and", inmediato, reg1, reg2, constreg);
                     break;
    case N_NOR     : gen_inst1("nor", inmediato, reg1, reg2, constreg);
                     break;
    case N_NAND    : gen_inst1("nand", inmediato, reg1, reg2, constreg);
                     break;
    case N_NXOR    : gen_inst1("xnor", inmediato, reg1, reg2, constreg);
                     break;
    case N_CD      :
    case N_ACD     : gen_inst1(control ? "srl" : "sra", inmediato, reg1, reg2,
                               constreg);
                     break;
    case N_CI      :
    case N_ACI     : gen_inst1("sll", inmediato, reg1, reg2, constreg);
                     break;
    case N_SUMA    :
    case N_ASUMA   : gen_inst1("add", inmediato, reg1, reg2, constreg);
                     break;
    case N_RESTA   :
    case N_ARESTA  : gen_inst1("sub", inmediato, reg1, reg2, constreg);
                     break;
    case N_RESTAI  : gen_inst1("subr", inmediato, reg1, reg2, constreg);
                     break;
    case N_MUL     :
    case N_AMUL    : gen_inst1("multiply", NO, reg1, reg2, constreg);
                     break;
    case N_MOD     :
    case N_AMOD    :
    case N_DIV     :
    case N_ADIV    : reg = pedir_reg(NO);
                     gen_inst1("sra", SI, reg, reg2, 31);
                     emite_texto("mtsr q,");
                     emite_registro(reg);
                     emite_nueva_linea();
                     gen_inst1(control ? "dividu" : "divide", NO, reg1, reg2,
                               constreg);
                     if (oper == N_MOD || oper == N_AMOD) {
                       emite_texto("mfsr ");
                       emite_registro(reg1);
                       emite_linea(",q");
                     }
                     libera_reg(reg);
                     break;
    case N_SMOD    :
    case N_SDIV    : emite_linea("mtsrim q,0");
                     gen_inst1("dividu", NO, reg1, reg2, constreg);
                     if (oper == N_SMOD) {
                       emite_texto("mfsr ");
                       emite_registro(reg1);
                       emite_linea(",q");
                     }
                     break;
    case N_NOT     : gen_inst1("cpeq", SI, reg1, reg2, 0);
                     break;
    case N_NEG     : gen_inst1("subr", SI, reg1, reg2, 0);
                     break;
    case N_NEGPF   : reg = pedir_reg(NO);
                     gen_inst3("const", SI, reg, 1);
                     gen_inst1("sll", SI, reg, reg, 31);
                     gen_inst1("xor", NO, reg1, reg2, reg);
                     if (reg1 != reg2)
                       gen_inst1("or", SI, reg1 + 1, reg2 + 1, 0);
                     libera_reg(reg);
                     break;
    case N_COM     : gen_inst1("nand", NO, reg1, reg2, reg2);
                     break;
    case N_CPAL    : gen_inst2("load 0,4,", NO, reg1, reg2);
                     break;
    case N_CBYTE   : gen_inst2("load 0,20,", NO, reg1, reg2);
                     gen_inst1("exbyte", SI, reg1, reg1, 0);
                     break;
    case N_CCHAR   : gen_inst2("load 0,20,", NO, reg1, reg2);
                     gen_inst1("exbyte", SI, reg1, reg1, 0);
                     gen_inst1("sll", SI, reg1, reg1, 24);
                     gen_inst1("sra", SI, reg1, reg1, 24);
                     break;
    case N_CSHORT  : gen_inst2("load 0,20,", NO, reg1, reg2);
                     gen_inst3("exhws", NO, reg1, reg1);
                     break;
    case N_CUSHORT : gen_inst2("load 0,20,", NO, reg1, reg2);
                     gen_inst1("exhw", SI, reg1, reg1, 0);
                     break;
    case N_CFLOAT  : gen_inst2("load 0,4,", NO, reg1, reg2);
                     gen_inst4(reg1, reg1, "0,0,2,1");
                     break;
    case N_CDOUBLE : gen_inst1("add", SI, reg1 + 1, reg2, 4);
                     gen_inst2("load 0,4,", NO, reg1 + 1, reg1 + 1);
                     gen_inst2("load 0,4,", NO, reg1, reg2);
                     break;
    case N_SUMAPF  : gen_inst1("dadd", NO, reg1, reg2, constreg);
                     break;
    case N_RESTAPF : gen_inst1("dsub", NO, reg1, reg2, constreg);
                     break;
    case N_MULPF   : gen_inst1("dmul", NO, reg1, reg2, constreg);
                     break;
    case N_DIVPF   : gen_inst1("ddiv", NO, reg1, reg2, constreg);
                     break;
    case N_ENTPF   : gen_inst4(reg1, reg2, "0,0,2,0");
                     break;
    case N_IGUALPF : gen_inst1("deq", NO, reg1, reg2, constreg);
                     break;
    case N_MAYORPF : gen_inst1("dgt", NO, reg1, reg2, constreg);
                     break;
    case N_MAYORIPF: gen_inst1("dge", NO, reg1, reg2, constreg);
                     break;
    case N_IGUAL   : gen_inst1("cpeq", inmediato, reg1, reg2, constreg);
                     break;
    case N_NOIGUAL : gen_inst1("cpneq", inmediato, reg1, reg2, constreg);
                     break;
    case N_MAYOR   : gen_inst1("cpgt", inmediato, reg1, reg2, constreg);
                     break;
    case N_MAYORI  : gen_inst1("cpge", inmediato, reg1, reg2, constreg);
                     break;
    case N_MENOR   : gen_inst1("cplt", inmediato, reg1, reg2, constreg);
                     break;
    case N_MENORI  : gen_inst1("cple", inmediato, reg1, reg2, constreg);
                     break;
    case N_SMAYOR  : gen_inst1("cpgtu", inmediato, reg1, reg2, constreg);
                     break;
    case N_SMAYORI : gen_inst1("cpgeu", inmediato, reg1, reg2, constreg);
                     break;
    case N_SMENOR  : gen_inst1("cpltu", inmediato, reg1, reg2, constreg);
                     break;
    case N_SMENORI : gen_inst1("cpleu", inmediato, reg1, reg2, constreg);
                     break;
    default        : error("Error del compilador");
                     break;
  }
  if ((oper >= N_IGUAL && oper <= N_MAYORIPF) || oper == N_NOT)
    if (!control)
      gen_inst1("srl", SI, reg1, reg1, 31);
}

int pedir_reg(int pareja)
{
  int c = ultimo_temporal;

  if (!pareja) {
    while (1) {
      if (temporales[c] == 0) {
        exp_temp[c] = NULL;
        temporales[c] = 2;
        ultimo_temporal = c + 1;
        if (ultimo_temporal == TEMP)
          ultimo_temporal = 0;
        return c + 96;
      }
      c++;
      if (c == TEMP)
        c = 0;
      if (c == ultimo_temporal)
        break;
    }
  } else {
    ultimo_temporal &= ~1;
    c &= ~1;
    while (1) {
      if (temporales[c] == 0 && temporales[c + 1] == 0) {
        exp_temp[c] = NULL;
        temporales[c] = 3;
        exp_temp[c + 1] = NULL;
        temporales[c + 1] = 3;
        ultimo_temporal = c + 2;
        if (ultimo_temporal == TEMP)
          ultimo_temporal = 0;
        ultimo_temporal = (ultimo_temporal + 2) & 15;
        return c + 96;
      }
      c += 2;
      if (c == TEMP)
        c = 0;
      if (c == ultimo_temporal)
        break;
    }
  }
  error("Temporales sobrepasados");
}

void libera_reg(int reg)
{
  if (reg < 98 || reg > 111)
    return;
  reg -= 96;
  temporales[reg] -= 2;
  if (temporales[reg] == 0)
    return;
  if (temporales[reg] == 1) {
    temporales[reg] = 0;
    temporales[reg + 1] = 0;
    return;
  }
  if (temporales[reg] < 0)
    error("Liberando registro temporal incorrecto");
}

/*
** Genera codigo para un nodo del arbol.
**
** Se le indica el registro donde se debe poner el resultado o 0 para
** indicar que el resultado no se ocupa.
*/
void gen_nodo(struct nodo *nodo, int resultado)
{
  int bits;

  if (nodo->oper >= N_AOR && nodo->oper <= N_ASIGNA) {
    checa_preinc(nodo, bits = 3);
    gen_nodo3(nodo, resultado);
  } else if (nodo->oper == N_SUMA && nodo->der->oper == N_CONST &&
            (nodo->izq->oper == N_DIR || nodo->izq->oper == N_DIRG ||
             nodo->izq->oper == N_DIRE)) {
    gen_nodo2(nodo->izq, resultado, nodo->der->esp);
    bits = tipo_reg(resultado);
    if (bits == 1)
      exp_temp[resultado - 96] = nodo;
    return;
  } else {
    switch (nodo->oper) {
      case N_APFUNC:
      case N_CONST :
      case N_LIT   :
      case N_DIR   :
      case N_DIRG  :
      case N_DIRE  :
      case N_APRES :
      case N_CEROPF:
      case N_NUMPF : gen_nodo2(nodo, resultado, 0);
                     return;
      case N_PAR   :
      case N_PARF  : checa_preinc(nodo, bits = 1);
                     gen_nodo(nodo->izq, resultado);
                     break;
      case N_FUNC  : gen_nodo1(nodo, resultado);
                     return;
      case N_FUNCI : checa_preinc(nodo, bits = 4);
                     gen_nodo1(nodo, resultado);
                     break;
      case N_INC   :
      case N_PINC  : bits = 0;
                     gen_nodo3(nodo, resultado);
                     break;
      case N_ANDB  :
      case N_ORB   : checa_preinc(nodo, bits = 3);
                     gen_nodo3(nodo, resultado);
                     break;
      case N_TRI   : checa_preinc(nodo, bits = 7);
                     gen_nodo3(nodo, resultado);
                     break;
      case N_COMA  : checa_preinc(nodo, 1);
                     gen_nodo(nodo->izq, 0);
                     checa_posinc(nodo, 1);
                     checa_preinc(nodo, bits = 2);
                     gen_nodo(nodo->der, resultado);
                     break;
      case N_NEG   :
      case N_NEGPF :
      case N_COM   :
      case N_CBYTE :
      case N_CCHAR :
      case N_CPAL  :
      case N_CSHORT:
      case N_CUSHORT:
      case N_CFLOAT:
      case N_CDOUBLE:
      case N_ENTPF :
      case N_PFENT : checa_preinc(nodo, bits = 1);
                     gen_nodo4(nodo, resultado);
                     break;
      default      : checa_preinc(nodo, bits = 3);
                     gen_nodo4(nodo, resultado);
                     break;
    }
  }
  checa_posinc(nodo, bits);
}

void checa_preinc(struct nodo *nodo, int bits)
{
  if (bits & 1)
    nodo_preinc(&nodo->izq);
  if (bits & 2)
    nodo_preinc(&nodo->der);
  if (bits & 4)
    nodo_preinc((struct nodo **) (&nodo->esp));
}

void checa_posinc(struct nodo *nodo, int bits)
{
  if (bits & 1)
    nodo_posinc(nodo->izq);
  if (bits & 2)
    nodo_posinc(nodo->der);
  if (bits & 4)
    nodo_posinc((struct nodo *) nodo->esp);
}

void nodo_preinc(struct nodo **nodo)
{
  int temp, op;

  op = (*nodo)->oper;
  if ((op == N_INC || op == N_PINC) && en_registro((*nodo)->izq)) {
    temp = (int) (*nodo)->der;
    switch ((*nodo)->esp) {
      case SINT  :
      case UINT  : crea_nodo(N_CPAL, (*nodo)->izq, NULL, op == N_INC ? 0 : temp);
                   break;
      case SSHORT: crea_nodo(N_CSHORT, (*nodo)->izq, NULL, op == N_INC ? 0 : temp);
                   break;
      case USHORT: crea_nodo(N_CUSHORT, (*nodo)->izq, NULL, op == N_INC ? 0 : temp);
                   break;
      case SCHAR : crea_nodo(N_CCHAR, (*nodo)->izq, NULL, op == N_INC ? 0 : temp);
                   break;
      default    : crea_nodo(N_CBYTE, (*nodo)->izq, NULL, op == N_INC ? 0 : temp);
                   break;
    }
    *nodo = ultimo_nodo;
    if (op == N_INC) {
      if (temp < 0)
        gen_inst1("sub", SI, registro((*nodo)->izq), registro((*nodo)->izq), -temp);
      else
        gen_inst1("add", SI, registro((*nodo)->izq), registro((*nodo)->izq), temp);
    }
  }
}

void nodo_posinc(struct nodo *nodo)
{
  int temp, op;

  op = nodo->oper;
  temp = nodo->esp;
  if ((op == N_CPAL || op == N_CSHORT || op == N_CUSHORT
    || op == N_CBYTE || op == N_CCHAR)
   && temp != 0) {
    if (temp < 0)
      gen_inst1("sub", SI, registro(nodo->izq), registro(nodo->izq), -temp);
    else
      gen_inst1("add", SI, registro(nodo->izq), registro(nodo->izq), temp);
    anula_nodos(registro(nodo->izq));
  }
}

void gen_nodo1(struct nodo *nodo, int resultado)
{
  int op, total, cuenta, pila_extra, tam_resultado, pos_pila, anota_pila;
  int dir_complicada, regresa_struct, reg, temp, reg1, reg2, reg3, tam;
  int real, etiq;
  struct nodo *nodo_temp;
  struct nombres *ap;

  op = nodo->oper;
  if ((op == N_FUNC) || (op == N_FUNCI)) {
    /*
    ** Paso 1: Si es una función indirecta puede tener una dirección
    **         complicada o simple.
    */
    if (op == N_FUNCI) {
      nodo_temp = (struct nodo *) nodo->esp;
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
        if ((int) nodo_temp->der)
          pila_extra += (int) nodo_temp->der * 4;
        else if (nodo_temp->regs >= 12)
          total++;
      }
      nodo_temp = (struct nodo *) nodo_temp->esp;
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
      if (nodo_temp->oper == N_PAR && (int) nodo_temp->der != 0) {
        anota_pila = pila_temporal;
        reg = pedir_reg(NO);
        gen_nodo(nodo_temp, reg);
        reg1 = pedir_reg(NO);
        reg2 = pedir_reg(NO);
        reg3 = pedir_reg(NO);
        gen_inst1("add", SI, reg1, 125, pos_pila + pila_temporal);
        gen_inst3("const", SI, reg2, tam = (int) nodo_temp->der - 2);
        if (tam < -65536 && tam > 65535)
          gen_inst3("consth", SI, reg2, tam);
        gen_destino(etiq = ++sig_etiq);
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
      nodo_temp = (struct nodo *) nodo_temp->esp;
    }

    /*
    ** Paso 4: Todos los argumentos que requieren todos los registros
    **         (ej. llamada a función) se evaluan primero, en el
    **         caso de multiples argumentos complicados entonces
    **         se van guardando en la pila y luego se cargan.
    **         También resolvemos los casos de dirección complicada.
    */
    if (op == N_FUNCI && dir_complicada) {
      gen_nodo((struct nodo *) nodo->esp, 120);
      gen_inst1("sub", SI, 125, 125, 4);
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
          && (int) nodo_temp->der == 0) {
          if (nodo_temp->oper == N_PARF)
            reg = (reg + 1) & ~1;
          if (nodo_temp->regs >= 12) {
            if (cuenta--) {
              gen_nodo(nodo_temp, 120);
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
              gen_nodo(nodo_temp, reg);
          }
          if (nodo_temp->oper == N_PAR)
            reg++;
          else
            reg += 2;
        }
        nodo_temp = (struct nodo *) nodo_temp->esp;
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
        && (int) nodo_temp->der == 0) {
        if (nodo_temp->oper == N_PARF)
          reg = (reg + 1) & ~1;
        if (nodo_temp->regs < 12)
          gen_nodo(nodo_temp, reg);
        if (nodo_temp->oper == N_PAR)
          reg++;
        else
          reg += 2;
      }
      nodo_temp = (struct nodo *) nodo_temp->esp;
    }

    /*
    ** Paso 5: Generamos la llamada de función.
    */
    if (op == N_FUNCI) {
      if (dir_complicada) {
        gen_inst2("load 0,4,", NO, 120, 125);
        emite_linea("calli lr0,gr120");
        gen_inst1("add", SI, 125, 125, 4);
        pila_temporal -= 4;
      } else {
        gen_nodo((struct nodo *) nodo->esp, 120);
        emite_linea("calli lr0,gr120");
        emite_linea("nop");
      }
    } else {
      ap = (struct nombres *) nodo->esp;
      llamada(ap);
    }
    if (resultado == 0)
      resultado = 96;
    if (!regresa_struct && pila_extra) {
      gen_inst1("add", SI, 125, 125, pila_extra);
      pila_temporal -= pila_extra;
    }
    if (resultado != 96) {
      gen_inst1("or", SI, resultado, 96, 0);
      if ((int) nodo->der == 2)
        gen_inst1("or", SI, resultado + 1, 97, 0);
    }
    for (cuenta = 0; cuenta < TEMP; cuenta++)
      exp_temp[cuenta] = NULL;
  }
}

void gen_nodo2(struct nodo *nodo, int resultado, int ajuste)
{
  int op, temp, etiq;
  struct nombres *ap;

  op = nodo->oper;
  if (op == N_APFUNC) {
    if (resultado == 0)
      return;
    ap = (struct nombres *) nodo->esp;
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(ap->nombre);
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(ap->nombre);
    emite_nueva_linea();
    return;
  }
  if (op == N_CEROPF) {
    if (resultado == 0)
      return;
    gen_inst3("const", SI, resultado, 0);
    gen_inst3("const", SI, resultado + 1, 0);
    return;
  }
  if (op == N_NUMPF) {
    if (resultado == 0)
      return;
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
      return;
    gen_inst3("const", SI, resultado, nodo->esp);
    if (nodo->esp < -65536 || nodo->esp > 65535)
      gen_inst3("consth", SI, resultado, nodo->esp);
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    return;
  }
  if (op == N_LIT) {
    if (resultado == 0)
      return;
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
      return;
    gen_inst1("add", SI, resultado, 125, pila_temporal + pila);
    return;
  }
  if (op == N_DIR) {
    if (resultado == 0)
      return;
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    gen_inst1("add", SI, resultado, 125,
              (virtuales[nodo->esp] >> 2) + pila_temporal + ajuste);
    return;
  }
  if (op == N_DIRG) {
    if (resultado == 0)
      return;
    ap = (struct nombres *) nodo->esp;
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(ap->nombre);
    if (ajuste) {
      emite_texto("+");
      emite_numero(ajuste);
    }
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    emite_nombre(ap->nombre);
    if (ajuste) {
      emite_texto("+");
      emite_numero(ajuste);
    }
    emite_nueva_linea();
    return;
  }
  if (op == N_DIRE) {
    if (resultado == 0)
      return;
    etiq = nodo->esp;
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    if (temp >= 2 && temp <= 4)
      temp += 3;
    estado_buf[total_lineas] = temp;
    emite_texto("const ");
    emite_registro(resultado);
    emite_texto(",");
    emite_etiq(etiq);
    if (ajuste) {
      emite_texto("+");
      emite_numero(ajuste);
    }
    emite_nueva_linea();
    estado_buf[total_lineas] = temp;
    emite_texto("consth ");
    emite_registro(resultado);
    emite_texto(",");
    emite_etiq(etiq);
    if (ajuste) {
      emite_texto("+");
      emite_numero(ajuste);
    }
    emite_nueva_linea();
    return;
  }
}

void gen_nodo3(struct nodo *nodo, int resultado)
{
  int op, reg, temp, reg1, reg2, c;
  int real, inmediato, etiq;

  op = nodo->oper;
  if (op == N_ASIGNA) {
    real = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    if (en_registro(nodo->der)) {
      if (nodo->esp == FLOAT) {
        if (resultado == 0)
          resultado = 96;
        gen_nodo(nodo->izq, resultado);
        gen_inst4(registro(nodo->der), resultado, "0,0,1,2");
      } else {
        gen_nodo(nodo->izq, registro(nodo->der));
        if (resultado != 96 ||
           (nodo->izq->oper != N_FUNC && nodo->izq->oper != N_FUNCI)) {
          if (resultado && resultado != registro(nodo->der)) {
            gen_inst1("or", 1, resultado, registro(nodo->der), 0);
            if (real)
              gen_inst1("or", 1, resultado + 1, registro(nodo->der) + 1, 0);
          }
        }
      }
      anula_nodos(registro(nodo->der));
      if (tipo_reg(resultado) == 1)
        exp_temp[resultado - 96] = NULL;
      return;
    }
    if (nodo->der->regs < 12) {
      if (resultado == 0 && es_constante(nodo->izq)
      && (nodo->esp == SSHORT || nodo->esp == USHORT
       || nodo->esp == SCHAR  || nodo->esp == UCHAR)) {
        resultado = nodo->izq->esp + 256;
      } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
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
        if (reg = existe_nodo(nodo->der)) ;
        else {
          reg = pedir_reg(NO);
          gen_nodo(nodo->der, reg);
        }
        almacena_mem(nodo->esp, resultado, reg);
        libera_reg(reg);
      }
      anula_nodos(0);
      if (tipo_reg(resultado) == 1)
        exp_temp[resultado - 96] = NULL;
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
    anula_nodos(0);
    if (tipo_reg(resultado) == 1)
      exp_temp[resultado - 96] = NULL;
    return;
  }
  if (op >= N_AOR && op <= N_AMOD) {
    real = (nodo->esp == FLOAT || nodo->esp == DOUBLE);
    if (real) {
      if (op == N_ASUMA)
        op = N_SUMAPF;
      else if (op == N_ARESTA)
        op = N_RESTAPF;
      else if (op == N_AMUL)
        op = N_MULPF;
      else if (op == N_ADIV)
        op = N_DIVPF;
    }
    if (en_registro(nodo->der)) {    /* Es operación con registro */
      if (nodo->esp == SINT || nodo->esp == UINT) {
        if (es_constante(nodo->izq) &&
           (op >= N_AOR && op <= N_ARESTA)) {
          gen_oper(op, SI, registro(nodo->der), registro(nodo->der),
                   nodo->izq->esp, nodo->esp == UINT);
        } else {
          if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
            reg = registro(nodo->izq->izq);
          else if (reg = existe_nodo(nodo->izq)) ;
          else {
            reg = pedir_reg(NO);
            gen_nodo(nodo->izq, reg);
          }
          gen_oper(op, NO, registro(nodo->der), registro(nodo->der), reg,
                   nodo->esp == UINT);
          libera_reg(reg);
        }
        if (resultado)
          gen_inst1("or", SI, resultado, registro(nodo->der), 0);
        anula_nodos(registro(nodo->der));
        if (tipo_reg(resultado) == 1)
          exp_temp[resultado - 96] = NULL;
        return;
      } else {
        if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
          reg = registro(nodo->izq->izq);
        else if (!real && (reg = existe_nodo(nodo->izq))) ;
        else {
          reg = pedir_reg(real);
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
          gen_inst4(registro(nodo->der), resultado, "0,0,1,2");
        libera_reg(reg);
        anula_nodos(registro(nodo->der));
        if (tipo_reg(resultado) == 1)
          exp_temp[resultado - 96] = NULL;
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
        else if (es_constante(nodo->izq) && op >= N_AOR && op <= N_ARESTA) {
          inmediato = SI;
          reg1 = nodo->izq->esp;
        } else {
          reg1 = resultado;
          gen_nodo(nodo->izq, reg1);
        }
        if (reg2 = existe_nodo(nodo->der)) ;
        else {
          reg2 = pedir_reg(NO);
          gen_nodo(nodo->der, reg2);
        }
        temp = pedir_reg(real);
        carga_mem(nodo->esp, temp, reg2);
        gen_oper(op, inmediato, resultado, temp, reg1, nodo->esp == UINT);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        libera_reg(reg2);
        anula_nodos(0);
        if (tipo_reg(resultado) == 1)
          exp_temp[resultado - 96] = NULL;
        return;
      } else if ((nodo->der->regs > nodo->izq->regs) &&
                 (nodo->izq->regs < 12)) {
        if (resultado == 0)
          resultado = 96;
        inmediato = NO;
        if (reg2 = existe_nodo(nodo->der)) ;
        else {
          reg2 = pedir_reg(NO);
          gen_nodo(nodo->der, reg2);
        }
        if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq))
          reg1 = registro(nodo->izq->izq);
        else if (es_constante(nodo->izq) && op >= N_AOR && op <= N_ARESTA) {
          inmediato = SI;
          reg1 = nodo->izq->esp;
        } else {
          reg1 = resultado;
          gen_nodo(nodo->izq, reg1);
        }
        temp = pedir_reg(real);
        carga_mem(nodo->esp, temp, reg2);
        gen_oper(op, inmediato, resultado, temp, reg1, nodo->esp == UINT);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        libera_reg(reg2);
        anula_nodos(0);
        if (tipo_reg(resultado) == 1)
          exp_temp[resultado - 96] = NULL;
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
        gen_oper(op, NO, resultado, temp, resultado, nodo->esp == UINT);
        almacena_mem(nodo->esp, resultado, reg2);
        libera_reg(temp);
        anula_nodos(0);
        if (tipo_reg(resultado) == 1)
          exp_temp[resultado - 96] = NULL;
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
      anula_nodos(registro(nodo->izq));
    } else {
      if (reg1 = existe_nodo(nodo->izq)) ;
      else
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
        anula_nodos(0);
      } else {
        carga_mem(nodo->esp, resultado, reg1);
        if (temp < 0)
          gen_inst1("sub", SI, resultado, resultado, -temp);
        else
          gen_inst1("add", SI, resultado, resultado, temp);
        almacena_mem(nodo->esp, resultado, reg1);
        anula_nodos(0);
      }
      libera_reg(reg1);
    }
    return;
  }
  if (op == N_ANDB) {
    salta_expr(nodo->izq, temp = ++sig_etiq, NO);
    salta_expr(nodo->der, temp, NO);
    if (resultado == 0)
      resultado = 96;
    gen_inst3("const", SI, resultado, 1);
    salto(etiq = ++sig_etiq);
    gen_destino(temp);
    gen_inst3("const", SI, resultado, 0);
    gen_destino(etiq);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
    return;
  }
  if (op == N_ORB) {
    salta_expr(nodo->izq, etiq = ++sig_etiq, SI);
    salta_expr(nodo->der, temp = ++sig_etiq, NO);
    if (resultado == 0)
      resultado = 96;
    gen_destino(etiq);
    gen_inst3("const", SI, resultado, 1);
    salto(etiq = ++sig_etiq);
    gen_destino(temp);
    gen_inst3("const", SI, resultado, 0);
    gen_destino(etiq);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
    return;
  }
  if (op == N_TRI) {
    salta_expr((struct nodo *) nodo->esp, etiq = ++sig_etiq, NO);
    if (resultado == 0)
      resultado = 96;
    gen_nodo(nodo->izq, resultado);
    salto(temp = ++sig_etiq);
    gen_destino(etiq);
    gen_nodo(nodo->der, resultado);
    gen_destino(temp);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
    return;
  }
}

void gen_nodo4(struct nodo *nodo, int resultado)
{
  int op, anota_pila;
  int reg, temp, reg1, reg2, reg3, tam;
  int real, inmediato, etiq, terminal;

  op = nodo->oper;
  if (op == N_COPIA) {
    if (resultado == 0)
      resultado = 96;
    anota_pila = pila_temporal;
    reg = pedir_reg(NO);
    if ((nodo->izq->regs >= nodo->der->regs) &&
        (nodo->der->regs < 12)) {
      gen_nodo(nodo->izq, resultado);
      gen_nodo(nodo->der, reg);
    } else if ((nodo->der->regs > nodo->izq->regs) &&
               (nodo->izq->regs < 12)) {
      gen_nodo(nodo->der, reg);
      gen_nodo(nodo->izq, resultado);
    } else {
      gen_nodo(nodo->der, reg);
      gen_inst1("sub", 1, 125, 125, 4);
      pila_temporal += 4;
      gen_inst2("store 0,4,", NO, reg, 125);
      gen_nodo(nodo->izq, resultado);
      gen_inst2("load 0,4,", NO, reg, 125);
      gen_inst1("add", 1, 125, 125, 4);
      pila_temporal -= 4;
    }
    exp_temp[reg - 96] = NULL;
    reg1 = pedir_reg(NO);
    gen_inst1("or", SI, reg1, resultado, 0);
    reg2 = pedir_reg(NO);
    reg3 = pedir_reg(NO);
    gen_inst3("const", SI, reg2, tam = nodo->esp - 2);
    if (tam < -65536 && tam > 65535)
      gen_inst3("consth", SI, reg2, tam);
    gen_destino(etiq = ++sig_etiq);
    gen_inst2("load 0,4,", NO, reg3, reg);
    gen_inst1("add", SI, reg, reg, 4);
    gen_inst2("store 0,4,", NO, reg3, reg1);
    gen_inst1("add", SI, reg1, reg1, 4);
    emite_texto("jmpfdec ");
    emite_registro(reg2);
    emite_texto(",");
    emite_etiq(etiq);
    emite_nueva_linea();
    gen_libre(1);
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
      if (resultado && resultado != registro(nodo->izq))
        gen_inst1("or", SI, resultado, registro(nodo->izq), 0);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, registro(nodo->izq->izq));
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    } else if (reg1 = existe_nodo(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, reg1);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    }
  }
  if (op == N_CCHAR) {
    if (en_registro(nodo->izq)) {
      if (resultado) {
        gen_inst1("sll", SI, resultado, registro(nodo->izq), 24);
        gen_inst1("sra", SI, resultado, resultado, 24);
      }
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("exbyte", SI, resultado, resultado, 0);
      gen_inst1("sll", SI, resultado, resultado, 24);
      gen_inst1("sra", SI, resultado, resultado, 24);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    } else if (reg1 = existe_nodo(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, reg1);
      gen_inst1("exbyte", SI, resultado, resultado, 0);
      gen_inst1("sll", SI, resultado, resultado, 24);
      gen_inst1("sra", SI, resultado, resultado, 24);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    }
  }
  if (op == N_CBYTE) {
    if (en_registro(nodo->izq)) {
      if (resultado)
        gen_inst1("and", SI, resultado, registro(nodo->izq), 255);
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("exbyte", SI, resultado, resultado, 0);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    } else if (reg1 = existe_nodo(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, reg1);
      gen_inst1("exbyte", SI, resultado, resultado, 0);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    }
  }
  if (op == N_CSHORT) {
    if (en_registro(nodo->izq)) {
      if (resultado) {
        gen_inst1("sll", 1, resultado, registro(nodo->izq), 16);
        gen_inst1("sra", 1, resultado, resultado, 16);
      }
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst3("exhws", NO, resultado, resultado);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    } else if (reg1 = existe_nodo(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, reg1);
      gen_inst3("exhws", NO, resultado, resultado);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    }
  }
  if (op == N_CUSHORT) {
    if (en_registro(nodo->izq)) {
      if (resultado) {
        gen_inst1("or", SI, resultado, registro(nodo->izq), 0);
        gen_inst3("consth", SI, resultado, 0);
      }
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("exhw", SI, resultado, resultado, 0);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    } else if (reg1 = existe_nodo(nodo->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,20,", NO, resultado, reg1);
      gen_inst1("exhw", SI, resultado, resultado, 0);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      return;
    }
  }
  if (op == N_PFENT) {
    if (resultado == 0)
      resultado = 96;
    if (resultado & 1) {
      reg1 = pedir_reg(SI);
      gen_nodo(nodo->izq, reg1);
      gen_inst4(resultado, reg1, "0,3,0,2");
      libera_reg(reg1);
    } else {
      gen_nodo(nodo->izq, resultado);
      gen_inst4(resultado, resultado, "0,3,0,2");
    }
    return;
  }
  if (op == N_CFLOAT) {
    if (en_registro(nodo->izq)) {
      if (resultado)
        gen_inst4(resultado, registro(nodo->izq), "0,0,2,1");
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, registro(nodo->izq->izq));
      gen_inst4(resultado, resultado, "0,0,2,1");
      return;
    }
  }
  if (op == N_CDOUBLE) {
    if (en_registro(nodo->izq)) {
      if (resultado && resultado != registro(nodo->izq)) {
        gen_inst1("or", 1, resultado, registro(nodo->izq), 0);
        gen_inst1("or", 1, resultado + 1, registro(nodo->izq) + 1, 0);
      }
      return;
    } else if (nodo->izq->oper == N_CPAL && en_registro(nodo->izq->izq)) {
      if (resultado == 0)
        resultado = 96;
      gen_inst2("load 0,4,", NO, resultado, registro(nodo->izq->izq));
      gen_inst1("add", SI, resultado + 1, registro(nodo->izq->izq), 4);
      gen_inst2("load 0,4,", NO, resultado + 1, resultado + 1);
      return;
    }
  }
  terminal = 0;
  real = (op >= N_IGUALPF && op <= N_MAYORIPF)
      || (op >= N_SUMAPF && op <= N_DIVPF);
  if (nodo->der != NULL) {  /* Detecta un nodo binario */
    if ((nodo->izq->regs >= nodo->der->regs) &&
        (nodo->der->regs < 12)) {
      if (resultado == 0)
        resultado = 96;
      if ((nodo->izq->oper == N_CPAL || nodo->izq->oper == N_CDOUBLE) &&
          en_registro(nodo->izq->izq))
        reg1 = registro(nodo->izq->izq);
      else if (!real && (reg1 = existe_nodo(nodo->izq)))
        terminal = 1;
      else if (nodo->izq->oper == N_FUNC || nodo->izq->oper == N_FUNCI) {
        reg1 = 96;
        gen_nodo(nodo->izq, reg1);
      } else {
        if (es_local(resultado) || (terminal = nodo_terminal(nodo->izq)))
          reg1 = pedir_reg(real);
        else
          reg1 = resultado;
        gen_nodo(nodo->izq, reg1);
      }
      inmediato = NO;
      if ((nodo->der->oper == N_CPAL || nodo->der->oper == N_CDOUBLE) &&
          en_registro(nodo->der->izq))
        reg2 = registro(nodo->der->izq);
      else if (es_constante(nodo->der) && op >= N_OR && op <= N_RESTA) {
        inmediato = SI;
        reg2 = nodo->der->esp;
      } else if (!real && (reg2 = existe_nodo(nodo->der))) ;
      else {
        reg2 = pedir_reg(real);
        gen_nodo(nodo->der, reg2);
      }
      gen_oper(op, inmediato, resultado, reg1, reg2, nodo->esp);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      if (!inmediato)
        libera_reg(reg2);
      if (es_local(resultado) || terminal)
        libera_reg(reg1);
      return;
    } else if ((nodo->der->regs > nodo->izq->regs) &&
               (nodo->izq->regs < 12)) {
      if (resultado == 0)
        resultado = 96;
      if ((nodo->der->oper == N_CPAL || nodo->der->oper == N_CDOUBLE) &&
          en_registro(nodo->der->izq))
        reg2 = registro(nodo->der->izq);
      else if (!real && (reg2 = existe_nodo(nodo->der))) ;
      else if (nodo->der->oper == N_FUNC || nodo->der->oper == N_FUNCI) {
        reg2 = 96;
        gen_nodo(nodo->der, reg2);
      } else {
        reg2 = pedir_reg(real);
        gen_nodo(nodo->der, reg2);
      }
      if ((nodo->izq->oper == N_CPAL || nodo->izq->oper == N_CDOUBLE) &&
           en_registro(nodo->izq->izq))
        reg1 = registro(nodo->izq->izq);
      else if (!real && (reg1 = existe_nodo(nodo->izq)))
        terminal = 1;
      else {
        if (es_local(resultado) || reg2 == resultado
        || (terminal = nodo_terminal(nodo->izq)))
          reg1 = pedir_reg(real);
        else
          reg1 = resultado;
        gen_nodo(nodo->izq, reg1);
      }
      gen_oper(op, NO, resultado, reg1, reg2, nodo->esp);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      libera_reg(reg2);
      if (es_local(resultado) || reg2 == resultado || terminal)
        libera_reg(reg1);
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
      if (es_local(resultado))
        reg1 = pedir_reg(real);
      else
        reg1 = resultado;
      gen_nodo(nodo->izq, reg1);
      if (real) {
        gen_inst2("load 0,4,", NO, 121, 125);
        gen_inst1("add", 1, 125, 125, 4);
        pila_temporal -= 4;
      }
      gen_inst2("load 0,4,", NO, 120, 125);
      gen_inst1("add", 1, 125, 125, 4);
      pila_temporal -= 4;
      gen_oper(op, NO, resultado, reg1, 120, nodo->esp);
      temp = tipo_reg(resultado);
      if (temp == 1)
        exp_temp[resultado - 96] = nodo;
      if (es_local(resultado))
        libera_reg(reg1);
      return;
    }
  } else {
    if (reg1 = existe_nodo(nodo->izq))
      terminal = 1;
    else {
      gen_nodo(nodo->izq, reg1 = resultado);
      terminal = 0;
    }
    gen_oper(op, NO, resultado, reg1, reg1, NO);
    temp = tipo_reg(resultado);
    if (temp == 1)
      exp_temp[resultado - 96] = nodo;
    if (terminal)
      libera_reg(reg1);
    return;
  }
}

/*
** Anula todas las subexpresiones que utilicen el registro indicado,
** si se indica registro 0 entonces anula todas las subexpresiones que
** utilicen memoria.
*/
void anula_nodos(int reg)
{
  int c;
  struct nodo *nodo;

  for (c = 0; c < TEMP; c++) {
    nodo = exp_temp[c];
    if (nodo != NULL) {
      if (nodo->oper != N_CONST && nodo->oper != N_DIRG &&
          nodo->oper != N_DIRE && nodo->oper != N_APFUNC) {
        if (reg == 0 || anula(nodo, reg)) {
          exp_temp[c] = NULL;
        }
      }
    }
  }
}

int anula(struct nodo *nodo, int reg)
{
  int op;

  op = nodo->oper;
  if (nodo->izq != NULL)
    if (anula(nodo->izq, reg))
      return 1;
  if ((op != N_INC) && (op != N_PINC) && (op != N_RESULTA) && (op != N_FUNC)
   && (op != N_FUNCI) && (op != N_PAR) && (op != N_PARF) && (nodo->der != NULL))
    if (anula(nodo->der, reg))
      return 1;
  if ((op == N_FUNCI) || (op == N_PAR) || (op == N_PARF) ||
      (op == N_TRI) || (op == N_RESULTA))
    if ((struct nodo *) nodo->esp != NULL)
      if (anula((struct nodo *) nodo->esp, reg))
        return 1;
  if (op == N_DIR) {
    if (reg) {
      if (virtuales[nodo->esp] & 3)
        return 0;
      if ((virtuales[nodo->esp] >> 2) == reg)
        return 1;
      else
        return 0;
    } else {
      if (virtuales[nodo->esp] & 3)
        return 1;
      return 0;
    }
  } else if (op == N_DIRG && reg == 0) {
    return 1;
  } else
    return 0;
}

int existe_nodo(struct nodo *nodo)
{
  int c;

  for (c = 0; c < TEMP; c++) {
    if (exp_temp[c] == nodo) {
      temporales[c] += 2;
      return c + 96;
    }
  }
  return 0;
}

int nodo_terminal(struct nodo *nodo)
{
  int op;

  op = nodo->oper;
  while (1) {
    if (nodo->izq == NULL)
      return 1;
    if ((op != N_NEG) && (op != N_NEGPF) && (op != N_COM) && (op != N_NOT)
     && (op != N_CBYTE) && (op != N_CPAL) && (op != N_CSHORT) && (op != N_CUSHORT)
     && (op != N_CDOUBLE) && (op != N_CFLOAT) && (op != N_ENTPF) && (op != N_PFENT)
     && (op != N_INC) && (op != N_PINC) && (op != N_CCHAR))
      return 0;
    nodo = nodo->izq;
  }
}

void carga_reg(int tipo, int reg1, int reg2)
{
  if (tipo == SINT || tipo == UINT) {
    gen_inst1("or", SI, reg1, reg2, 0);
  } else if (tipo == SSHORT) {
    gen_inst1("sll", SI, reg1, reg2, 16);
    gen_inst1("sra", SI, reg1, reg1, 16);
  } else if (tipo == USHORT) {
    gen_inst1("or", SI, reg1, reg2, 0);
    gen_inst3("consth", SI, reg1, 0);
  } else if (tipo == DOUBLE) {
    gen_inst1("or", SI, reg1, reg2, 0);
    gen_inst1("or", SI, reg1 + 1, reg2 + 1, 0);
  } else if (tipo == FLOAT) {
    gen_inst4(reg1, reg2, "0,0,2,1");
  } else if (tipo == SCHAR) {
    gen_inst1("sll", SI, reg1, reg2, 24);
    gen_inst1("sra", SI, reg1, reg1, 24);
  } else
    gen_inst1("and", SI, reg1, reg2, 255);
}

void carga_mem(int tipo, int reg1, int reg2)
{
  if (tipo == SINT || tipo == UINT)
    gen_inst2("load 0,4,", NO, reg1, reg2);
  else if (tipo == SSHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst3("exhws", NO, reg1, reg1);
  } else if (tipo == USHORT) {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst1("exhw", SI, reg1, reg1, 0);
  } else if (tipo == DOUBLE) {
    gen_inst1("add", SI, reg1 + 1, reg2, 4);
    gen_inst2("load 0,4,", NO, reg1 + 1, reg1 + 1);
    gen_inst2("load 0,4,", NO, reg1, reg2);
  } else if (tipo == FLOAT) {
    gen_inst2("load 0,4,", NO, reg1, reg2);
    gen_inst4(reg1, reg1, "0,0,2,1");
  } else if (tipo == SCHAR) {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst1("exbyte", SI, reg1, reg1, 0);
    gen_inst1("sll", SI, reg1, reg1, 24);
    gen_inst1("sra", SI, reg1, reg1, 24);
  } else {
    gen_inst2("load 0,20,", NO, reg1, reg2);
    gen_inst1("exbyte", SI, reg1, reg1, 0);
  }
}

void almacena_mem(int tipo, int reg1, int reg2)
{
  int reg, es_numero = NO;

  if (reg1 > 255) {
    es_numero = SI;
    reg1 -= 256;
  }
  if (tipo == SINT || tipo == UINT)
    gen_inst2("store 0,4,", NO, reg1, reg2);
  else if (tipo == SSHORT || tipo == USHORT) {
    reg = pedir_reg(NO);
    gen_inst2("load 0,20,", NO, reg, reg2);
    gen_inst1("inhw", es_numero, reg, reg, reg1);
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  } else if (tipo == DOUBLE) {
    gen_inst2("store 0,4,", NO, reg1, reg2);
    reg = pedir_reg(NO);
    gen_inst1("add", SI, reg, reg2, 4);
    gen_inst2("store 0,4,", NO, reg1 + 1, reg);
    libera_reg(reg);
  } else if (tipo == FLOAT) {
    reg = pedir_reg(NO);
    gen_inst4(reg, reg1, "0,0,1,2");
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  } else {
    reg = pedir_reg(NO);
    gen_inst2("load 0,20,", NO, reg, reg2);
    gen_inst1("inbyte", es_numero, reg, reg, reg1);
    gen_inst2("store 0,4,", NO, reg, reg2);
    libera_reg(reg);
  }
}

/*
** Copia una estructura para resultado de función.
*/
void copia_resultado(struct nodo **nodo, int tam)
{
  crea_nodo(N_APRES, NULL, NULL, 0);
  crea_nodo(N_COPIA, ultimo_nodo, *nodo, (tam + 3) >> 2);
  *nodo = ultimo_nodo;
}

/*
** Pone el prologo para el codigo generado.
*/
void prologo(void)
{
  emite_linea("; ");
  emite_texto("; ");
  emite_linea(PROGRAMA);
  emite_linea("COMIENZO:");
  emite_linea("const gr96,INICIO");
  emite_linea("consth gr96,INICIO");
  emite_linea("jmpi gr96");
  emite_linea("nop");
  emite_linea(".ascii \"Compilador de ANSI C G11, \"");
  emite_linea(".ascii \"(c) Copyright Oscar Toledo G.1995-1998\"");
  emite_linea(".byte 0");
  emite_linea(".align");
}

/*
** Pone el epilogo para el codigo generado.
*/
void epilogo(void)
{
  emite_linea("; --- Fin de compilación ---");
  emite_linea("INICIO:");
  emite_linea("sub gr1,gr1,32");
  emite_linea("asgeu 64,gr1,gr126");
  emite_linea("add lr1,gr1,40");
  emite_linea("const gr121,__main");
  emite_linea("consth gr121,__main");
  emite_linea("calli lr0,gr121");
  emite_linea("nop");
  emite_linea("cpeq gr97,gr96,0");
  emite_linea("jmpf gr97,FIN");
  emite_linea("nop");
  emite_linea("const gr121,_main");
  emite_linea("consth gr121,_main");
  emite_linea("calli lr0,gr121");
  emite_linea("nop");
  emite_linea("FIN:");
  emite_linea("const gr121,__exit");
  emite_linea("consth gr121,__exit");
  emite_linea("calli lr0,gr121");
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
void emite_nombre(char *nombre)
{
  emite_texto("_");
  emite_texto(nombre);
}

/*
** Llama a la función especificada.
*/
void llamada(struct nombres *ap)
{
  int a;

  if (prog_grande) {
    for (a = 0; a < total_funciones; a++) {
      if (funciones[a].func == ap)
        break;
    }
    emite_texto("call lr0,");
    emite_nombre(ap->nombre);
    emite_texto(",");
    emite_etiq(funciones[a].etiqueta);
    emite_nueva_linea();
  } else {
    emite_texto("call lr0,");
    emite_nombre(ap->nombre);
    emite_nueva_linea();
  }
  gen_libre(1);
}

/*
** Salta a la etiqueta interna especificada.
*/
void salto(int etiq)
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
**
** Las expresiones booleanas generadas son muy buenas, este codigo
** me costo mucho trabajo.
*/
#define es_comparacion(nodo) (nodo->oper >= N_IGUAL && nodo->oper <= N_MAYORIPF)

void salta_expr(struct nodo *expr, int etiq, int forza_not)
{
  int etiq_or, etiq_and, reg, reg1, c;

  if (forza_not && expr->oper == N_NOT) {
    expr = expr->izq;
    forza_not = NO;
  }
  if (!forza_not && expr->oper == N_ANDB) {
    salta_expr(expr->izq, etiq, NO);
    salta_expr(expr->der, etiq, NO);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (!forza_not && expr->oper == N_ORB) {
    salta_expr(expr->izq, etiq_or = ++sig_etiq, SI);
    salta_expr(expr->der, etiq, NO);
    gen_destino(etiq_or);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (forza_not && expr->oper == N_ANDB) {
    salta_expr(expr->izq, etiq_and = ++sig_etiq, NO);
    salta_expr(expr->der, etiq, SI);
    gen_destino(etiq_and);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (forza_not && expr->oper == N_ORB) {
    salta_expr(expr->izq, etiq, SI);
    salta_expr(expr->der, etiq, SI);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (expr->oper == N_NOT && expr->izq->oper == N_ANDB) {
    salta_expr(expr->izq->izq, etiq_and = ++sig_etiq, NO);
    salta_expr(expr->izq->der, etiq, SI);
    gen_destino(etiq_and);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (expr->oper == N_NOT && expr->izq->oper == N_ORB) {
    salta_expr(expr->izq->izq, etiq, SI);
    salta_expr(expr->izq->der, etiq, SI);
    for (c = 0; c < TEMP; c++)
      exp_temp[c] = NULL;
  } else if (es_comparacion(expr)) {
    crea_nodo(expr->oper, expr->izq, expr->der, 1);
    reg = pedir_reg(NO);
    gen_nodo(ultimo_nodo, reg);
    if (forza_not)
      salta_si_verdadero(etiq, reg);
    else
      salta_si_falso(etiq, reg);
    libera_reg(reg);
  } else if (expr->oper == N_NOT && es_comparacion(expr->izq)) {
    crea_nodo(expr->izq->oper, expr->izq->izq, expr->izq->der, 1);
    reg = pedir_reg(NO);
    gen_nodo(ultimo_nodo, reg);
    if (forza_not)
      salta_si_falso(etiq, reg);
    else
      salta_si_verdadero(etiq, reg);
    libera_reg(reg);
  } else if (expr->oper == N_NOT) {
    if (expr->izq->oper == N_CPAL && en_registro(expr->izq->izq))
      gen_inst1("cpneq", SI, 97, registro(expr->izq->izq), 0);
    else if (expr->izq->oper == N_FUNC || expr->izq->oper == N_FUNCI) {
      gen_nodo(expr->izq, 96);
      gen_inst1("cpneq", SI, 97, 96, 0);
    } else {
      reg1 = pedir_reg(NO);
      gen_nodo(expr->izq, reg1);
      gen_inst1("cpneq", SI, 97, reg1, 0);
      libera_reg(reg1);
    }
    if (forza_not)
      salta_si_falso(etiq, 97);
    else
      salta_si_verdadero(etiq, 97);
  } else {
    if (expr->oper == N_CPAL && en_registro(expr->izq))
      gen_inst1("cpneq", SI, 97, registro(expr->izq), 0);
    else if (expr->oper == N_FUNC || expr->oper == N_FUNCI) {
      gen_nodo(expr, 96);
      gen_inst1("cpneq", SI, 97, 96, 0);
    } else {
      reg1 = pedir_reg(NO);
      gen_nodo(expr, reg1);
      gen_inst1("cpneq", SI, 97, reg1, 0);
      libera_reg(reg1);
    }
    if (forza_not)
      salta_si_verdadero(etiq, 97);
    else
      salta_si_falso(etiq, 97);
  }
}

/*
** Prueba el registro y salta si es falso.
*/
void salta_si_falso(int etiq, int reg)
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
void salta_si_verdadero(int etiq, int reg)
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
void emite_etiq(int etiq)
{
  emite_texto("c");
  emite_numero(etiq);
}

void dos_puntos(void)
{
  emite_car(58);
}

/*
** Seudo-operacion para definir un byte.
*/
void def_byte(void)
{
  emite_texto(".byte ");
}

/*
** Seudo-operacion para definir una palabra.
*/
void def_palabra(void)
{
  emite_texto(".word ");
}

/*
** Define espacio
*/
void def_espacio(int val)
{
  emite_texto(".space ");
  emite_numero(val);
  emite_nueva_linea();
}

/*
** Define nombre global
*/
void def_global(char *nombre)
{
  emite_texto(".global ");
  emite_nombre(nombre);
  emite_nueva_linea();
}

/*
** Hace una comparación y un salto. (para switch)
** No pierde el valor con el que esta comparando.
*/
void compara_y_salta(int valor, int etiqueta)
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
void vacia_lits(void)
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

void gen_inst1(char *instruccion, int inmediato, int reg1, int reg2, int valreg)
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

void gen_inst2(char *instruccion, int inmediato, int reg1, int valreg)
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

void gen_inst3(char *instruccion, int inmediato, int reg1, int valreg)
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

void gen_inst4(int reg1, int reg2, char *pars)
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
  emite_texto(",");
  emite_linea(pars);
}

void gen_destino(int etiq)
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
void gen_libre(int tipo)
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

int tipo_reg(int reg)
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

int max(int a, int b)
{
  if (a > b)
    return a;
  else
    return b;
}

void emite_registro(int reg)
{
  if (reg < 128) {
    emite_texto("gr");
    emite_numero(reg);
  } else {
    emite_texto("lr");
    emite_numero(reg - 128);
  }
}

void segmento_cero(void)
{
  emite_linea(".bss");
}

void segmento_codigo(void)
{
  emite_linea(".text");
}
