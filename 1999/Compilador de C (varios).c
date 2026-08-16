/*
** Compilador de C para G11.
** Preprocesador y funciones varias.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 01-jun-1995.
** Revisión: 07-sep-1998. Rediseño para apoyar el analizador léxico.
** Revisión: 14-sep-1998. Se elimina isspace(), ya soporta los trigráfos, y
**                        también concatenación de cadenas ANSI, el análisis
**                        léxico ya cumple ANSI, inicio del rediseño del
**                        preprocesador para que sea ANSI.
** Revisión: 15-sep-1998. El preprocesador ya es ANSI, me costo mucho trabajo.
** Revisión: 09-oct-1998. Los archivos <x.h> los busca en el directorio del
**                        compilador, y acomoda correctamente caminos relativos.
** Revisión: 26-oct-1998. Corrección en la substitución de #macro. Corrección
**                        del funcionamiento de #line.
** Revisión: 24-nov-1998. Se acelera el procesamiento de la entrada. Procesa
**                        nuevos #pragma (para programas grandes)
*/

/*
** !!! Curiosidad: ¿El tipo de una cadena literal debe ser apuntador a char
**                 o arreglo de char?
*/

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

.global _compara_cadenas
_compara_cadenas:
load 0,&14,gr96,lr2
exbyte gr96,gr96,0
load 0,&14,gr97,lr3
exbyte gr97,gr97,0
cpeq gr96,gr96,gr97
jmpfi gr96,lr0
cpeq gr97,gr97,0
jmpti gr97,lr0
const gr96,1
add lr2,lr2,1
jmp _compara_cadenas
add lr3,lr3,1
#endasm

/*
** Nota: Esta tabla esta modificada para que el caracter 0x5f
**       sea tomado como una letra.
*/
char _ctype[256] = {
  0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
  0x09, 0x09, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x40, 0x40, 0x40, 0x40, 0x02,
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

/*
** Función para cálcular la dispersión PJW (P.J. Weinberger)
**
** Tomada del libro "Compiladores: Principios, técnicas y herramientas"
** Alfred V. Aho, Ravi Sethi, Jeffrey D. Ullman. Addison-Wesley 1990.
** Páginas 450-452.
*/
int calcula_dispersion(char *cadena)
{
  int val = 0, temp;

  while (*cadena) {
    val = (val << 4) + *cadena++;
    if (temp = val & 0xf0000000) {
      val ^= temp >> 24;
      val &= ~0xf0000000;
    }
  }
  return val % NUM_PRIMO;
}

/*
** Una nueva variable/función global.
*/
struct nombres *nueva_glb(char *nombre, int id, int clase,
                          struct tipo *tipo, int valor)
{
  struct nombres *ap;
  int dispersion;

  ap = malloc(sizeof(struct nombres) + strlen(nombre));
  if (ap == NULL) {
    error("No hay memoria");
    return NULL;
  }
  dispersion = calcula_dispersion(nombre);
  ap->sig = tabla[dispersion];
  tabla[dispersion] = ap;
  ap->ident = id;
  ap->clase = clase;
  ap->nivel = 0;
  ap->tipo = tipo;
  ap->posicion = valor;
  strcpy(ap->nombre, nombre);
  return ap;
}

/*
** Una nueva variable local, agregada al principio de la lista.
** (esta variación se usa para las variables locales)
*/
struct nombres *nueva_loc(char *nombre, int id, int clase,
                          struct tipo *tipo, int valor)
{
  struct nombres *ap;

  ap = malloc(sizeof(struct nombres) + strlen(nombre));
  if (ap == NULL) {
    error("No hay memoria");
    return NULL;
  }
  ap->sig = locales;
  locales = ap;
  ap->ident = id;
  ap->clase = clase;
  ap->nivel = nivel;
  ap->tipo = tipo;
  ap->posicion = valor;
  strcpy(ap->nombre, nombre);
  return ap;
}

/*
** Una nueva variable local, agregada al final de la lista.
** (esta variación se usa para agregar los parametros al comienzo
** de una definición de función)
*/
struct nombres *nueva_loc2(char *nombre, int id, int clase,
                           struct tipo *tipo, int valor)
{
  struct nombres *ap, *final;

  ap = malloc(sizeof(struct nombres) + strlen(nombre));
  if (ap == NULL) {
    error("No hay memoria");
    return NULL;
  }
  if (locales == NULL)
    locales = ap;
  else {
    final = locales;
    while (final->sig != NULL)
      final = final->sig;
    final->sig = ap;
  }
  ap->sig = NULL;
  ap->ident = id;
  ap->clase = clase;
  ap->nivel = nivel;
  ap->tipo = tipo;
  ap->posicion = valor;
  strcpy(ap->nombre, nombre);
  return ap;
}

/*
** Una nueva estructura (en realidad un rótulo de estructura).
*/
struct rotulo *nueva_estructura(char *nombre)
{
  struct rotulo *ap;
  int dispersion;

  ap = malloc(sizeof(struct rotulo) + strlen(nombre));
  if (ap == NULL) {
    error("No hay memoria");
    return NULL;
  }
  dispersion = calcula_dispersion(nombre);
  ap->sig = tabla_estruct[dispersion];
  tabla_estruct[dispersion] = ap;
  ap->es_union = 0;
  ap->que_es = 0;
  ap->lista = NULL;
  ap->tam = 0;
  strcpy(ap->nombre, nombre);
  return ap;
}

/*
** Un nuevo miembro de una estructura.
*/
struct miembro *nuevo_miembro(struct miembro **lista, char *nombre)
{
  struct miembro *nuevo, *sig;

  nuevo = malloc(sizeof(struct miembro) + strlen(nombre));
  if (nuevo == NULL) {
    error("No hay memoria");
    return NULL;
  }
  if (*lista == NULL)
    *lista = nuevo;
  else {
    sig = *lista;
    while (sig->sig != NULL)
      sig = sig->sig;
    sig->sig = nuevo;
  }
  nuevo->sig = NULL;
  nuevo->tipo = NULL;
  nuevo->posicion = 0;
  strcpy(nuevo->nombre, nombre);
  return nuevo;
}

/*
** Un nuevo enumerador.
*/
void nuevo_enum(char *nombre, int valor)
{
  struct enumerador *nuevo;
  int dispersion;

  nuevo = malloc(sizeof(struct enumerador) + strlen(nombre));
  if (nuevo == NULL) {
    error("No hay memoria");
    return;
  }
  dispersion = calcula_dispersion(nombre);
  nuevo->sig = tabla_enum[dispersion];
  tabla_enum[dispersion] = nuevo;
  nuevo->valor = valor;
  strcpy(nuevo->nombre, nombre);
}

/*
** Busca una variable/función global.
*/
struct nombres *busca_glb(char *nombre)
{
  struct nombres *ap;

  ap = tabla[calcula_dispersion(nombre)];
  while (ap != NULL) {
    if (compara_cadenas(nombre, ap->nombre))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Busca una variable/función local.
*/
struct nombres *busca_loc(char *nombre)
{
  struct nombres *ap;

  ap = locales;
  while (ap != NULL) {
    if (compara_cadenas(nombre, ap->nombre))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Busca una estructura.
*/
struct rotulo *busca_estructura(char *nombre)
{
  struct rotulo *ap;

  ap = tabla_estruct[calcula_dispersion(nombre)];
  while (ap != NULL) {
    if (compara_cadenas(nombre, ap->nombre))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Busca un miembro de estructura.
*/
struct miembro *busca_miembro(struct miembro *lista, char *nombre)
{
  while (lista != NULL) {
    if (compara_cadenas(nombre, lista->nombre))
      return lista;
    lista = lista->sig;
  }
  return NULL;
}

/*
** Busca un enumerador.
*/
struct enumerador *busca_enum(char *nombre)
{
  struct enumerador *ap;

  ap = tabla_enum[calcula_dispersion(nombre)];
  while (ap != NULL) {
    if (compara_cadenas(nombre, ap->nombre))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Imprime un retorno de carro y una cadena a la consola.
*/
void mensaje(char *cad)
{
}

/*
** Retorna el caracter actual y avanza.
*/
int obt_car(void)
{
  if (*pos_linea)
    return *pos_linea++;
  else
    return 0;
}

/*
** Obtiene otra linea de la entrada.
*/
void lee_linea(char *base)
{
  int k, estado;
  int unidad;

  while (1) {
    pos_linea = base;
    *pos_linea = 0;
    if (entrada == -1) {
      mensaje_urgente(ventana, 2, 0x00480000, 0x006bffff);
      eof = 1;
      return;
    }
    unidad = entrada;
    estado = 0;
    while ((k = fgetc(unidad)) >= 0) {
      if (_ctype[k] & 32)
        continue;
      if (k == '\t')
        k = ' ';
      if (k == '\n' || (pos_linea - base) >= MAX_LINEA)
        break;
      if (k == '?') {
        if (++estado == 3)
          estado = 1;
      } else if (estado == 2) {
        switch (k) {
          case '(':
            pos_linea -= 2;
            k = '[';
            break;
          case '/':
            pos_linea -= 2;
            k = '\\';
            break;
          case ')':
            pos_linea -= 2;
            k = ']';
            break;
          case '\'':
            pos_linea -= 2;
            k = '^';
            break;
          case '<':
            pos_linea -= 2;
            k = '{';
            break;
          case '!':
            pos_linea -= 2;
            k = '|';
            break;
          case '>':
            pos_linea -= 2;
            k = '}';
            break;
          case '-':
            pos_linea -= 2;
            k = '~';
            break;
          case '=':
            pos_linea -= 2;
            k = '#';
            break;
        }
        estado = 0;
      } else
        estado = 0;
      *pos_linea++ = k;
    }
    *pos_linea = 0;         /* Agrega un caracter nulo */
    lineas_totales++;
    archivo_actual->linea_actual++; /* Se ha leido una línea más */
    if ((++archivo_actual->linea_real & 31) == 0) {
      mensaje_urgente(ventana, 2, 0x00480000, 0x006bffff);
      multitarea();
    }
    if (k <= 0 && archivo_actual != NULL)
      fin_include();
    if (pos_linea != base) {
      pos_linea = base;
      return;
    }
  }
}

/*
** Efectua el preprocesamiento.
*/
void preprocesa(void)
{
  int k, car, c, literal;
  char nombre[TAM_NOMBRE], *def, *busqueda;
  int subs, pars, args, paren, m, hay_if, hay_include, hay_line;
  struct macro *macro;

  hay_if = hay_include = hay_line = NO;
  dentro_pp = SI;
  while (1) {
    primer_paso();
    if (eof) {
      dentro_pp = NO;
      return;
    }
    obt_lex();
    if (clave_lex == C_PREPROC1) {
      obt_lex();
      if (clave_lex == C_NULO)
        continue;
      if (clave_lex != C_IDENT && grupo_lex != GRUPO_0) {
        error("Directiva incorrecta");
        continue;
      }
      if (compara_cadenas("ifdef", cad_lex)) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        obt_lex();
        if (clave_lex != C_IDENT && grupo_lex != GRUPO_0)
          error("No es un nombre legal");
        if (macro_especial(cad_lex) || busca_macro(cad_lex) == NULL)
          evadir_nivel = nivel_if;
        continue;
      }
      if (compara_cadenas("ifndef", cad_lex)) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        obt_lex();
        if (clave_lex != C_IDENT && grupo_lex != GRUPO_0)
          error("No es un nombre legal");
        if (macro_especial(cad_lex) || busca_macro(cad_lex) != NULL)
          evadir_nivel = nivel_if;
        continue;
      }
      if (compara_cadenas("if", cad_lex)) {
        ++nivel_if;
        if (evadir_nivel)
          continue;
        hay_if = SI;
        break;
      }
      if (compara_cadenas("elif", cad_lex)) {
        if (nivel_if == 0)
          error("No hay #if...");
        if (evadir_nivel)
          continue;
        hay_if = SI;
        break;
      }
      if (compara_cadenas("else", cad_lex)) {
        if (nivel_if) {
          if (evadir_nivel == nivel_if)
            evadir_nivel = 0;
          else if (evadir_nivel == 0)
            evadir_nivel = nivel_if;
        } else
          error("No hay #if...");
        continue;
      }
      if (compara_cadenas("endif", cad_lex)) {
        if (nivel_if) {
          if (evadir_nivel == nivel_if)
            evadir_nivel = 0;
          --nivel_if;
        } else error("No hay #if...");
        continue;
      }
      if (evadir_nivel)
        continue;
      if (compara_cadenas("asm", cad_lex)) {
        while (1) {
          lee_linea(linea);
          if (eof) {
            dentro_pp = NO;
            return;
          }
          espacios();
          if (*pos_linea == '#')
            break;
          emite_linea(linea);
        }
        continue;
      }
      if (compara_cadenas("include", cad_lex)) {
        while (*pos_linea == ' ')
          pos_linea++;
        if (*pos_linea == '<' || *pos_linea == '"') {
          p_include();
          continue;
        } else {
          hay_include = SI;
          break;
        }
      }
      if (compara_cadenas("define", cad_lex)) {
        nueva_macro();
        continue;
      }
      if (compara_cadenas("undef", cad_lex)) {
        obt_lex();
        if (clave_lex != C_IDENT && grupo_lex != GRUPO_0)
          error("No es un nombre legal");
        if (macro_especial(cad_lex))
          error("No es posible borrar macro especial");
        else
          borra_macro(cad_lex);
        continue;
      }
      if (compara_cadenas("line", cad_lex)) {
        hay_line = SI;
        break;
      }
      if (compara_cadenas("error", cad_lex)) {
        espacios();
        error(pos_linea);
        continue;
      }
      if (compara_cadenas("pragma", cad_lex)) {
        obt_lex();
        if (clave_lex == C_IDENT) {
          if (compara_cadenas("version", cad_lex))
            error(PROGRAMA);
          if (compara_cadenas("ajedrez", cad_lex))
            inicia_tarea("X:/Ajedrez con sonido", "");
          if (compara_cadenas("no_advertir_ansi", cad_lex))
            adv_ansi = NO;
          if (compara_cadenas("advertir_ansi", cad_lex))
            adv_ansi = SI;
          if (compara_cadenas("no_advertir_conversion", cad_lex))
            adv_cs = NO;
          if (compara_cadenas("advertir_conversion", cad_lex))
            adv_cs = SI;
          if (compara_cadenas("programa_grande", cad_lex))
            prog_grande = SI;
          if (compara_cadenas("programa_normal", cad_lex))
            prog_grande = NO;
        }
        continue;
      }
    }
    if (evadir_nivel)
      continue;
    pos_linea = linea;
    break;
  }
  literal = NO;
  subs = SI;
  while (subs) {
    subs = NO;
    pos_linea_m = linea_m;
    while (car = *pos_linea++) {  /* !!! Anotar las macros que ya no debe substituir */
      if (car == ' ')
        pp_espacios();
      else if (car == '"')
        pp_comillas();
      else if (car == 39)
        pp_apostrofe();
      else if (isalpha(car)) {
        pos_linea--;
        k = 0;
        while (isalnum(*pos_linea)) {
          if (k < MAX_NOMBRE)
            nombre[k++] = *pos_linea;
          obt_car();
        }
        nombre[k] = 0;
        if (k = macro_especial(nombre)) {
          if (literal)
            strcpy(nombre, "\"");
          else
            strcpy(nombre, "");
          switch (k) {
            case 1: /* __DATE__ */
              strcat(nombre, "\"Sep 14 1998\"");  /* !!! Leer la fecha */
              break;
            case 2: /* __FILE__ */
              strcat(nombre, "\"");
              strcat(nombre, archivo_actual->nombre_actual);
              strcat(nombre, "\"");
              break;
            case 3: /* __LINE__ */
              k = archivo_actual->linea_actual;
              def = nombre;
              while (*def)
                def++;
              pars = 10000;
              m = 0;
              while (pars) {
                subs = ((k / pars) % 10) + '0';
                k %= pars;
                pars /= 10;
                if (subs == '0' && m == 0 && pars)
                  continue;
                *def++ = subs;
                m = 1;
              }
              *def = 0;
              break;
            case 4: /* __STDC__ */
              strcat(nombre, "1");
              break;
            case 5: /* __TIME__ */
              strcat(nombre, "\"19:52:39\"");     /* !!! Leer la hora */
              break;
            case 6: /* __cplusplus */
              /* Una pequeña exageración */
              break;
          }
          if (literal)
            strcat(nombre, "\"");
          literal = 0;
          k = 0;
          while (c = nombre[k++])
            almacena_car(c);
        } else if ((macro = busca_macro(nombre)) != NULL) {
          m = 0;
          def = macro->definicion;
          pars = macro->parametros;
          if (pars >= 0) {
            espacios();
            if (*pos_linea != '(')
              error("Falta parentesis izquierdo en macro");
            obt_car();
            k = m;
            args = paren = 0;
            while (1) {
              if (*pos_linea == '\0')
                lee_linea(linea);
              if (eof)
                break;
              if (*pos_linea == '"') {
                amacs[m++] = obt_car();
                while (*pos_linea && (*pos_linea != '"'
                    || *(pos_linea - 1) == '\\'
                    && *(pos_linea - 2) != '\\')) {
                  if (m < MAX_AMAC)
                    amacs[m++] = obt_car();
                  else {
                    error("Macro muy complicada");
                    cancela();
                  }
                }
                amacs[m++] = obt_car();
                continue;
              }
              if (*pos_linea == '\'') {
                amacs[m++] = obt_car();
                while (*pos_linea && (*pos_linea != '\''
                    || *(pos_linea - 1) == '\\'
                    && *(pos_linea - 2) != '\\')) {
                  if (m < MAX_AMAC)
                    amacs[m++] = obt_car();
                  else {
                    error("Macro muy complicada");
                    cancela();
                  }
                }
                amacs[m++] = obt_car();
                continue;
              }
              if (*pos_linea == ',' && paren == 0) {
                if (m - k == 0)
                  error("Parametro vacio");
                amacs[m++] = 0;
                pos_linea++;
                ++args;
                k = m;
                continue;
              }
              if (*pos_linea == '(')
                ++paren;
              if (*pos_linea == ')') {
                if (paren == 0) {
                  if (m - k) {
                    amacs[m++] = 0;
                    ++args;
                  }
                  break;
                }
                --paren;
              }
              if (m < MAX_AMAC)
                amacs[m++] = obt_car();
              else {
                error("Macro muy complicada");
                cancela();
              }
            }
            if (args != pars)
              error("Número incorrecto de parametros");
            if (*pos_linea != ')')
              error("Falta parentesis derecho en macro");
            else
              pos_linea++;
          }
          if (literal)
            almacena_car('"');
          while (*def) {
            if (*def != 127)
              almacena_car(*def++);
            else {
              busqueda = amacs;
              k = *++def;
              while (--k)
                while (*busqueda++) ;
              while (*busqueda)
                almacena_car(*busqueda++);
              ++def;
            }
          }
          if (literal)
            almacena_car('"');
          literal = 0;
          subs = SI;
        } else {
          if (literal) {
            error("El operador # no esta con nombre de macro");
            literal = 0;
          }
          k = 0;
          while (c = nombre[k++])
            almacena_car(c);
        }
      } else {
        if (car == '#')
          literal = 1;
        else {
          if (literal) {
            error("El operador # no esta con nombre de macro");
            literal = 0;
          }
          almacena_car(car);
        }
      }
    }
    almacena_car(0);
    strcpy(linea, linea_m);
    pos_linea = linea;
    if ((pos_linea_m - linea_m) >= MAX_LINEA) {
      error("Línea muy larga");
      break;
    }
  }
  if (hay_if) {
    obt_lex();
    if (expr_preproc() == 0)
      evadir_nivel = nivel_if;
    pos_linea = linea;
    *pos_linea = 0;
  } else if (hay_include) {
    p_include();
    pos_linea = linea;
    *pos_linea = 0;
  } else if (hay_line) {
    p_line();
    pos_linea = linea;
    *pos_linea = 0;
  }
  dentro_pp = NO;
}

/*
** Evalua una expresión del preprocesador
*/
int expr_preproc(void)
{
  int valor;

  valor = expr_1();
  while (clave_lex == C_COMA) {
    obt_lex();
    valor = expr_1();
  }
  return valor;
}

int expr_1(void)
{
  int valor1, valor2, valor3;

  valor1 = expr_2();
  if (clave_lex == C_TRINARIO) {
    obt_lex();
    valor2 = expr_2();
    if (clave_lex == C_DPUNTOS)
      obt_lex();
    else
      error("Falta simbolo de dos puntos");
    valor3 = expr_2();
    valor1 = valor1 ? valor2 : valor3;
  }
  return valor1;
}

int expr_2(void)
{
  int valor1, valor2;

  valor1 = expr_3();
  while (clave_lex == C_OROR) {
    obt_lex();
    valor2 = expr_3();
    valor1 = valor1 || valor2;
  }
  return valor1;
}

int expr_3(void)
{
  int valor1, valor2;

  valor1 = expr_4();
  while (clave_lex == C_ANDAND) {
    obt_lex();
    valor2 = expr_4();
    valor1 = valor1 && valor2;
  }
  return valor1;
}

int expr_4(void)
{
  int valor;

  valor = expr_5();
  while (clave_lex == C_OR) {
    obt_lex();
    valor |= expr_5();
  }
  return valor;
}

int expr_5(void)
{
  int valor;

  valor = expr_6();
  while (clave_lex == C_XOR) {
    obt_lex();
    valor ^= expr_6();
  }
  return valor;
}

int expr_6(void)
{
  int valor;

  valor = expr_7();
  while (clave_lex == C_AND) {
    obt_lex();
    valor &= expr_7();
  }
  return valor;
}

int expr_7(void)
{
  int valor;

  valor = expr_8();
  if (grupo_lex != GRUPO_2)
    return valor;
  while (1) {
    if (clave_lex == C_IGUALIGUAL) {
      obt_lex();
      valor = valor == expr_8();
    } else if (clave_lex == C_NOIGUAL) {
      obt_lex();
      valor = valor != expr_8();
    } else
      return valor;
  }
}

int expr_8(void)
{
  int valor;

  valor = expr_9();
  if (grupo_lex != GRUPO_3)
    return valor;
  while (1) {
    if (clave_lex == C_MENOR) {
      obt_lex();
      valor = valor < expr_9();
    } else if (clave_lex == C_MAYOR) {
      obt_lex();
      valor = valor > expr_9();
    } else if (clave_lex == C_MENORIGUAL) {
      obt_lex();
      valor = valor <= expr_9();
    } else if (clave_lex == C_MAYORIGUAL) {
      obt_lex();
      valor = valor >= expr_9();
    } else
      return valor;
  }
}

int expr_9(void)
{
  int valor;

  valor = expr_10();
  if (grupo_lex != GRUPO_4)
    return valor;
  while (1) {
    if (clave_lex == C_IZQ) {
      obt_lex();
      valor <<= expr_10();
    } else if (clave_lex == C_DER) {
      obt_lex();
      valor >>= expr_10();
    } else
      return valor;
  }
}

int expr_10(void)
{
  int valor;

  valor = expr_11();
  if (grupo_lex != GRUPO_5)
    return valor;
  while (1) {
    if (clave_lex == C_MAS) {
      obt_lex();
      valor += expr_11();
    } else if (clave_lex == C_MENOS) {
      obt_lex();
      valor -= expr_11();
    } else
      return valor;
  }
}

int expr_11(void)
{
  int valor;

  valor = expr_12();
  if (grupo_lex != GRUPO_6)
    return valor;
  while (1) {
    if (clave_lex == C_MUL) {
      obt_lex();
      valor *= expr_12();
    } else if (clave_lex == C_DIV) {
      obt_lex();
      valor /= expr_12();
    } else if (clave_lex == C_MOD) {
      obt_lex();
      valor %= expr_12();
    } else
      return valor;
  }
}

int expr_12(void)
{
  if (clave_lex == C_MENOS) {
    obt_lex();
    return -expr_13();
  }
  if (clave_lex == C_MAS) {
    obt_lex();
    return  expr_13();
  }
  if (clave_lex == C_NOT) {
    obt_lex();
    return !expr_13();
  }
  if (clave_lex == C_COM) {
    obt_lex();
    return ~expr_13();
  }
  return expr_13();
}

int expr_13(void)
{
  int p, valor = 0;

  if (clave_lex == C_PARENI) {
    obt_lex();
    valor = expr_preproc();
    if (clave_lex == C_PAREND)
      obt_lex();
    else
      error("Falta parentesis derecho");
  } else if (clave_lex == C_NUM) {
    valor = valor_lex;
    obt_lex();
  } else if (clave_lex == C_IDENT) {
    if (compara_cadenas("defined", cad_lex)) {
      obt_lex();
      if (clave_lex == C_PARENI) {
        obt_lex();
        p = 1;
      } else
        p = 0;
      if (clave_lex != C_IDENT && grupo_lex != GRUPO_0)
        error("Se requiere un nombre de macro");
      else {
        if (macro_especial(cad_lex))
          valor = 1;
        else if (busca_macro(cad_lex) != NULL)
          valor = 1;
        obt_lex();
      }
      if (p) {
        if (clave_lex == C_PAREND)
          obt_lex();
        else
          error("Falta parentesis derecho");
      }
    }
  } else {
    error("Expresión errónea");
  }
  return valor;
}

/*
** Detecta macros especiales de ANSI C
*/
int macro_especial(char *nombre)
{
  if (*nombre != '_' || *(nombre + 1) != '_')
    return 0;
  if (compara_cadenas("__DATE__", nombre))
    return 1;
  if (compara_cadenas("__FILE__", nombre))
    return 2;
  if (compara_cadenas("__LINE__", nombre))
    return 3;
  if (compara_cadenas("__STDC__", nombre))
    return 4;
  if (compara_cadenas("__TIME__", nombre))
    return 5;
  if (compara_cadenas("__cplusplus", nombre))
    return 6;
  return 0;
}

/*
** Anida un nuevo archivo de entrada
*/
void nuevo_archivo(char *archivo)
{
  struct archivo *nuevo_archivo;
  char *bloque;
  char *nombre = archivo;
  int control;

  nuevo_archivo = malloc(sizeof(struct archivo));
  if (nuevo_archivo == NULL) {
    error("No hay memoria");
    return;
  }
  control = fopen(archivo, "r");
  if (control == 0) {
    error("No se pudo abrir el archivo");
    return;
  }
  while (*nombre)
    nombre++;
  while (*--nombre != '/') ;
  nombre++;
  nuevo_archivo->archivo = control;
  nuevo_archivo->linea_actual = 0; /* Ninguna línea leida */
  nuevo_archivo->linea_real = 0;
  bloque = malloc(strlen(nombre) + 1);
  if (bloque != NULL) {
    nuevo_archivo->nombre_actual = bloque;
    strcpy(nuevo_archivo->nombre_actual, nombre);
  } else {
    error("No hay memoria");
    fclose(control);
    free(nuevo_archivo);
    return;
  }
  bloque = malloc(strlen(archivo) + 1);
  if (bloque != NULL) {
    nuevo_archivo->nombre_real = bloque;
    strcpy(nuevo_archivo->nombre_real, archivo);
  } else {
    error("No hay memoria");
    fclose(control);
    free(nuevo_archivo->nombre_actual);
    free(nuevo_archivo);
    return;
  }
  nuevo_archivo->anterior = archivo_actual;
  archivo_actual = nuevo_archivo;
  entrada = archivo_actual->archivo;
  pos_linea = linea;
  *pos_linea = 0;
}

/*
** Procesa #include, abre el nuevo archivo.
*/
void p_include(void)
{
  char *rastreo, *comienzo;
  char *camino, *ap;
  int estatus;

  espacios();           /* Salta los espacios */
  rastreo = pos_linea;
  if (*rastreo == '<') {
    camino = camino_tarea;
    comienzo = ++rastreo;
    estatus = 1;
  } else if (*rastreo == '"') {
    camino = nombre_archivo;
    comienzo = ++rastreo;
    estatus = 2;
  } else {
    estatus = 0;
    comienzo = rastreo;
    error("Error de sintaxis");
  }
  while (*rastreo != '>' && *rastreo != '"' && *rastreo)
    ++rastreo;
  if (*rastreo == '>' && estatus == 1) ;
  else if (*rastreo == '"' && estatus == 2) ;
  else {
    if (estatus != 0)
      error("Falta > o \" al final");
    return;
  }
  *rastreo = 0;
  if (*(comienzo + 1) == ':' && *(comienzo + 2) == '/') {
    nuevo_archivo(comienzo);
  } else {
    ap = linea_m;
    while (*ap++ = *camino++) ;
    if (*comienzo == '/')
      ap = linea_m + 2;
    else {
      while (*--ap != '/') ;
      ap++;
    }
    while (*ap++ = *comienzo++) ;
    nuevo_archivo(linea_m);
  }
  envia_mensaje(ventana, 2, 0x00330000, 0x006fffff);
}

/*
** Fin de un archivo #include
*/
void fin_include(void)
{
  struct archivo *temp;

  fclose(entrada);
  temp = archivo_actual->anterior;
  free(archivo_actual->nombre_real);
  free(archivo_actual->nombre_actual);
  free(archivo_actual);
  archivo_actual = temp;
  if (archivo_actual == NULL)
    entrada = -1;
  else
    entrada = archivo_actual->archivo;
  envia_mensaje(ventana, 2, 0x00330000, 0x006FFFFF);
}

/*
** Procesa la directiva #line
*/
void p_line(void)
{
  char *nombre;

  obt_lex();
  if (clave_lex == C_NUM) {
    archivo_actual->linea_actual = valor_lex - 1;
    obt_lex();
    if (clave_lex == C_NULO)
      return;
    if (clave_lex != C_CAD) {
      error("Debe ser cadena de texto");
      return;
    }
    nombre = malloc(strlen(cad_lex) + 1);
    if (nombre == NULL) {
      error("No hay memoria");
      return;
    }
    strcpy(nombre, cad_lex);
    free(archivo_actual->nombre_actual);
    archivo_actual->nombre_actual = nombre;
  }
}

/*
** Primer paso del preprocesamiento, pega líneas terminadas en \, y
** elimina los comentarios.
*/
void primer_paso(void)
{
  int car;

  lee_linea(linea);
  pos_linea_m = linea_m;
  if (eof)
    return;
  while (car = *pos_linea++) {
    if (car == ' ')
      pp_espacios();
    else if (car == '"')
      pp_comillas();
    else if (car == '\'')
      pp_apostrofe();
    else if (car == '/' && *pos_linea == '/')
      break;
    else if (car == '/' && *pos_linea == '*') {
      if (pos_linea_m != linea_m && *(pos_linea_m - 1) != ' ')
        almacena_car(' ');
      pp_comentarios();
    } else if (car == '\\' && *pos_linea == 0) {
      lee_linea(linea);
      if (eof)
        return;
    } else
      almacena_car(car);
  }
  if (pos_linea_m != linea_m && *(pos_linea_m - 1) == ' ')
    pos_linea_m--;
  almacena_car(0);
  if ((pos_linea_m - linea_m) >= MAX_LINEA)
    error("Línea muy larga");
  pos_linea = linea;
  pos_linea_m = linea_m;
  while (*pos_linea++ = *pos_linea_m++) ;
  pos_linea = linea;
}

void almacena_car(int c)
{
  *pos_linea_m = c;
  if ((pos_linea_m - linea_m) < MAX_LINEA)
    pos_linea_m++;
}

/*
** Elimina espacios.
*/
void pp_espacios(void)
{
  if (pos_linea_m != linea_m && *(pos_linea_m - 1) != ' ')
    almacena_car(' ');
  while (*pos_linea == ' ')
    pos_linea++;
}

/*
** Procesa cadenas de caracteres.
*/
void pp_comillas(void)
{
  almacena_car('"');
  while (*pos_linea != '"' ||
        *(pos_linea - 1) == 92 && *(pos_linea - 2) != 92) {
    if (*pos_linea == 0) {
      error("Faltan comillas");
      break;
    }
    almacena_car(obt_car());
  }
  almacena_car(obt_car());
}

/*
** Procesa caracteres encerrados entre '
*/
void pp_apostrofe(void)
{
  almacena_car('\'');
  while (*pos_linea != '\'' ||
        *(pos_linea - 1) == 92 && *(pos_linea - 2) != 92) {
    if (*pos_linea == 0) {
      error("Falta un apostrofe");
      break;
    }
    almacena_car(obt_car());
  }
  almacena_car(obt_car());
}

/*
** Procesa y elimina comentarios.
*/
void pp_comentarios(void)
{
  pos_linea++;
  while (*pos_linea != '*' || *(pos_linea + 1) != '/') {
    if (*pos_linea == 0)
      lee_linea(linea);
    else
      ++pos_linea;
    if (eof)
      break;
  }
  pos_linea += 2;
}

/*
** Añade una nueva macro en la tabla.
*/
void nueva_macro(void)
{
  struct macro *macro, *macro_vieja;
  int k;
  int num_pars;
  int l, base;        /* Indice en la tabla de argumentos de macros */
  int numero, concuerda;
  char *busqueda;

  /*
  ** Busca un nombre legal
  */
  obt_lex();
  if (clave_lex != C_IDENT && grupo_lex != GRUPO_0) {
    error("No es un nombre legal");
    return;
  }
  if (macro_especial(cad_lex)) {
    error("No es posible redefinir una macro especial");
    return;
  }

  /*
  ** Analiza la lista de parametros y guarda el nombre de cada uno.
  */
  l = 0;
  num_pars = -1;
  if (*pos_linea == '(') {   /* Genera una lista de nombres de parametros */
    num_pars = 0;
    obt_car();
    while (*pos_linea != ')') {
      espacios();
      if (isalpha(*pos_linea)) {
        while (isalnum(*pos_linea)) {
          if (l < MAX_AMAC - 1)
            amacs[l++] = obt_car();
          else {
            error("Macro muy complicada");
            cancela();
          }
        }
        amacs[l++] = 0;
        num_pars++;
      }
      espacios();
      if (*pos_linea == ',')
        obt_car();
      else if (*pos_linea != ')') {
        error("Falta ) en #define");
        break;
      }
    }
    obt_car();
    amacs[l++] = 0;
  }
  espacios();

  /*
  ** Ahora substituye los nombres en la macro por secuencias 0x7f Núm.
  ** También efectua concatenación (##).
  */
  base = l;
  while (*pos_linea) {
    if (num_pars > 0 && isalpha(*pos_linea)) {
      numero = 1;
      busqueda = amacs;
      while (*busqueda) {         /* Rastrea cada nombre */
        k = 0;
        while (1) {
          if (busqueda[k] != *(pos_linea + k))
            break;
          if (*(pos_linea + k) < ' ')
            break;
          k++;
        }
        if (isalnum(busqueda[k]) || isalnum(*(pos_linea + k))) {
          ++numero;
          while (*busqueda++) ;
        } else {
          pos_linea += k;
          if (l < MAX_AMAC - 1) {
            amacs[l++] = 127;
            amacs[l++] = numero;
          } else {
            error("Macro muy complicada");
            cancela();
          }
          break;
        }
      }
      if (*busqueda == 0) {
        while (isalnum(*pos_linea)) {
          if (l < MAX_AMAC - 1)
            amacs[l++] = obt_car();
          else {
            error("Macro muy complicada");
            cancela();
          }
        }
      }
    } else if (*pos_linea == '"') {
      amacs[l++] = obt_car();
      while (*pos_linea && (*pos_linea != '"'
          || *(pos_linea - 1) == '\\'
          && *(pos_linea - 2) != '\\')) {
        if (l < MAX_AMAC - 1)
          amacs[l++] = obt_car();
        else {
          error("Macro muy complicada");
          cancela();
        }
      }
      amacs[l++] = obt_car();
    } else if (*pos_linea == '\'') {
      amacs[l++] = obt_car();
      while (*pos_linea && (*pos_linea != '\''
          || *(pos_linea - 1) == '\\'
          && *(pos_linea - 2) != '\\')) {
        if (l < MAX_AMAC - 1)
          amacs[l++] = obt_car();
        else {
          error("Macro muy complicada");
          cancela();
        }
      }
      amacs[l++] = obt_car();
    } else if (*pos_linea == '#' && *(pos_linea + 1) == '#') {
      while (l && amacs[l - 1] == ' ')
        l--;
      pos_linea += 2;
      while (*pos_linea == ' ')
        pos_linea++;
    } else {
      if (l < MAX_AMAC - 1)
        amacs[l++] = obt_car();
      else {
        error("Macro muy complicada");
        cancela();
      }
    }
  }
  amacs[l++] = 0;

  /*
  ** Siguiendo a ANSI C, si se encuentra una macro con el mismo nombre debe
  ** ser igual o de lo contrario es un error.
  */
  macro_vieja = busca_macro(cad_lex);
  if (macro_vieja != NULL) {
    concuerda = compara_cadenas(macro_vieja->definicion, amacs + base);
    if (macro_vieja->parametros != num_pars)
      concuerda = NO;
    if (!concuerda)
      error("Macro redefinida");
    return;
  }

  /*
  ** Finalmente la nueva y reluciente macro es agregada a la tabla de dispersión
  */
  macro = malloc(sizeof(struct macro) + strlen(cad_lex));
  if (macro == NULL) {
    error("No hay memoria");
    return;
  }
  strcpy(macro->nombre, cad_lex);
  macro->parametros = num_pars;
  k = calcula_dispersion(cad_lex);
  macro->sig = macros[k];
  macros[k] = macro;
  macro->definicion = NULL;
  busqueda = malloc(l - base);
  if (busqueda == NULL) {
    error("No hay memoria");
    cancela();
  }
  memcpy(busqueda, amacs + base, l - base);
  macro->definicion = busqueda;
}

/*
** Elimina una macro de la tabla.
*/
void borra_macro(char *nombre)
{
  struct macro *macro, **anterior;

  anterior = &macros[calcula_dispersion(nombre)];
  macro = *anterior;
  while (macro != NULL) {
    if (compara_cadenas(nombre, macro->nombre)) {
      *anterior = macro->sig;
      free(macro->definicion);
      free(macro);
      return;
    }
    anterior = &macro->sig;
    macro = macro->sig;
  }
}

/*
** Busca una macro en la tabla.
*/
struct macro *busca_macro(char *nombre)
{
  struct macro *macro;

  macro = macros[calcula_dispersion(nombre)];
  while (macro != NULL) {
    if (compara_cadenas(nombre, macro->nombre))
      return macro;
    macro = macro->sig;
  }
  return NULL;
}

/*
** Desvia la salida a la consola.
*/
void hacia_consola(void)
{
  desvio_salida = salida;
  salida = 0;
}

/*
** Regresa la salida al archivo.
*/
void hacia_archivo(void)
{
  if (desvio_salida)
    salida = desvio_salida;
  desvio_salida = 0;
}

/*
** Vacia el buffer
*/
void vacia_buffer(void)
{
  int a, b;

  a = total_lineas;
  for (b = 0; b < MAX_LIN; b++)
    emite_texto("\2\n");
  buffer_vacio = SI;
}

/*
** Manda un caracter a la salida.
**
** Efectua algunas optimizaciones también.
*/
int emite_car(int c)
{
  char *ap, *ap1;
  int val0, val1, val2;

  if (c == 0)
    return 0;
  if (salida) {
    *ap_buf_retrasado++ = c;
    if (c == '\n') {
      buffer_vacio = NO;
      *ap_buf_retrasado++ = 0;
      val0 = (total_lineas - 2) & (MAX_LIN - 1);
      val1 = (total_lineas - 3) & (MAX_LIN - 1);
      val2 = (total_lineas - 4) & (MAX_LIN - 1);
      if (((estado_buf[val0] >= 1 && estado_buf[val0] <= 4)
        || estado_buf[val0] == 7)
       && ((estado_buf[val1] == 8)
        || (estado_buf[val1] == 11 && estado_buf[val2] == 8))) {
        if (estado_buf[val1] == 8)
          linea_inst[val1] = NULL;
        else
          linea_inst[val2] = NULL;
      }
      if (estado_buf[val0] == 10 && estado_buf[val1] == 8
      && (estado_buf[val2] & ~1) == 12) {
        linea_inst[val1] = NULL;
        val0 = *(ap = linea_inst[val0] + 3) == 'i';
        ap1 = linea_inst[val2] + 3;
        *ap++ = *ap1++ ^ 0x12;
        if (val0)
          *ap++ = 'i';
        while ((*ap++ = *ap1++) != ',');
      }
      total_lineas = (total_lineas + 1) & (MAX_LIN - 1);
      if ((ap = linea_inst[total_lineas]) != NULL) {
        ap1 = ap;
        while (*ap && *ap != '\2') {
          if (*ap == '\1')
            ap++;
          else
            *ap1++ = *ap++;
        }
        ap = linea_inst[total_lineas];
        if (fwrite(salida, ap, ap1 - ap) != ap1 - ap) {
          cierra_salida();
          error("Error al escribir");
          cancela();
        }
      }
      if (ap_buf_retrasado - buf_retrasado >= (MAX_BUFR - 64))
        ap_buf_retrasado = buf_retrasado;
      linea_inst[total_lineas] = ap_buf_retrasado;
      estado_buf[total_lineas] = 0;
    }
  } else
    ;
  return c;
}

/*
** Cambio de linea a la salida.
*/
void emite_nueva_linea(void)
{
  emite_car('\n');
}

/*
** Manda una línea a la salida, hace un cambio de línea también.
*/
void emite_linea(char *ap)
{
  emite_texto(ap);
  emite_nueva_linea();
}

/*
** Manda un texto a la salida.
*/
void emite_texto(char *ap)
{
  while (emite_car(*ap++));
}

/*
** Saca un número decimal en la salida.
*/
void emite_numero(int numero)
{
  if (numero < 0) {
    emite_car('-');
    if (numero < -9)
      emite_numero(-(numero / 10));
    emite_car(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      emite_numero(numero / 10);
    emite_car((numero % 10) + '0');
  }
}

/*
** Ilustra los mensajes de error.
error(ap)
  unsigned char ap[];
{
  int k;
  unsigned char entrada[81];

  hacia_consola();
  emite_texto("Línea ");
  emite_numero(linea_actual);
  emite_texto(", ");
  if (!dentro_funcion)
    emite_car('(');
  if (funcion_actual == NULL)
    emite_texto("comienzo del archivo");
  else
    emite_texto(funcion_actual + NOMBRE);
  if (!dentro_funcion)
    emite_car(')');
  emite_texto(" + ");
  emite_numero(linea_actual - comienzo_funcion);
  emite_texto(": ");
  color(15);
  emite_texto(ap);
  emite_nueva_linea();

  color(14);
  emite_texto(linea);
  emite_nueva_linea();

  k = 0;
  while (k < pos_linea) {
    if (linea[k++] == 9)
      emite_car(9);
    else
      emite_car(' ');
  }
  emite_car('^');
  emite_nueva_linea();
  ++errores;

  hacia_archivo();
  if (pausa) {
    mensaje("¿ Continuar (Si, No, Pasar de largo) ? ");
    gets(entrada);
    k = entrada[0];
    if ((k == 'N') || (k == 'n'))
      cancela();
    if ((k == 'P') || (k == 'p'))
      pausa = NO;
  }
}
*/

/*
** Salta los espacios en la entrada.
*/
void espacios(void)
{
  while (1) {
    while (*pos_linea == 0) {
      if (dentro_pp)
        return;
      preprocesa();
      if (eof)
        break;
    }
    if (*pos_linea == ' ')
      obt_car();
    else
      return;
  }
}

/*
** Analizador léxico (compatible con C++)
*/
char *palabras_reservadas[] = {
  "asm", "auto", "break", "case",
  "catch", "char", "class", "const",
  "continue", "default", "delete", "do",
  "double", "else", "enum", "extern",
  "float", "for", "friend", "goto",
  "if", "inline", "int", "long",
  "new", "operator", "private", "protected",
  "public", "register", "return", "short",
  "signed", "sizeof", "static", "struct",
  "switch", "template", "this", "throw",
  "try", "typedef", "union", "unsigned",
  "virtual", "void", "volatile", "while",
  "z"};

int lista_reservadas[] = {
  0, 2, 3, 9, 13, 16, 19, -1,
  20, -1, -1, 23, -1, 24, 25, 26,
  -1, 29, 31, 37, 42, 44, 47, -1,
  -1, -1};

/*
** Obtiene un componente léxico.
*/
void obt_lex(void)
{
  int c, sc;

  grupo_lex = NINGUNO;
  clave_lex = C_ERROR;
  while (1) {
    while (*pos_linea == ' ')
      pos_linea++;
    if (*pos_linea == 0 && !dentro_pp)
      preprocesa();
    else
      break;
    if (eof)
      return;
  }
  c = *pos_linea;
  sc = *(pos_linea + 1);
  if (c == 'L' || c == 'l') {
    if (sc == '"') {
      pos_linea += 2;
      p_cadena(1);
      return;
    } else if (sc == '\'') {
      pos_linea += 2;
      p_caracter(1);
      return;
    }
  }
  if (isalpha(c)) {
    p_ident();
    return;
  }
  if (isdigit(c)) {
    p_numero();
    return;
  }
  if (c)
    pos_linea++;
  switch (c) {
    case '"':
      p_cadena(0);
      break;
    case '\'':
      p_caracter(0);
      break;
    case ';':
      clave_lex = C_PCOMA;
      break;
    case '(':
      clave_lex = C_PARENI;
      break;
    case ')':
      clave_lex = C_PAREND;
      break;
    case '[':
      clave_lex = C_CORCHI;
      break;
    case ']':
      clave_lex = C_CORCHD;
      break;
    case '{':
      clave_lex = C_LLAVEI;
      break;
    case '}':
      clave_lex = C_LLAVED;
      break;
    case '?':
      clave_lex = C_TRINARIO;
      break;
    case '~':
      clave_lex = C_COM;
      break;
    case ',':
      clave_lex = C_COMA;
      break;
    case '!':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_NOIGUAL;
        grupo_lex = GRUPO_2;
      } else {
        clave_lex = C_NOT;
      }
      break;
    case '%':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_MODIGUAL;
        grupo_lex = GRUPO_1;
      } else {
        clave_lex = C_MOD;
        grupo_lex = GRUPO_6;
      }
      break;
    case '^':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_XORIGUAL;
        grupo_lex = GRUPO_1;
      } else {
        clave_lex = C_XOR;
      }
      break;
    case '&':
      if (sc == '&') {
        pos_linea++;
        clave_lex = C_ANDAND;
      } else if (sc == '=') {
        pos_linea++;
        clave_lex = C_ANDIGUAL;
        grupo_lex = GRUPO_1;
      } else {
        clave_lex = C_AND;
      }
      break;
    case '*':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_PORIGUAL;
        grupo_lex = GRUPO_1;
      } else {
        clave_lex = C_MUL;
        grupo_lex = GRUPO_6;
      }
      break;
    case '-':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_MENOSIGUAL;
        grupo_lex = GRUPO_1;
      } else if (sc == '-') {
        pos_linea++;
        clave_lex = C_DEC;
      } else if (sc == '>') {
        pos_linea++;
        if (*pos_linea == '*') {
          pos_linea++;
          clave_lex = C_APMIEMBRO;
          grupo_lex = GRUPO_7;
        } else {
          clave_lex = C_APUNTA;
        }
      } else {
        clave_lex = C_MENOS;
        grupo_lex = GRUPO_5;
      }
      break;
    case '+':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_MASIGUAL;
        grupo_lex = GRUPO_1;
      } else if (sc == '+') {
        pos_linea++;
        clave_lex = C_INC;
      } else {
        clave_lex = C_MAS;
        grupo_lex = GRUPO_5;
      }
      break;
    case '=':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_IGUALIGUAL;
        grupo_lex = GRUPO_2;
      } else {
        clave_lex = C_IGUAL;
        grupo_lex = GRUPO_1;
      }
      break;
    case '|':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_ORIGUAL;
        grupo_lex = GRUPO_1;
      } else if (sc == '|') {
        pos_linea++;
        clave_lex = C_OROR;
      } else {
        clave_lex = C_OR;
      }
      break;
    case ':':
      if (sc == ':') {
        pos_linea++;
        clave_lex = C_ALCANCE;
      } else {
        clave_lex = C_DPUNTOS;
      }
      break;
    case '/':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_DIVIGUAL;
        grupo_lex = GRUPO_1;
      } else {
        clave_lex = C_DIV;
        grupo_lex = GRUPO_6;
      }
      break;
    case '>':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_MAYORIGUAL;
        grupo_lex = GRUPO_3;
      } else if (sc == '>') {
        pos_linea++;
        if (*pos_linea == '=') {
          pos_linea++;
          clave_lex = C_DERIGUAL;
          grupo_lex = GRUPO_1;
        } else {
          clave_lex = C_DER;
          grupo_lex = GRUPO_4;
        }
      } else {
        clave_lex = C_MAYOR;
        grupo_lex = GRUPO_3;
      }
      break;
    case '<':
      if (sc == '=') {
        pos_linea++;
        clave_lex = C_MENORIGUAL;
        grupo_lex = GRUPO_3;
      } else if (sc == '<') {
        pos_linea++;
        if (*pos_linea == '=') {
          pos_linea++;
          clave_lex = C_IZQIGUAL;
          grupo_lex = GRUPO_1;
        } else {
          clave_lex = C_IZQ;
          grupo_lex = GRUPO_4;
        }
      } else {
        clave_lex = C_MENOR;
        grupo_lex = GRUPO_3;
      }
      break;
    case '.':
      if (isdigit(sc)) {
        p_numero();
        return;
      }
      if (sc == '*') {
        pos_linea++;
        clave_lex = C_MIEMBRO;
        grupo_lex = GRUPO_7;
      } else if (sc == '.' && *(pos_linea + 1) == '.') {
        pos_linea += 2;
        clave_lex = C_PUNTOS;
      } else {
        clave_lex = C_PUNTO;
      }
      break;
    case '#':
      if (sc == '#') {
        pos_linea++;
        clave_lex = C_PREPROC2;
      } else {
        clave_lex = C_PREPROC1;
      }
      break;
    case '\0':
      clave_lex = C_NULO;
      break;
  }
}

/*
** Procesa un identificador y averigua si es una palabra reservada.
*/
void p_ident(void)
{
  char *ap1, *ap = cad_lex;
  int c, p;

  clave_lex = C_IDENT;
  while (isalnum(*pos_linea)) {
    if (ap < cad_lex + (TAM_NOMBRE - 1))
      *ap++ = *pos_linea;
    pos_linea++;
  }
  *ap = 0;
  if (*cad_lex >= 'a' && *cad_lex <= 'z') {
    p = lista_reservadas[*cad_lex - 'a'];
    if (p >= 0) {
      while (1) {
        ap = palabras_reservadas[p++];
        if (*cad_lex != *ap)
          break;
        ap1 = cad_lex;
        while (*ap && *ap == *ap1) {
          ap++;
          ap1++;
        }
        if (*ap == *ap1) {
          grupo_lex = GRUPO_0;
          clave_lex = p - 1;
          break;
        }
      }
    }
  }
}

void p_caracter(int tipo)
{
  valor_lex = 0;
  clave_lex = C_NUM;
  while (*pos_linea && *pos_linea != '\'')
    valor_lex = (valor_lex << (tipo ? 16 : 8)) + caracter_literal();
  pos_linea++;
  if ((unsigned) valor_lex > 0x7FFFFFFF)
    tipo_lex = t_uint;
  else
    tipo_lex = t_sint;
}

void p_cadena(int tipo)
{
  int ap;

  if (tipo)
    ap_lit = (ap_lit + 1) & ~1;
  ap = ap_lit;
  clave_lex = C_CAD;
  valor_lex = 0;
  if (tipo)
    tipo_lex = t_awchar;
  else
    tipo_lex = t_achar;
  while (1) {
    while (*pos_linea && *pos_linea != '"') {
      if (ap < TAM_LITS) {
        lits[ap++] = caracter_literal();
        if (tipo)
          lits[ap++] = 0;
      }
    }
    pos_linea++;
    while (1) {
      while (*pos_linea == ' ')
        pos_linea++;
      if (*pos_linea == 0 && !dentro_pp)
        preprocesa();
      else
        break;
    }
    if (*pos_linea != '"')
      break;
    pos_linea++;
  }
  if (ap < TAM_LITS) {
    lits[ap++] = 0;
    if (tipo)
      lits[ap++] = 0;
  } else
    error("Demasiadas cadenas de texto");
  valor_lex = ap - ap_lit;
}

void p_numero(void)
{
  char *ap = pos_linea;      /* Salva la posición actual */
  int negativo, exponente, sin_signo, doble;
  double escala, fraccion;

  valor_lex = 0;
  clave_lex = C_NUM;
  tipo_lex = t_sint;
  while (isdigit(*pos_linea))
    pos_linea++;
  if (*pos_linea == '.') {   /* Número de punto flotante detectado */
    pos_linea = ap;          /* Restaura la posición */
    while (isdigit(*pos_linea))
      valor_lex = valor_lex * 10 + (*pos_linea++ - '0');
    pos_linea++;             /* Evade el punto */
    fraccion = 0.0;
    escala = 1.0;
    while (isdigit(*pos_linea)) {
      escala *= 10.0;
      fraccion += (*pos_linea++ - '0') / escala;
    }
    fraccion += valor_lex;
    if (*pos_linea == 'E' || *pos_linea == 'e') {  /* Procesa el exponente */
      pos_linea++;
      negativo = exponente = 0;
      if (*pos_linea == '+')
        pos_linea++;
      else if (*pos_linea == '-') {
        negativo = 1;
        pos_linea++;
      }
      while (isdigit(*pos_linea))
        exponente = exponente * 10 + (*pos_linea++ - '0');
      while (exponente--) {
        if (negativo)
          fraccion /= 10.0;
        else
          fraccion *= 10.0;
      }
    }
    clave_lex = C_NUMF;
    tipo_lex = t_double;
    if (*pos_linea == 'F' || *pos_linea == 'f') {
      pos_linea++;
    } else if (*pos_linea == 'L' || *pos_linea == 'l') {  /* Long Double, ignora */
      pos_linea++;
    }
    ap_lit = (ap_lit + 3) & ~3;
    valor_lex = ap_lit;
    *((double *) (lits + ap_lit)) = fraccion;
  } else {
    pos_linea = ap;          /* Restaura la posición */
    if (*pos_linea == '0') { /* Octal */
      pos_linea++;
      if (*pos_linea == 'X' || *pos_linea == 'x') {
        pos_linea++;
        while (isxdigit(*pos_linea)) {
          if (*pos_linea <= '9')
            valor_lex = valor_lex * 16 + (*pos_linea++ - '0');
          else
            valor_lex = valor_lex * 16 + ((*pos_linea++ & 7) + 9);
        }
      } else {
        while (*pos_linea >= '0' && *pos_linea <= '7')
          valor_lex = valor_lex * 8 + (*pos_linea++ - '0');
      }
    } else {
      while (*pos_linea >= '0' && *pos_linea <= '9')
        valor_lex = valor_lex * 10 + (*pos_linea++ - '0');
    }
    sin_signo = doble = NO;
    while (1) {
      if (*pos_linea == 'U' || *pos_linea == 'u') {
        if (sin_signo)
          error("Demasiadas U");
        sin_signo = SI;
        pos_linea++;
        tipo_lex = t_uint;
      } else if (*pos_linea == 'L' || *pos_linea == 'l') {
        if (doble)
          error("Demasiadas L");
        doble = SI;
        pos_linea++;
      } else
        break;
    }
    if ((unsigned) valor_lex > 0x7FFFFFFF)
      tipo_lex = t_uint;
  }
}

int caracter_literal(void)
{
  int i, oct;

  if (*pos_linea != '\\')
    return *pos_linea++;
  switch (*++pos_linea) {
    case '"':
    case '\'':
    case '?':
    case '\\':
      return *pos_linea++;
    case 'a':    /* BEL, campanita */
      pos_linea++;
      return 7;
    case 'b':    /* BS, retroceso */
      pos_linea++;
      return 8;
    case 'f':    /* FF, cambio de página */
      pos_linea++;
      return 12;
    case 'n':    /* LF, cambio de línea */
      pos_linea++;
      return 10;
    case 'r':    /* CR, retorno de carro */
      pos_linea++;
      return 13;
    case 't':    /* HT, tabulador horizontal */
      pos_linea++;
      return 9;
    case 'v':    /* VT, tabulador vertical */
      pos_linea++;
      return 11;
    case 'x':
      pos_linea++;
      if (!isxdigit(*pos_linea))
        error("Falta constante hexadecimal");
      oct = 0;
      while (isxdigit(*pos_linea)) {
        if (*pos_linea > '9')
          oct = oct * 16 + ((*pos_linea++ & 7) + 9);
        else
          oct = oct * 16 + (*pos_linea++ - '0');
      }
      return oct;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      i = oct = 0;
      while (i++ < 3 && *pos_linea >= '0' && *pos_linea <= '7')
        oct = (oct << 3) + (*pos_linea++ - '0');
      return oct;
    default:
      error("Secuencia de escape inválida");
      return 0;
  }
}
