/*
** Compilador de C para G11.
** Análisis sintáctico de alto nivel.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 03-jun-1995.
** Revisión: 05-sep-1998. Rediseño para apoyar el nuevo analizador léxico.
** Revisión: 12-sep-1998. Rediseño de p_tipo_1 para soportar ANSI C.
** Revisión: 08-oct-1998. Soporte de inicializaciones globales.
** Revisión: 12-oct-1998. Ya detecta case repetido. Ahora los tipos se asignan
**                        dinámicamente. Ya apoya prototipos, const y volatile.
** Revisión: 14-oct-1998. Nuevo estilo en p_tipo_1 para no aceptar clases de
**                        almacenamiento.
** Revisión: 26-oct-1998. Detecta redefinición de local con nombre de parametro.
**                        Correcciones en declaraciones de función K&R.
** Revisión: 06-nov-1998. Corrección en procesamiento de tamaño de arreglo,
**                        podía perder el tipo. Corrección en p_tipo_1, leía
**                        clase_alm en estilo 2.
** Revisión: 11-nov-1998. Corrección de un defecto en procesamiento de static
**                        en locales (¡No podía inicializar!). Apoya el nuevo
**                        segmento .bss
** Revisión: 30-dic-1998. Corrección en p_bloque(), usaba apuntadores nulos al
**                        seguir la lista de sentencias.
** Revisión: 27-ene-1999. Corrección en la inicialización de arreglos y
**                        estructuras locales.
*/

/*
** Efectua el proceso de compilación, inicia el análisis sintáctico
** descendente recursivo. (es el más sencillo, se sigue la sintaxis del
** lenguaje y se crea una función por cada nodo)
**
** En este nivel se aceptan declaraciones o definiciones de función
*/
void analiza(void)
{
  while (eof == 0) {    /* Trabaja hasta que no haya más entrada */
    if (p_tipo_1(1)) {  /* Siempre es cierto */
      switch (clase_alm) {
        case TYPEDEF:
          decl_typedef(NO);
          break;
        case STATIC:
          decl_glb(STATIC);
          break;
        case EXTERN:
          decl_glb(EXTERN);
          break;
        case REGISTER:
        case AUTO:
          error("No se acepta register ni auto");
        default:
          decl_glb(AUTO);
          break;
      }
    }
  }
}

/*
** Declara un tipo.
*/
void decl_typedef(int local)
{
  struct nombres *chequeo;

  while (1) {
    if (fin_sentencia())
      break;
    if (p_tipo_2(1)) {
      if (clave_lex == C_PAREND)
        obt_lex();
      else
        error("Falta parentesis derecho");
    }
    if (t_primero->tipo == FUNCION)
      error("No se puede definir un tipo de función");
    if (local)
      chequeo = busca_loc(nombre_tipo);
    else
      chequeo = busca_glb(nombre_tipo);
    if (chequeo != NULL)
      redefinido(nombre_tipo);
    if (local)
      nueva_loc(nombre_tipo, TYPEDEF, AUTO, t_primero, 0);
    else
      nueva_glb(nombre_tipo, TYPEDEF, STATIC, t_primero, 0);
    if (clave_lex != C_COMA)
      break;
    obt_lex();
  }
  punto_y_coma();
}

/*
** Declara una variable global.
**
** Crea una entrada en la tabla, para que las
** referencias subsiguientes la llamen por nombre.
*/
void decl_glb(int clase)
{
  char nombre[TAM_NOMBRE];
  int p;
  struct nombres *chequeo;

  while (1) {
    if (fin_sentencia())
      break;
    p = p_tipo_2(1);
    strcpy(nombre, nombre_tipo);
    if (t_primero->tipo == FUNCION) {
      if (t_primero->sig == t_float)
        t_primero->sig = t_double;
      if (p == 0 && (clave_lex == C_PCOMA || clave_lex == C_COMA)) {
        if ((chequeo = busca_glb(nombre)) != NULL) {
          if (chequeo->ident != FUNCION)
            redefinido(nombre);
          else if (chequeo->posicion != FUNC_REF)
            redefinido(nombre);
          else {
            chequeo->posicion = FUNC_TIPO;
            chequeo->tipo = t_primero;
          }
        } else
          nueva_glb(nombre, FUNCION, STATIC, t_primero, FUNC_TIPO);
      } else {
        if (clase == EXTERN)
          error("Definiendo una función extern");
        if (clase != STATIC)
          def_global(nombre);
        nueva_func(t_primero, nombre, p);
        return;
      }
    } else {
      if (busca_glb(nombre) != NULL) {  /* ¿ Ya estaba en la tabla ? */
        if (clase != EXTERN)
          redefinido(nombre);
      } else {                  /* Agrega la nueva variable */
        if (clase == STATIC)
          nueva_glb(nombre, VARIABLE, STATIC, t_primero, p = ++sig_etiq);
        else
          nueva_glb(nombre, VARIABLE, STATIC, t_primero, p = 0);
        if (clase != EXTERN) {
          if (clave_lex != C_IGUAL)
            segmento_cero();
          if (p)
            emite_etiq(p);
          else {
            def_global(nombre);
            emite_nombre(nombre);
          }
          dos_puntos();
          emite_nueva_linea();
          if (clave_lex == C_IGUAL) {
            obt_lex();
            inicializa(t_primero);
          } else {
            def_espacio((tam_tipo(t_primero) + 3) & ~3);
            segmento_codigo();
          }
        }
      }
    }
    if (clave_lex != C_COMA)
      break;
    obt_lex();
  }
  punto_y_coma();
}

/*
** Procesa declaraciones de variables locales.
*/
struct sentencia *decl_loc(void)
{
  char nombre[TAM_NOMBRE];
  struct tipo *salva_tipo_basico, *salva_t_primero, *tipo_expr, *tipo_temp;
  int p, estaticas;
  struct nodo *nodo_expr;
  struct nombres *chequeo, *ap;
  struct sentencia *agregado, *temp, *lista = NULL;

  while (1) {
    if (p_tipo_1(0) == 0)
      break;
    if (clase_alm == TYPEDEF) {
      decl_typedef(SI);
      continue;
    }
    if (clase_alm == STATIC)
      estaticas = SI;
    else
      estaticas = NO;
    if (dentro_switch)
      error("No se pueden efectuar declaraciones dentro de un switch");
    while (1) {
      if (fin_sentencia())
        break;
      p = p_tipo_2(1);
      strcpy(nombre, nombre_tipo);
      if (((chequeo = busca_loc(nombre)) != NULL)
        && (nivel == chequeo->nivel || (nivel == 1 && chequeo->nivel == 0)))
        redefinido(nombre);
      if (t_primero->tipo == FUNCION) {
        if (p) {
          if (clave_lex != C_PAREND)
            error("Falta parentesis derecho");
          else
            obt_lex();
        }
        nueva_loc(nombre, FUNCION, AUTO, t_primero, FUNC_TIPO);
      } else if (estaticas) {
        nueva_loc(nombre, VARIABLE, STATIC, t_primero, p = ++sig_etiq);
        if (clave_lex != C_IGUAL)
          segmento_cero();
        emite_etiq(p);
        dos_puntos();
        emite_nueva_linea();
        if (clave_lex == C_IGUAL) {
          obt_lex();
          inicializa(t_primero);
        } else {
          def_espacio((tam_tipo(t_primero) + 3) & ~3);
          segmento_codigo();
        }
      } else {
        /*
        ** Las estructuras y las matrices jamás caben en un registro.
        **
        ** Las variables normales las dejamos al libre albedrío del
        ** compilador.
        */
        if (t_primero->tipo != STRUCT && t_primero->tipo != ARREGLO) {
          ap = nueva_loc(nombre, VARIABLE, AUTO, t_primero,
                         var_virtual(0, t_primero->tipo == DOUBLE));
        } else
          ap = nueva_loc(nombre, VARIABLE, AUTO, t_primero,
                 var_virtual((((tam_tipo(t_primero) + 3) & ~3) << 2) | 1, 0));
        if (t_primero->tipo != STRUCT && t_primero->tipo != ARREGLO
         && clave_lex == C_IGUAL) {
          obt_lex();
          salva_tipo_basico = tipo_basico;
          salva_t_primero = t_primero;
          tipo_expr = almacena_expresion(NO);
          nodo_expr = ultimo_nodo;
          t_primero = salva_t_primero;
          tipo_basico = salva_tipo_basico;
          if (tipo_expr->tipo == FUNCION) {
            tipo_temp = crea_tipo(APUNTADOR);
            tipo_temp->sig = tipo_expr;
            tipo_expr = tipo_temp;
          }
          convierte_tipo(&nodo_expr, tipo_expr, t_primero, SI);
          crea_nodo(N_DIR, NULL, NULL, ap->posicion);
          crea_nodo(N_ASIGNA, nodo_expr, ultimo_nodo, ap->tipo->tipo);
          agregado = nueva_sentencia(t_expresion);
          agregado->def.t_expresion.expresion = ultimo_nodo;
          if (lista == NULL)
            lista = agregado;
          else
            temp->sig = agregado;
          temp = agregado;
        } else if (clave_lex == C_IGUAL) {
          obt_lex();
          salva_tipo_basico = tipo_basico;
          salva_t_primero = t_primero;
          p = ++sig_etiq;
          emite_etiq(p);
          dos_puntos();
          emite_nueva_linea();
          inicializa(t_primero);
          t_primero = salva_t_primero;
          tipo_basico = salva_tipo_basico;
          crea_nodo(N_DIRE, NULL, NULL, p);
          nodo_expr = ultimo_nodo;
          crea_nodo(N_DIR, NULL, NULL, ap->posicion);
          crea_nodo(N_COPIA, ultimo_nodo, nodo_expr, (tam_tipo(t_primero) + 3) / 4);
          agregado = nueva_sentencia(t_expresion);
          agregado->def.t_expresion.expresion = ultimo_nodo;
          if (lista == NULL)
            lista = agregado;
          else
            temp->sig = agregado;
          temp = agregado;
        }
      }
      if (clave_lex != C_COMA)
        break;
      obt_lex();
    }
    punto_y_coma();
  }
  return lista;
}

/*
** Procesa la primera parte de un tipo, la variable tipo_basico
** contiene el tipo base del tipo completo.
**
** Si estilo es 0, acepta cualquier tipo explicito.
** Si estilo es 1, acepta aunque no haya tipo. (toma como int).
** Si estilo es 2, no acepta especificadores de almacenamiento pero
**                 requiere tipo.
*/
int p_tipo_1(int estilo)
{
  int tipo_base = 0, tipo = 0, signo = 0;
  int corto = 0, largo = 0, con = NO, vol = NO;
  struct nombres *chequeo;

  if (estilo != 2)
    clase_alm = 0;
  while (1) {
    if (estilo != 2) {
      switch (clave_lex) {
        case C_TYPEDEF:
          if (clase_alm)
            almacena_conf();
          clase_alm = TYPEDEF;
          obt_lex();
          continue;
        case C_REGISTER:
          if (clase_alm)
            almacena_conf();
          clase_alm = REGISTER;
          obt_lex();
          continue;
        case C_STATIC:
          if (clase_alm)
            almacena_conf();
          clase_alm = STATIC;
          obt_lex();
          continue;
        case C_EXTERN:
          if (clase_alm)
            almacena_conf();
          clase_alm = EXTERN;
          obt_lex();
          continue;
        case C_AUTO:
          if (clase_alm)
            almacena_conf();
          clase_alm = AUTO;
          obt_lex();
          continue;
      }
    }
    switch (clave_lex) {
      case C_CONST:
        if (con)
          califica_conf();
        con = SI;
        obt_lex();
        continue;
      case C_VOLATILE:
        if (vol)
          califica_conf();
        vol = SI;
        obt_lex();
        continue;
      case C_VOID:
        tipo_basico = t_void;
        if (tipo || corto || largo || signo || con || vol)
          tipo_conf();
        obt_lex();
        return 1;
      case C_CHAR:
        tipo_basico = t_schar;
        if (tipo || corto || largo)
          tipo_conf();
        tipo = 1;
        obt_lex();
        continue;
      case C_INT:
        tipo_basico = t_sint;
        if (tipo)
          tipo_conf();
        tipo = 1;
        obt_lex();
        continue;
      case C_SHORT:
        if (largo || corto)
          tipo_conf();
        corto = 1;
        obt_lex();
        continue;
      case C_LONG:
        if (largo || corto)
          tipo_conf();
        largo = 1;
        obt_lex();
        continue;
      case C_SIGNED:
        if (signo)
          tipo_conf();
        signo = 1;
        obt_lex();
        continue;
      case C_UNSIGNED:
        if (signo)
          tipo_conf();
        signo = 2;
        obt_lex();
        continue;
      case C_FLOAT:
        tipo_basico = t_float;
        if (tipo)
          tipo_conf();
        tipo = 1;
        obt_lex();
        continue;
      case C_DOUBLE:
        tipo_basico = t_double;
        if (tipo)
          tipo_conf();
        tipo = 1;
        obt_lex();
        continue;
      case C_IDENT:
        if (((chequeo = busca_loc(cad_lex)) != NULL)
         || ((chequeo = busca_glb(cad_lex)) != NULL)) {
          if (chequeo->ident == TYPEDEF) {
            if (tipo || corto || largo || signo)
              tipo_conf();
            obt_lex();
            tipo_basico = chequeo->tipo;
            return 1;
          }
        }
      default:
        if (tipo == 0 && (corto || largo || signo || estilo == 1)) {
          tipo_basico = t_sint;
          tipo = 1;
        }
        if (tipo == 0 && !vol && !con && (estilo == 2 || clase_alm == 0))
          return 0;
        if (tipo_basico == t_schar) {
          if (corto || largo)
            tipo_conf();
          tipo_basico = (signo == 1) ? t_schar : t_uchar;
        } else if (tipo_basico == t_float || tipo_basico == t_double) {
          if (corto || signo || (largo && tipo_basico == t_float))
            tipo_conf();
        } else {
          if (corto)
            tipo_basico = (signo == 2) ? t_ushort : t_sshort;
          else if (largo)
            tipo_basico = (signo == 2) ? t_uint : t_sint;
          else
            tipo_basico = (signo == 2) ? t_uint : t_sint;
        }
        califica(con, vol);
        return 1;
      case C_STRUCT:
        obt_lex();
        p_estructura(NO);
        califica(con, vol);
        return 1;
      case C_UNION:
        obt_lex();
        p_estructura(SI);
        califica(con, vol);
        return 1;
      case C_ENUM:
        obt_lex();
        p_enumerador();
        califica(con, vol);
        return 1;
    }
  }
}

/*
** Califica con const o volatile un tipo básico
*/
void califica(int con, int vol)
{
  struct tipo *temp, *nuevo_tipo = NULL;

  if (con + vol) {
    if (con) {
      nuevo_tipo = crea_tipo(CONST);
      nuevo_tipo->sig = tipo_basico;
    }
    if (vol) {
      temp = crea_tipo(VOLATILE);
      if (nuevo_tipo == NULL) {
        nuevo_tipo = temp;
        nuevo_tipo->sig = tipo_basico;
      } else {
        nuevo_tipo->sig = temp;
        temp->sig = tipo_basico;
      }
    }
    tipo_basico = nuevo_tipo;
  }
}

void tipo_conf(void)
{
  error("Tipo inválido");
}

void almacena_conf(void)
{
  error("Almacenamiento inválido en declaración");
}

void califica_conf(void)
{
  error("Const o volatile incorrectos");
}

/*
** Genera un tipo procesado.
*/
int p_tipo_2(int nombre)
{
  int func;

  *nombre_tipo = 0;
  t_primero = t_ultimo = NULL;
  func = p_tipo_3(nombre);
  if (t_ultimo == NULL)
    t_primero = t_ultimo = tipo_basico;
  else
    t_ultimo->sig = tipo_basico;
  return func;
}

/*
** Procesa las decoraciones de tipo.
*/
int p_tipo_3(int nombre)
{
  int p, tam, con, vol;

  if (clave_lex == C_MUL) {
    obt_lex();
    con = 0;
    vol = 0;
    while (1) {
      if (clave_lex == C_CONST) {
        if (con)
          califica_conf();
        con = 1;
        obt_lex();
      } else if (clave_lex == C_VOLATILE) {
        if (vol)
          califica_conf();
        vol = 1;
        obt_lex();
      } else
        break;
    }
    p = p_tipo_3(nombre);
    if (con)
      encadena(CONST);
    if (vol)
      encadena(VOLATILE);
    encadena(APUNTADOR);
    return p;
  }
  if (clave_lex == C_PARENI) {
    obt_lex();
    p = p_tipo_3(nombre);
    if (clave_lex != C_PAREND)
      error("Falta parentesis derecho");
    else
      obt_lex();
  } else if (nombre) {
    if (clave_lex != C_IDENT) {
      if (nombre != 2)
        error("No es un nombre legal");
    } else {
      strcpy(nombre_tipo, cad_lex);
      obt_lex();
    }
  }
  if (clave_lex == C_PARENI) {
    obt_lex();
    encadena(FUNCION);
    if (clave_lex == C_PAREND) {
      obt_lex();
      return 0;
    } else if (prototipo()) {
      return 0;
    } else
      return 1;
  }
  while (clave_lex == C_CORCHI) {
    obt_lex();
    encadena(ARREGLO);
    t_ultimo->especial.tam = tam_arreglo();
  }
  return 0;
}

/*
** Busca un posible prototipo ANSI
*/
int prototipo(void)
{
  char salva_nombre[TAM_NOMBRE];
  struct tipo *s_t_primero, *s_t_ultimo, *s_tipo_basico, *agregado;
  int es_prototipo, num_pars, pars_var, s_clase_alm;
  struct proto *lista, *ultimo, *nuevo;

  strcpy(salva_nombre, nombre_tipo);
  s_t_primero = t_primero;
  s_t_ultimo = t_ultimo;
  s_tipo_basico = tipo_basico;
  s_clase_alm = clase_alm;
  es_prototipo = NO;
  num_pars = 0;
  pars_var = NO;
  lista = ultimo = NULL;
  if (p_tipo_1(0)) {
    es_prototipo = SI;
    if (clase_alm != 0 || tipo_basico != t_void || clave_lex != C_PAREND) {
      while (1) {
        if (clase_alm != 0 && clase_alm != REGISTER)
          error("Clase de almacenamiento incorrecta");
        ++num_pars;
        if (p_tipo_2(2)) {
          if (clave_lex == C_PAREND)
            obt_lex();
          else
            error("Falta parentesis derecho");
        }
        nuevo = pide_espacio(&primer_tipo, &ultimo_tipo,
                             sizeof(struct proto) + strlen(nombre_tipo));
        strcpy(nuevo->nombre, nombre_tipo);
        if (t_primero->tipo == ARREGLO) {
          agregado = crea_tipo(APUNTADOR);
          agregado->sig = t_primero->sig;
          t_primero = agregado;
        } else if (t_primero->tipo == FUNCION) {
          agregado = crea_tipo(APUNTADOR);
          agregado->sig = t_primero;
          t_primero = agregado;
        }
        if (t_primero == t_float)
          t_primero = t_double;
        nuevo->tipo = t_primero;
        nuevo->sig = NULL;
        if (lista == NULL)
          lista = nuevo;
        if (ultimo != NULL)
          ultimo->sig = nuevo;
        ultimo = nuevo;
        if (clave_lex == C_COMA)
          obt_lex();
        else if (clave_lex != C_PUNTOS)
          break;
        if (clave_lex == C_PUNTOS) {
          obt_lex();
          pars_var = SI;
          break;
        }
        if (p_tipo_1(0) == 0) {
          error("Parametro vacio");
          break;
        }
      }
    }
    if (clave_lex == C_PAREND)
      obt_lex();
    else
      error("Falta parentesis derecho");
    s_t_ultimo->especial.proto = lista;
    s_t_ultimo->num_pars = pars_var ? num_pars + 64 : num_pars;
  }
  clase_alm = s_clase_alm;
  tipo_basico = s_tipo_basico;
  t_ultimo = s_t_ultimo;
  t_primero = s_t_primero;
  strcpy(nombre_tipo, salva_nombre);
  return es_prototipo;
}

/*
** Encadena otro tipo
*/
void encadena(int tipo)
{
  struct tipo *nuevo = crea_tipo(tipo);

  if (t_primero == NULL)
    t_primero = nuevo;
  if (t_ultimo != NULL)
    t_ultimo->sig = nuevo;
  t_ultimo = nuevo;
}

/*
** Obtiene el tamaño de un arreglo.
*/
int tam_arreglo(void)
{
  char salva_nombre[TAM_NOMBRE];
  struct tipo *s_t_primero, *s_t_ultimo, *s_tipo_basico;
  int num;

  strcpy(salva_nombre, nombre_tipo);
  s_t_primero = t_primero;
  s_t_ultimo = t_ultimo;
  s_tipo_basico = tipo_basico;
  if (clave_lex == C_CORCHD)    /* Tamaño nulo */
    num = 0;
  else {
    num = expr_constante();     /* Procesa una expresión constante */
    if (num == 0) {
      error("No se acepta una dimensión cero");
      num = 1;                  /* Forza a 1 */
    }
    if (num < 0) {
      error("Tamaño negativo");
      num = -num;
    }
  }
  if (clave_lex == C_CORCHD)
    obt_lex();
  else
    error("Falta corchete derecho");
  tipo_basico = s_tipo_basico;
  t_ultimo = s_t_ultimo;
  t_primero = s_t_primero;
  strcpy(nombre_tipo, salva_nombre);
  return num;                   /* Retorna el tamaño */
}

/*
** Procesa una declaración de estructura o unión.
*/
/* !!! Apoyar miembros de bits reales */
void p_estructura(int es_union)
{
  char nombre_miembro[TAM_NOMBRE];
  struct miembro *miembro, *lista;
  struct rotulo *estructura;
  int posicion, tam, numero_bits;
  struct tipo *tipo_optimo;

  if (clave_lex == C_IDENT) {
    if ((estructura = busca_estructura(cad_lex)) != NULL) {
      if (estructura->que_es == ENUM)
        redefinido(cad_lex);
      if (es_union != estructura->es_union)
        if (es_union)
          error("Se uso union en lugar de struct");
        else
          error("Se uso struct en lugar de union");
      obt_lex();
      if (estructura->tam) {
        tipo_basico = crea_tipo(STRUCT);
        tipo_basico->especial.est = estructura;
        return;
      }
    } else {
      estructura = nueva_estructura(cad_lex);
      estructura->que_es = STRUCT;
      estructura->es_union = es_union;
      obt_lex();
    }
  } else {
    estructura = nueva_estructura("");
    estructura->que_es = STRUCT;
    estructura->es_union = es_union;
  }
  if (clave_lex != C_LLAVEI) {
    tipo_basico = crea_tipo(STRUCT);
    tipo_basico->especial.est = estructura;
    return;
  }
  obt_lex();
  lista = NULL;
  posicion = tam = 0;
  while (p_tipo_1(2)) {
    while (1) {
      if (es_union)
        posicion = 0;
      if (fin_sentencia())
        break;
      numero_bits = 0;
      if (clave_lex == C_DPUNTOS) {
        obt_lex();
        *nombre_miembro = 0;
        numero_bits = expr_constante();
        if (tipo_basico != t_sint && tipo_basico != t_uint)
          error("No es de tipo int o unsigned int");
        if (numero_bits > 32)
          error("Más de 32 bits en el miembro");
        if (numero_bits == 0)
          posicion = ((posicion + 3) & ~3) + 4;
        else if (numero_bits > 16)
          posicion += 4;
        else if (numero_bits > 8)
          posicion += 2;
        else
          posicion++;
      } else {
        if (p_tipo_2(1)) {
          if (clave_lex == C_PAREND)
            obt_lex();
          else
            error("Falta parentesis derecho");
        }
        strcpy(nombre_miembro, nombre_tipo);
        if (clave_lex == C_DPUNTOS) {
          obt_lex();
          numero_bits = expr_constante();
          if (t_primero != t_sint && t_primero != t_uint)
            error("No es de tipo int o unsigned int");
          if (numero_bits == 0)
            error("Miembro vacio");
          if (numero_bits > 32)
            error("Más de 32 bits en el miembro");
          if (numero_bits > 16) {
            posicion = (posicion + 3) & ~3;
          } else if (numero_bits > 8) {
            posicion = (posicion + 1) & ~1;
            if (t_primero == t_sint)
              t_primero = t_sshort;
            else
              t_primero = t_ushort;
          } else {
            t_primero = t_uchar;
          }
          numero_bits = 0;
        } else {
          tipo_optimo = t_primero;
          while (tipo_optimo->tipo == ARREGLO)
            tipo_optimo = tipo_optimo->sig;
          if (tipo_optimo == t_sshort || tipo_optimo == t_ushort)
            posicion = (posicion + 1) & ~1;
          else if (tipo_optimo != t_schar && tipo_optimo != t_uchar)
            posicion = (posicion + 3) & ~3;
        }
      }
      if (*nombre_miembro) {
        if ((miembro = busca_miembro(lista, nombre_miembro)) != NULL)
          redefinido(nombre_miembro);
        else
          miembro = nuevo_miembro(&lista, nombre_miembro);
        miembro->tipo = t_primero;
        miembro->posicion = posicion;
        posicion += tam_tipo(t_primero);
      }
      if (es_union)
        tam = (posicion > tam) ? posicion : tam;
      else
        tam = posicion;
      if (clave_lex != C_COMA)
        break;
      obt_lex();
    }
    punto_y_coma();
  }
  if (lista != NULL)
    estructura->lista = lista;
  estructura->tam = tam;
  if (tam == 0)
    if (es_union)
      error("Unión vacia");
    else
      error("Estructura vacia");
  tipo_basico = crea_tipo(STRUCT);
  tipo_basico->especial.est = estructura;
  if (clave_lex == C_LLAVED)
    obt_lex();
  else
    error("Falta llave derecha");
}

/*
** Procesa un enumerador.
*/
void p_enumerador(void)
{
  unsigned char nombre_miembro[TAM_NOMBRE];
  struct rotulo *enumerador;
  int valor;

  if (clave_lex == C_IDENT) {
    if ((enumerador = busca_estructura(cad_lex)) != NULL) {
      if (enumerador->que_es != ENUM)
        redefinido(cad_lex);
      obt_lex();
      if (enumerador->tam) {
        tipo_basico = t_sint;
        return;
      }
    } else {
      enumerador = nueva_estructura(cad_lex);
      enumerador->que_es = ENUM;
      obt_lex();
    }
  } else {
    enumerador = nueva_estructura("");
    enumerador->que_es = ENUM;
  }
  if (clave_lex != C_LLAVEI) {
    tipo_basico = t_sint;
    return;
  }
  obt_lex();
  enumerador->tam = 1;
  valor = 0;
  while (1) {
    if (clave_lex != C_IDENT)
      break;
    strcpy(nombre_miembro, cad_lex);
    obt_lex();
    if (clave_lex == C_IGUAL) {
      obt_lex();
      valor = expr_constante();
    }
    if (busca_enum(nombre_miembro) != NULL)
      redefinido(nombre_miembro);
    else
      nuevo_enum(nombre_miembro, valor++);
    if (clave_lex != C_COMA)
      break;
    obt_lex();
  }
  if (clave_lex == C_LLAVED)
    obt_lex();
  else
    error("Falta llave derecha");
  tipo_basico = t_sint;
}

/*
** Obtiene el tamaño de un tipo.
*/
int tam_tipo(struct tipo *tipo)
{
  int tam;

  switch (tipo->tipo) {
    case SCHAR:
    case UCHAR:
      return 1;
    case SSHORT:
    case USHORT:
      return 2;
    case APUNTADOR:
    case SINT:
    case UINT:
    case FLOAT:
      return 4;
    case DOUBLE:
      return 8;
    case VOID:
      error("Uso incorrecto de void");
      return 0;
    case FUNCION:
      error("Uso incorrecto de tipo de función");
      return 0;
    case ARREGLO:
      if (tipo->especial.tam == 0)
        error("Tamaño nulo de arreglo");
      return tam_tipo(tipo->sig) * tipo->especial.tam;
    case STRUCT:
      tam = tipo->especial.est->tam;
      if (tam == 0)
        error("Estructura o unión incompleta");
      return (tam + 3) & ~3;
  }
}

/*
** Inicializa objetos globales.
*/
void inicializa(struct tipo *tipo)
{
  if (nivel == 0) {
    etiq_lit = ++sig_etiq;
    ap_lit = 0;
  }
  inic(tipo, NO);
  emite_linea(".align");
  if (nivel == 0 && ap_lit)
    vacia_lits();
}

/*
** Procesa inicializadores globales.
*/
int inic(struct tipo *tipo, int dentro)
{
  struct tipo *tipo2, *tiposig;
  int tamreal, total, recorte, cuantos = 0, paso = NO, conteo, tipos;
  int valor;
  struct rotulo *est;
  struct miembro *fsig;
  struct nombres *busca;
  int j, k;

  if (tipo->tipo == ARREGLO) {               /* Inicialización de arreglo */
    total = tamreal = tipo->especial.tam;    /* Tamaño del arreglo */
    tiposig = tipo->sig;
  } else if (tipo->tipo == APUNTADOR) {      /* Inicialización de apuntador */
    total = 1;
    tiposig = tipo->sig;
    while (tiposig->tipo == APUNTADOR)
      tiposig = tiposig->sig;
  } else if (tipo->tipo == STRUCT) {         /* Inicialización de estructura */
    est = tipo->especial.est;                /* Obtiene tamaño de estructura */
    total = est->tam;
    if (total == 0)
      error("Estructura incompleta");
    tamreal = total;
    tiposig = t_schar;                       /* Rellena con bytes */
  } else {                                   /* Inicialización simple */
    total = 1;                               /* Solo se requiere un elemento */
    tiposig = tipo;
  }
  if (clave_lex == C_LLAVEI                  /* Lista de inicializadores */
   || (tipo->tipo == STRUCT && dentro)) {
    if (clave_lex == C_LLAVEI) {
      obt_lex();
      paso = SI;
    }
    if (tipo->tipo == ARREGLO && tiposig->tipo == ARREGLO && clave_lex != C_LLAVEI
     && clave_lex != C_CAD) {                /* Si es un arreglo multidim. */
      while (tiposig->tipo == ARREGLO) {     /* pero el inicializador no esta */
        total *= tiposig->especial.tam;
        tiposig = tiposig->sig;              /* así, tratar como unidimensional */
      }
      tamreal = total;                       /* Tamaño total */
      if (total == 0)
        error("Falta una dimensión en arreglo");
    }
    if (tipo->tipo != STRUCT) {              /* Inicialización normal */
      while (1) {
        if (clave_lex == C_LLAVED) {
          obt_lex();
          break;
        }
        inic(tiposig, SI);
        ++cuantos;
        if (clave_lex != C_COMA) {
          if (clave_lex == C_LLAVED)
            obt_lex();
          else
            error("Falta llave derecha");
          break;
        }
        obt_lex();
      }
    } else {                                 /* Inicialización de estructura */
      fsig = est->lista;                     /* Inicio de estructura */
      while (1) {
        if (clave_lex == C_LLAVED) {
          if (paso)
            obt_lex();
          break;
        }
        inic(fsig->tipo, SI);                /* Inicializa un miembro */
        cuantos = tam_tipo(fsig->tipo);
        if (est->es_union)
          fsig = NULL;
        else
          fsig = fsig->sig;
        if (!est->es_union) {
          if (fsig == NULL)
            cuantos = tamreal;
          else
            cuantos = fsig->posicion;
        }
        if (clave_lex != C_COMA              /* Verifica final */
        || fsig == NULL) {
          if (paso && clave_lex == C_COMA)
            obt_lex();
          if (paso) {
            if (clave_lex == C_LLAVED)
              obt_lex();
            else
              error("Falta llave derecha");
          }
          break;
        }
        obt_lex();
        if (paso && clave_lex == C_LLAVED) {
          obt_lex();
          break;
        }
      }
      tiposig = t_schar;                     /* Rellena con bytes */
    }
  } else if (tiposig->tipo == APUNTADOR) {   /* No es lista, es elemento */
    cuantos = inic(tiposig, dentro);
  } else {
    if (tipo->tipo == ARREGLO
     && tiposig->tipo == ARREGLO) {          /* Trata todos los arreglos */
      while (tiposig->tipo == ARREGLO) {     /* como unidimensionales */
        total *= tiposig->especial.tam;
        tiposig = tiposig->sig;
      }
      tamreal = total;
      if (total == 0)
        error("Falta una dimensión en arreglo");
    }
    if (clave_lex == C_CAD) {                /* Inicializador de cadena */
      cuantos = 1;
      if (tipo->tipo == APUNTADOR) {         /* Si es apuntador, genera ref. */
        emite_linea(".align");
        def_palabra();
        emite_etiq(etiq_lit);
        emite_texto("+");
        emite_numero(ap_lit);
        emite_nueva_linea();
        ap_lit += valor_lex;
        if ((tiposig->tipo != SCHAR && tiposig->tipo != UCHAR && tipo_lex != t_awchar)
         || (tiposig->tipo != USHORT && tipo_lex == t_awchar))
          error("No es apuntador a caracteres");
      } else if (tipo->tipo == ARREGLO) {
        if ((tiposig->tipo != SCHAR && tiposig->tipo != UCHAR && tipo_lex != t_awchar)
         || (tiposig->tipo != USHORT && tipo_lex == t_awchar))
          error("No es apuntador a caracteres");
        cuantos = valor_lex;
        if (total && cuantos > total)        /* Recorta al tamaño máximo */
          cuantos = total;
        k = ap_lit;
        while ((k - ap_lit) < cuantos) {
          def_byte();
          j = 8;
          while (1) {
            emite_numero(lits[k++]);
            if ((k - ap_lit) >= cuantos)
              break;
            if (--j == 0)
              break;
            emite_texto(",");
          }
          emite_nueva_linea();
        }
      } else
        error("La cadena solo puede ser asignada a arreglo o apuntador");
      obt_lex();
    } else if (clave_lex == C_IDENT || clave_lex == C_AND) {
      if (clave_lex == C_AND) {
        obt_lex();
        if (clave_lex != C_IDENT)
          error("Falta el nombre");
      }
      busca = busca_glb(cad_lex);
      if (busca == NULL) {
        busca = nueva_glb(cad_lex, FUNCION, STATIC, t_func, FUNC_REF);
        if (tiposig->tipo != FUNCION)
          error("Inicialización incorrecta");
      }
      if (tipo->tipo == APUNTADOR) {
        emite_linea(".align");
        def_palabra();
        if (busca->posicion)
          emite_etiq(busca->posicion);
        else
          emite_nombre(busca->nombre);
        emite_nueva_linea();
      } else
        error("Inicialización incorrecta");
      obt_lex();
      cuantos = 1;
    } else if (clave_lex == C_NUMF) {
      if (tipo->tipo == STRUCT)
        error("Se requiere llave para inicializar estructura");
      else {
        constan.valor_2[0] = *((int *) (lits + ap_lit));
        constan.valor_2[1] = *((int *) (lits + ap_lit + 4));
        if (tiposig->tipo == SCHAR || tiposig->tipo == UCHAR) {
          emite_texto(".byte ");
          emite_numero((int) constan.valor);
          emite_nueva_linea();
        } else if (tiposig->tipo == SSHORT || tiposig->tipo == USHORT) {
          emite_linea(".align 2");
          emite_texto(".hword ");
          emite_numero((int) constan.valor);
          emite_nueva_linea();
        } else if (tiposig->tipo == FLOAT) {
          constan.valor_1 = constan.valor;
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero(constan.valor_2[0]);
          emite_nueva_linea();
        } else if (tiposig->tipo == DOUBLE) {
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero(constan.valor_2[0]);
          emite_texto(",");
          emite_numero(constan.valor_2[1]);
          emite_nueva_linea();
        } else {
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero((int) constan.valor);
          emite_nueva_linea();
        }
      }
      obt_lex();
    } else {
      if (tipo->tipo == STRUCT)
        error("Se requiere llave para inicializar estructura");
      else {
        j = expr_constante();
        if (tiposig->tipo == SCHAR || tiposig->tipo == UCHAR) {
          emite_texto(".byte ");
          emite_numero(j);
          emite_nueva_linea();
        } else if (tiposig->tipo == SSHORT || tiposig->tipo == USHORT) {
          emite_linea(".align 2");
          emite_texto(".hword ");
          emite_numero(j);
          emite_nueva_linea();
        } else if (tiposig->tipo == FLOAT) {
          constan.valor_1 = j;
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero(constan.valor_2[0]);
          emite_nueva_linea();
        } else if (tiposig->tipo == DOUBLE) {
          constan.valor = j;
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero(constan.valor_2[0]);
          emite_texto(",");
          emite_numero(constan.valor_2[1]);
          emite_nueva_linea();
        } else {
          emite_linea(".align");
          emite_texto(".word ");
          emite_numero(j);
          emite_nueva_linea();
        }
      }
      cuantos = 1;
    }
  }
  if (total && cuantos > total)
    error("Demasiados inicializadores");
  if (tipo->tipo == ARREGLO || tipo->tipo == STRUCT) {
    if (tamreal == 0) {
      if (tipo->tipo == ARREGLO)
        tipo->especial.tam = cuantos;
      else
        cuantos = 1;
    } else if (cuantos < tamreal && tipo->tipo == ARREGLO) {
      def_espacio(tam_tipo(tiposig) * (tamreal - cuantos));
      emite_linea(".align");
      cuantos = tamreal;
    } else if (cuantos < tamreal && tipo->tipo == STRUCT) {
      def_espacio(tamreal - cuantos);
      emite_linea(".align");
      cuantos = 1;
    }
  }
  return cuantos;
}

/*
** Compila una función.
*/
void nueva_func(struct tipo *tipo, char *n_func, int parentesis)
{
  struct proto *cadena;
  struct nombres *local;

  nivel = 0;
  comienzo_funcion = linea_actual; /* Anota el comienzo de la función */
  dentro_funcion = SI;             /* Indica que esta dentro de una función */
  if (tipo->sig->tipo == ARREGLO)
    error("No se puede retornar un arreglo como resultado");
  if ((funcion_actual = busca_glb(n_func)) != NULL) { /* ¿Ya definida? */
    if (funcion_actual->ident != FUNCION)
      redefinido(n_func);          /* Ya hay una variable con ese nombre */
    else if (funcion_actual->posicion == FUNC_DEF)
      redefinido(n_func);          /* Se redefinio una función. */
    else {                         /* Es una función referenciada antes */
      /* !!! Checar los prototipos */
      funcion_actual->posicion = FUNC_DEF;
      funcion_actual->tipo = tipo;
    }
  } else /* No estaba en la tabla, definir cómo una función */
    funcion_actual = nueva_glb(n_func, FUNCION, STATIC, tipo, FUNC_DEF);
  hacia_consola();
  emite_texto("Compilando ");
  emite_texto(n_func);
  emite_texto("()...");
  emite_nueva_linea();
  hacia_archivo();

  total_regs = -1;        /* Registros requeridos para llamar otra función */
  variables_virtuales = 0;      /* Prepara la lista de variables virtuales */
  pila_args = 0;                /* Inicia la cuenta de argumentos */
  if (tipo->num_pars == 128) {  /* Procesamiento de declaración K&R */
    while (parentesis) {
      if (clave_lex == C_PAREND) {
        obt_lex();
        break;
      }
      if (clave_lex == C_IDENT) { /* Un nombre legal incrementa la cuenta */
        if (busca_loc(cad_lex) != NULL)
          redefinido(cad_lex);
        else {
          nueva_loc2(cad_lex, VARIABLE, AUTO, NULL, 0);
          ++pila_args;
        }
        obt_lex();
      } else {
        error("Nombre ilegal para el argumento");
        obt_lex();
      }
      if (clave_lex != C_PAREND) { /* Si no es parentesis, debe ser coma */
        if (clave_lex != C_COMA)
          error("Se requiere una coma");
        else
          obt_lex();
      }
      if (fin_sentencia())
        break;
    }
    while (p_tipo_1(0)) {  /* El usuario declara los tipos de los argumentos */
      if (clase_alm != 0 && clase_alm != REGISTER)
        error("Clase de almacenamiento incorrecta");
      tipos_args();
      punto_y_coma();
    }
    local = locales;       /* Según ANSI, si no tienen tipo son int */
    while (local != NULL) {
      if (local->tipo == NULL)
        local->tipo = t_sint;
      local = local->sig;
    }
  } else {  /* Tiene declaración ANSI, todo ha sido analizado antes */
    cadena = tipo->especial.proto;
    while (cadena != NULL) {
      if (busca_loc(cadena->nombre) != NULL)
        redefinido(cadena->nombre);
      else {
        nueva_loc2(cadena->nombre, VARIABLE, AUTO, cadena->tipo, 0);
        ++pila_args;
      }
      cadena = cadena->sig;
    }
  }
  ordena_args(pila_args);       /* Ordena los parametros de la función */
  etiq_lit = ++sig_etiq;        /* Obtiene una etiqueta para el buffer literal */
  ap_lit = 0;                   /* Limpia el buffer literal */
  total_funciones = 0;          /* Total de funciones llamadas */
  if (clave_lex == C_LLAVEI)
    funcion = sentencia();      /* Procesa las sentencias */
  else
    error("Falta llave de inicio");
  vacia_lits();                 /* Vacia los literales */
  gen_funcion(n_func, funcion); /* Genera el codigo definitivo */
  libera_sentencias(funcion);   /* Libera las listas generadas */
  libera_expr();
  funcion = NULL;
  dentro_funcion = NO;          /* Ahora no esta dentro de una función */
}

/*
** Declara los tipos de los argumentos.
*/
void tipos_args(void)
{
  struct tipo *nuevo_tipo;
  int p;
  struct nombres *ap_arg;

  while (1) {
    if (p_tipo_2(1)) {
      if (clave_lex == C_PAREND)
        obt_lex();
      else
        error("Falta parentesis derecho");
    }

    if (t_primero->tipo == FUNCION) {
      nuevo_tipo = crea_tipo(APUNTADOR);
      nuevo_tipo->sig = t_primero;
      t_primero = nuevo_tipo;
    }
    if (t_primero->tipo == ARREGLO) {
      nuevo_tipo = crea_tipo(APUNTADOR);
      nuevo_tipo->sig = t_primero->sig;
      t_primero = nuevo_tipo;
    }
    if (t_primero == t_float)
      t_primero = t_double;
    if ((ap_arg = busca_loc(nombre_tipo)) != NULL) {
      if (ap_arg->tipo != NULL)  /* Pone el tipo al parametro */
        error("Argumento redefinido");
      else
        ap_arg->tipo = t_primero;
    } else
      error("Se requiere el nombre de un argumento");
    if (fin_sentencia())
      return;
    if (clave_lex != C_COMA)
      error("Se requiere una coma");
    else
      obt_lex();
  }
}

/*
** Ordena los argumentos.
*/
void ordena_args(int cuantos)
{
  struct tipo *tipo, *tipo_func;
  struct nombres *ap_arg;
  int pos, pos2;   /* pos es para registros locales y pos2 para memoria */

  tipo_func = funcion_actual->tipo->sig;
  if (tipo_func->tipo != STRUCT)/* El primer byte es FUNCION */
    pos2 = 0;                   /* Función común */
  else
    pos2 = (tam_tipo(tipo_func) + 3) & ~3;
  pos = 2;                      /* Empezamos desde lr2 */
  ap_arg = locales;
  while (cuantos--) {
    tipo = ap_arg->tipo;
    if (tipo->tipo == DOUBLE)   /* Alineamos en registro par */
      pos = (pos + 1) & ~1;
    if (tipo->tipo != STRUCT)
      ap_arg->posicion = var_virtual((pos << 2) | 2, tipo->tipo == DOUBLE);
    else
      ap_arg->posicion = var_virtual((pos2 << 2) | 3, 0);
    if (tipo->tipo == STRUCT)
      pos2 += (tam_tipo(tipo) + 3) & ~3;
    else if (tipo->tipo == DOUBLE)
      pos += 2;
    else
      pos++;
    ap_arg = ap_arg->sig;
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
struct sentencia *sentencia(void)
{
  struct sentencia *temp;

  if (eof)
    return NULL;
  if (clave_lex == C_LLAVEI) {
    obt_lex();
    return p_bloque();
  }
  if (clave_lex == C_IF) {
    obt_lex();
    return s_if();
  }
  if (clave_lex == C_WHILE) {
    obt_lex();
    return s_while();
  }
  if (clave_lex == C_DO) {
    obt_lex();
    return s_do();
  }
  if (clave_lex == C_FOR) {
    obt_lex();
    return s_for();
  }
  if (clave_lex == C_SWITCH) {
    obt_lex();
    return s_switch();
  }
  if (clave_lex == C_CASE) {
    obt_lex();
    return s_case();
  }
  if (clave_lex == C_DEFAULT) {
    obt_lex();
    return s_default();
  }
  if (clave_lex == C_GOTO) {
    obt_lex();
    return s_goto();
  }
  if ((temp = p_etiqueta()) != NULL)
    return temp;
  if (clave_lex == C_RETURN) {
    obt_lex();
    return s_return();
  }
  if (clave_lex == C_BREAK) {
    obt_lex();
    return s_break();
  }
  if (clave_lex == C_CONTINUE) {
    obt_lex();
    return s_continue();
  }
  if (clave_lex == C_PCOMA) {
    obt_lex();
    return NULL;
  }
  almacena_expresion(SI);   /* Asume que es una expresión */
  temp = nueva_sentencia(t_expresion);
  temp->def.t_expresion.expresion = ultimo_nodo;
  punto_y_coma();
  return temp;
}

/*
** Checa punto y coma.
*/
void punto_y_coma(void)
{
  if (clave_lex != C_PCOMA)
    error("Falta punto y coma");
  else
    obt_lex();
}

/*
** Bloque de sentencias.
*/
struct sentencia *p_bloque(void)
{
  int c_dentro_switch, cuenta = 0;
  struct nodo *nodo_vars, *nodo_var;
  struct nombres *local, *local2, *local3;
  struct sentencia *lista = NULL, *temp, *agregado;
  struct contexto bloque;

  salva_contexto(&ultimo_tipo, &bloque);
  c_dentro_switch = dentro_switch;
  if (nivel - dentro_switch >= 1)
    dentro_switch = 0;          /* Vuelve a permitir declaraciones */
  local = locales;              /* Variables locales */
  ++nivel;                      /* Un nuevo nivel */
  nodo_vars = ultimo_nodo;
  temp = lista = decl_loc();    /* Procesa declaraciones locales */
  while (temp != NULL && temp->sig != NULL)
    temp = temp->sig;
  ultimo_nodo = nodo_vars;
  while (!eof && clave_lex != C_LLAVED) {
    if (lista == NULL) {
      lista = sentencia();      /* Procesa sentencias */
      temp = lista;
    } else if (temp != NULL) {
      temp->sig = sentencia();
    } else {
      temp = sentencia();
    }
    while (temp != NULL && temp->sig != NULL)
      temp = temp->sig;
  }
  obt_lex();
  --nivel;                      /* Cierra el nivel */
  local2 = locales;             /* Checa el número de vars. locales */
  if (nivel) {                  /* Mantiene las etiquetas para goto */
    locales = local;
    while (local2 != local) {
      local3 = local2->sig;
      if (local2->ident == ETIQUETA) {
        local2->sig = locales;
        locales = local2;
      } else
        free(local2);
      local2 = local3;
    }
  } else {
    while (local2 != NULL) {
      local3 = local2->sig;
      free(local2);
      local2 = local3;
    }
    locales = NULL;             /* Limpia las variables locales */
  }
  dentro_switch = c_dentro_switch;
  restaura_contexto(&bloque);
  return lista;
}

/*
** Sentencia "if"
*/
struct sentencia *s_if(void)
{
  struct sentencia *temp;

  temp = nueva_sentencia(t_if);
  if (clave_lex == C_PARENI)
    obt_lex();
  else
    error("Falta parentesis izquierdo");
  compara_no_cero(almacena_expresion(SI));
  if (clave_lex == C_PAREND)
    obt_lex();
  else
    error("Falta parentesis derecho");
  temp->def.t_if.expresion = ultimo_nodo;
  temp->def.t_if.lista1 = sentencia();
  if (clave_lex != C_ELSE) {
    temp->def.t_if.lista2 = NULL;
    return temp;
  }
  obt_lex();
  temp->def.t_if.lista2 = sentencia();
  return temp;
}

/*
** Sentencia "while"
*/
struct sentencia *s_while(void)
{
  struct bucle bucle;              /* Crea una entrada */
  struct sentencia *temp;

  bucle.ant = ultimo_bucle;
  bucle.bucle = ++sig_etiq;
  bucle.fin = ++sig_etiq;
  ultimo_bucle = &bucle;
  temp = nueva_sentencia(t_while);
  temp->def.t_while.etiqueta_continue = bucle.bucle;
  temp->def.t_while.etiqueta_break = bucle.fin;
  if (clave_lex == C_PARENI)
    obt_lex();
  else
    error("Falta parentesis izquierdo");
  compara_no_cero(almacena_expresion(SI));
  if (clave_lex == C_PAREND)
    obt_lex();
  else
    error("Falta parentesis derecho");
  temp->def.t_while.expresion = ultimo_nodo;
  temp->def.t_while.lista = sentencia();
  ultimo_bucle = bucle.ant;        /* Borra de la cola */
  return temp;
}

/*
** Sentencia "do"
*/
struct sentencia *s_do(void)
{
  struct bucle bucle;              /* Crea una entrada */
  struct sentencia *temp;

  bucle.ant = ultimo_bucle;
  bucle.bucle = ++sig_etiq;
  bucle.fin = ++sig_etiq;
  ultimo_bucle = &bucle;           /* Agrega a la cola (para el break) */
  temp = nueva_sentencia(t_do);
  temp->def.t_while.etiqueta_continue = bucle.bucle;
  temp->def.t_while.etiqueta_break = bucle.fin;
  temp->def.t_while.lista = sentencia();
  if (clave_lex == C_WHILE)
    obt_lex();
  else
    error("Falta el while");
  if (clave_lex == C_PARENI)
    obt_lex();
  else
    error("Falta parentesis izquierdo");
  compara_no_cero(almacena_expresion(SI));
  if (clave_lex == C_PAREND)
    obt_lex();
  else
    error("Falta parentesis derecho");
  temp->def.t_while.expresion = ultimo_nodo;
  punto_y_coma();
  ultimo_bucle = bucle.ant;        /* Borra de la cola */
  return temp;
}

/*
** Sentencia "for"
*/
struct sentencia *s_for(void)
{
  struct bucle bucle;
  struct sentencia *temp;

  bucle.ant = ultimo_bucle;
  bucle.bucle = ++sig_etiq;
  bucle.fin = ++sig_etiq;
  ultimo_bucle = &bucle;
  temp = nueva_sentencia(t_for);
  if (clave_lex == C_PARENI)
    obt_lex();
  else
    error("Falta parentesis izquierdo");
  if (clave_lex != C_PCOMA) {
    almacena_expresion(SI);
    temp->def.t_for.expresion1 = ultimo_nodo;
    punto_y_coma();
  } else {
    obt_lex();
    temp->def.t_for.expresion1 = NULL;
  }
  if (clave_lex != C_PCOMA) {
    compara_no_cero(almacena_expresion(SI));
    temp->def.t_for.expresion2 = ultimo_nodo;
    punto_y_coma();
  } else {
    obt_lex();
    temp->def.t_for.expresion2 = NULL;
  }
  if (clave_lex != C_PAREND) {
    almacena_expresion(SI);
    temp->def.t_for.expresion3 = ultimo_nodo;
    if (clave_lex == C_PAREND)
      obt_lex();
    else
      error("Falta parentesis derecho");
  } else {
    obt_lex();
    temp->def.t_for.expresion3 = NULL;
  }
  temp->def.t_for.lista = sentencia();
  temp->def.t_for.etiqueta_continue = bucle.bucle;
  temp->def.t_for.etiqueta_break = bucle.fin;
  ultimo_bucle = bucle.ant;
  return temp;
}

/*
** Sentencia "switch"
*/
struct sentencia *s_switch(void)
{
  struct bucle bucle;
  int c_dentro_switch;
  int c_etiqueta_default;
  struct sentencia *c_lista_case;
  struct sentencia *c_ultimo_case;
  struct sentencia *temp;

  c_dentro_switch = dentro_switch;
  c_etiqueta_default = etiqueta_default;
  c_lista_case = lista_case;
  c_ultimo_case = ultimo_case;
  lista_case = NULL;
  ultimo_case = NULL;
  etiqueta_default = 0;
  dentro_switch = nivel;
  bucle.ant = ultimo_bucle;
  bucle.bucle = 0;
  bucle.fin = ++sig_etiq;
  ultimo_bucle = &bucle;
  temp = nueva_sentencia(t_switch);
  temp->def.t_while.etiqueta_continue = 0;
  temp->def.t_while.etiqueta_break = bucle.fin;
  if (clave_lex == C_PARENI)
    obt_lex();
  else
    error("Falta parentesis izquierdo");
  checa_entero(almacena_expresion(SI));
  if (clave_lex == C_PAREND)
    obt_lex();
  else
    error("Falta parentesis derecho");
  temp->def.t_while.expresion = ultimo_nodo;
  temp->def.t_while.lista = sentencia();
  dentro_switch = c_dentro_switch;
  etiqueta_default = c_etiqueta_default;
  ultimo_bucle = bucle.ant;
  ultimo_case = c_ultimo_case;
  lista_case = c_lista_case;
  return temp;
}

/*
** Sentencia "case"
*/
struct sentencia *s_case(void)
{
  int num;
  struct sentencia *temp;

  if (!dentro_switch)
    error("El case no esta en un switch");
  num = expr_constante();       /* Busca el número */
  temp = lista_case;
  while (temp != NULL) {
    if (temp->def.t_case.constante == num) {
      error("case repetido");
      break;
    }
    temp = temp->def.t_case.sig_case;
  }
  temp = nueva_sentencia(t_case);
  if (lista_case == NULL)
    lista_case = temp;
  if (ultimo_case != NULL)
    ultimo_case->def.t_case.sig_case = temp;
  ultimo_case = temp;
  temp->def.t_case.sig_case = NULL;
  temp->def.t_case.constante = num;
  temp->def.t_case.etiqueta = ++sig_etiq;
  if (clave_lex == C_DPUNTOS)
    obt_lex();
  else
    error("Falta simbolo de dos puntos");
  return temp;
}

/*
** Sentencia "default"
*/
struct sentencia *s_default(void)
{
  struct sentencia *temp;

  if (!dentro_switch)
    error("El default no esta en un switch");
  else if (etiqueta_default)
    error("El default esta repetido");
  if (clave_lex == C_DPUNTOS)
    obt_lex();
  else
    error("Falta simbolo de dos puntos");
  temp = nueva_sentencia(t_default);
  temp->def.t_break.etiqueta = etiqueta_default = ++sig_etiq;
  return temp;
}

/*
** Sentencia "goto"
*/
struct sentencia *s_goto(void)
{
  unsigned char n[TAM_NOMBRE];
  struct sentencia *temp;

  temp = nueva_sentencia(t_goto);
  if (clave_lex == C_IDENT) {
    temp->def.t_break.etiqueta = agrega_etiqueta(cad_lex);
    obt_lex();
  } else
    error("Etiqueta incorrecta");
  punto_y_coma();
  return temp;
}

/*
** Procesa una posible definición de etiqueta para goto
*/
struct sentencia *p_etiqueta(void)
{
  struct sentencia *temp;

  if (clave_lex == C_IDENT) {
    while (*pos_linea == ' ')
      pos_linea++;
    if (*pos_linea == ':' && *(pos_linea + 1) != ':') {
      temp = nueva_sentencia(t_etiqueta);
      temp->def.t_break.etiqueta = agrega_etiqueta(cad_lex);
      obt_lex();
      obt_lex();
      return temp;
    }
  }
  return NULL;
}

int agrega_etiqueta(char *nombre)
{
  struct nombres *ap;

  if ((ap = busca_loc(nombre)) != NULL) {
    if (ap->ident != ETIQUETA)
      error("No es una etiqueta");
  } else
    ap = nueva_loc(nombre, ETIQUETA, AUTO, NULL, ++sig_etiq);
  return ap->posicion;
}

/*
** Sentencia "return"
*/
struct sentencia *s_return(void)
{
  struct nodo *nodo_expr;
  struct tipo *tipo, *tipo2, *tipo_temp;
  struct sentencia *temp;

  temp = nueva_sentencia(t_return);
  temp->def.t_return.expresion = NULL;
  temp->def.t_return.informacion = 0;

  /* Checa si hay una expresión */
  if (fin_sentencia() == 0) {
    tipo2 = almacena_expresion(SI);
    tipo = funcion_actual->tipo->sig;
    if (tipo->tipo == VOID)
      error("La función tiene tipo void");
    if (tipo->tipo == STRUCT) {             /* El primer byte es FUNCION */
      if (tipo2->tipo != STRUCT)
        error("El resultado no tiene tipo de estructura");
      else if (tipo->especial.est != tipo2->especial.est)
        error("Estructuras incompatibles");
      temp->def.t_return.informacion = tam_tipo(tipo2);
    } else if (tipo2->tipo == STRUCT) {
      error("La función no tiene tipo de estructura");
    } else {
      nodo_expr = ultimo_nodo;
      if (tipo2->tipo == FUNCION) {
        tipo_temp = crea_tipo(APUNTADOR);
        tipo_temp->sig = tipo2;
        tipo2 = tipo_temp;
      }
      convierte_tipo(&nodo_expr, tipo2, tipo, SI);
    }
    temp->def.t_return.expresion = ultimo_nodo;
  }
  punto_y_coma();
  return temp;
}

/*
** Sentencia "break"
*/
struct sentencia *s_break(void)
{
  struct sentencia *temp;

  temp = nueva_sentencia(t_break);
  temp->def.t_break.etiqueta = 0;
  if (ultimo_bucle == NULL)     /* Checa si hay un bucle abierto */
    error("break sin bucle");
  else
    temp->def.t_break.etiqueta = ultimo_bucle->fin;
  punto_y_coma();
  return temp;
}

/*
** Sentencia "continue"
*/
struct sentencia *s_continue(void)
{
  struct bucle *u_bucle;
  struct sentencia *temp;

  temp = nueva_sentencia(t_continue);
  u_bucle = ultimo_bucle;     /* Checa si hay un bucle abierto */
  while (1) {
    if (u_bucle == NULL) {
      error("continue sin bucle");
      break;
    }
    if (u_bucle->bucle)
      break;
    u_bucle = u_bucle->ant;
  }
  if (u_bucle != NULL)
    temp->def.t_break.etiqueta = u_bucle->bucle;
  else
    temp->def.t_break.etiqueta = 0;
  punto_y_coma();
  return temp;
}

/*
** Detecta el fin de una sentencia, un punto y coma
** o el fin de archivo.
*/
int fin_sentencia(void)
{
  return (clave_lex == C_PCOMA) || (eof);
}

void redefinido(char *nombre)
{
  error("Nombre redefinido");
}

/*
** Nueva sentencia
*/
struct sentencia *nueva_sentencia(enum tipo_sentencia tipo)
{
  struct sentencia *temp;

  switch (tipo) {
    case t_if       : temp = malloc(sizeof(struct sentencia_if));
                      break;

    case t_while    :
    case t_do       :
    case t_switch   : temp = malloc(sizeof(struct sentencia_while));
                      break;

    case t_for      : temp = malloc(sizeof(struct sentencia_for));
                      break;

    case t_case     : temp = malloc(sizeof(struct sentencia_case));
                      break;

    case t_return   : temp = malloc(sizeof(struct sentencia_return));
                      break;

    case t_expresion: temp = malloc(sizeof(struct sentencia_expr));
                      break;

    case t_break    :
    case t_continue :
    case t_goto     :
    case t_default  :
    case t_etiqueta : temp = malloc(sizeof(struct sentencia_break));
                      break;

    default         : error("Error interno del compilador");
                      cancela();
  }
  if (temp == NULL) {
    error("Función muy compleja");
    cancela();
  }
  temp->sig = NULL;
  temp->tipo = tipo;
  return temp;
}

/*
** Libera una lista de sentencias
*/
void libera_sentencias(struct sentencia *lista)
{
  struct sentencia *enlace, *temp;

  enlace = lista;
  while (enlace != NULL) {
    switch (enlace->tipo) {
      case t_if      : libera_sentencias(enlace->def.t_if.lista1);
                       libera_sentencias(enlace->def.t_if.lista2);
                       break;

      case t_while   :
      case t_do      :
      case t_switch  : libera_sentencias(enlace->def.t_while.lista);
                       break;

      case t_for     : libera_sentencias(enlace->def.t_for.lista);
                       break;

      case t_return  :
      case t_expresion:
      case t_case    :
      case t_default :
      case t_goto    :
      case t_etiqueta:
      case t_continue:
      case t_break   : break;

      default        : error("Error interno del compilador");
                       cancela();
    }
    temp = enlace->sig;
    free(enlace);
    enlace = temp;
  }
}
