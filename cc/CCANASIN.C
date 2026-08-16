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
