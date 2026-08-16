/*
** Compilador de C para G11.
** Evaluador de Expresiones.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Copyright Oscar Toledo G.1995-1998.
**
** Creación: 04-jun-1995.
** Revisión: 05-sep-1998. Rediseño para apoyar el nuevo analizador léxico.
** Revisión: 07-sep-1998. Rediseño para optimizar subexpresiones comunes.
** Revisión: 12-sep-1998. Soporte para signed char y unsigned char.
** Revisión: 12-oct-1998. Soporte para tipos dinámicos y prototipos.
** Revisión: 13-oct-1998. Avisa cuando se convierte implicitamente un apuntador
**                        a tipo aritmético o viceversa. Tratamiento ANSI de
**                        apuntadores a funciones. Ahora es estricto en el
**                        tratamiento de los apuntadores (ANSI).
** Revisión: 14-oct-1998. Mejoras en comprobación de tipos. Corrección de
**                        detalles.
** Revisión: 06-nov-1998. Se elimina una comprobación incorrecta de clase_alm
**                        en coerción de tipo.
** Revisión: 14-nov-1998. Aceleración del análisis de expresiones, evitando
**                        descensos recursivos inútiles, también ocupa menos
**                        pila.
** Revisión: 09-dic-1998. Corrección de defecto en generación de llamada a
**                        función (no hallaba nunca struct).
** Revisión: 12-dic-1998. Nueva acumulación de llamadas largas para ahorrar
**                        espacio.
*/

/*
** !!! Debería optimizar según ANSI para solo float, algunos programas
**     se harían más rápidos y otros más cortos.
** !!! Debe poder cálcular expresiones constantes de punto flotante,
**     para eso se crearía una nueva función como expr_constante()
**     que además retornaría el tipo.
** !!! No procesa const y volatile. (genera errores)
*/

/*
** Esta estructura contiene información sobre la expresión
** que se esta analizando.
*/
struct expr {
  struct tipo *tipo;    /* Tipo actual de la expresión */
};

/*
** Procesa una expresión constante.
*/
int expr_constante(void)
{
  struct nodo *origen;
  int valor;

  origen = ultimo_nodo;
  checa_entero(almacena_expresion(NO));
  if (ultimo_nodo->oper != N_CONST) {
    ultimo_nodo = origen;
    error("No es una expresión constante");
    return 1;
  } else {
    valor = ultimo_nodo->esp;
    ultimo_nodo = origen;
    return valor;
  }
}

/*
** Analiza una expresión y la mantiene en memoria.
*/
struct tipo *almacena_expresion(int operador_coma)
{
  struct expr info;

  if (operador_coma) {
    if (nivel13(&info, 0))
      carga_valor(&info);
  } else {
    if (nivel13(&info, 1))
      carga_valor(&info);
  }
  return info.tipo;
}

int nivel0(struct expr *info, int k)
{
  struct nodo *izq;

  do {
    obt_lex();
    if (k)
      carga_valor(info);
    izq = ultimo_nodo;
    k = nivel13(info, 1);
    crea_nodo(N_COMA, izq, ultimo_nodo, 0);
  } while (clave_lex == C_COMA);
  return k;
}

int nivel1(struct expr *info, int k)
{
  struct expr info2;
  int op;
  struct nodo *der, *izq;
  struct tipo *tipo, *tipo2, *temp;

  if (clave_lex == C_IGUAL)
    op = N_ASIGNA;
  else if (clave_lex == C_ORIGUAL)
    op = N_AOR;
  else if (clave_lex == C_XORIGUAL)
    op = N_AXOR;
  else if (clave_lex == C_ANDIGUAL)
    op = N_AAND;
  else if (clave_lex == C_IZQIGUAL)
    op = N_ACI;
  else if (clave_lex == C_DERIGUAL)
    op = N_ACD;
  else if (clave_lex == C_MASIGUAL)
    op = N_ASUMA;
  else if (clave_lex == C_MENOSIGUAL)
    op = N_ARESTA;
  else if (clave_lex == C_PORIGUAL)
    op = N_AMUL;
  else if (clave_lex == C_DIVIGUAL)
    op = N_ADIV;
  else if (clave_lex == C_MODIGUAL)
    op = N_AMOD;
  obt_lex();
  der = ultimo_nodo;
  tipo = info->tipo;
  if (k == 0)
    if (tipo->tipo != STRUCT)
      req_valorl();
  if (nivel13(&info2, 1)) {
    if (tipo->tipo == STRUCT)
      req_valorl();
    carga_valor(&info2);
    izq = ultimo_nodo;
  } else if (tipo->tipo == STRUCT) {
    tipo2 = info2.tipo;
    if (tipo2->tipo != STRUCT)
      error("Se requiere una estructura o unión");
    else if (tipo->especial.est != tipo2->especial.est)
      error("Estructuras incompatibles");
    if (op != N_ASIGNA)
      error("Asignación incompatible");
    crea_nodo(N_COPIA, der, ultimo_nodo, (tam_tipo(tipo) + 3) / 4);
    return 0;
  } else
    izq = ultimo_nodo;
  tipo2 = info2.tipo;
  if (tipo2->tipo == FUNCION) {
    temp = crea_tipo(APUNTADOR);
    temp->sig = tipo2;
    tipo2 = temp;
  }
  if ((tipo->tipo == DOUBLE || tipo->tipo == FLOAT)
    && (op == N_AOR || op == N_AXOR
     || op == N_AAND || op == N_ACI
     || op == N_ACD || op == N_AMOD))
    error("No se puede efectuar esta operación con reales");
  if ((op == N_ASUMA || op == N_ARESTA) && (k = dobla(tipo, tipo2, &izq))) {
    if (k == 2 && multi != 1) {
      crea_nodo(N_CONST, NULL, NULL, multi);
      crea_nodo(N_MUL, izq, ultimo_nodo, 0);
      izq = ultimo_nodo;
    }
  } else
    convierte_tipo(&izq, tipo2, tipo, SI);
  if (tipo->tipo == APUNTADOR)
    tipo = t_sint;
  crea_nodo(op, izq, der, tipo->tipo);
  return 0;
}

int nivel2(struct expr *info, int k)
{
  struct expr info2, info3;
  struct nodo *ext, *izq, *der;

  if (k)
    carga_valor(info);
  ext = ultimo_nodo;
  obt_lex();
  if (nivel13(&info2, 0))
    carga_valor(&info2);
  izq = ultimo_nodo;
  if (clave_lex == C_DPUNTOS)
    obt_lex();
  else
    error("Falta simbolo de dos puntos");
  if (nivel13(&info3, 2))
    carga_valor(&info3);
  der = ultimo_nodo;
  if ((ext->oper == N_CONST) && (izq->oper == N_CONST) &&
      (der->oper == N_CONST)) {
    crea_nodo(N_CONST, NULL, NULL, ext->esp ? izq->esp : der->esp);
  } else {
    haz_compatible(&izq, &info2, &der, &info3);
    crea_nodo(N_TRI, izq, der, (int) ext);
  }
  *info = info2;
  return 0;
}

int nivel3(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq;

  if (k)
    carga_valor(info);
  do {
    obt_lex();
    checa_numerico(info->tipo);
    compara_no_cero(info->tipo);
    izq = ultimo_nodo;
    if (nivel13(&info2, 4))
      carga_valor(&info2);
    checa_numerico(info2.tipo);
    compara_no_cero(info2.tipo);
    crea_nodo(N_ORB, izq, ultimo_nodo, 0);
  } while (clave_lex == C_OROR);
  return 0;
}

int nivel4(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq;

  if (k)
    carga_valor(info);
  do {
    obt_lex();
    checa_numerico(info->tipo);
    compara_no_cero(info->tipo);
    izq = ultimo_nodo;
    if (nivel13(&info2, 5))
      carga_valor(&info2);
    checa_numerico(info2.tipo);
    compara_no_cero(info2.tipo);
    crea_nodo(N_ANDB, izq, ultimo_nodo, 0);
  } while (clave_lex == C_ANDAND);
  return 0;
}

int nivel5(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;

  if (k)
    carga_valor(info);
  do {
    obt_lex();
    izq = ultimo_nodo;
    checa_entero(info->tipo);
    if (nivel13(&info2, 6))
      carga_valor(&info2);
    der = ultimo_nodo;
    checa_entero(info2.tipo);
    if (izq->oper == N_CONST && der->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp | der->esp);
    } else
      crea_nodo(N_OR, izq, der, 0);
  } while (clave_lex == C_OR);
  return 0;
}

int nivel6(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;

  if (k)
    carga_valor(info);
  do {
    obt_lex();
    izq = ultimo_nodo;
    checa_entero(info->tipo);
    if (nivel13(&info2, 7))
      carga_valor(&info2);
    der = ultimo_nodo;
    checa_entero(info2.tipo);
    if (izq->oper == N_CONST && der->oper == N_CONST)
      crea_nodo(N_CONST, NULL, NULL, izq->esp ^ der->esp);
    else
      crea_nodo(N_XOR, izq, der, 0);
  } while (clave_lex == C_XOR);
  return 0;
}

int nivel7(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;

  if (k)
    carga_valor(info);
  do {
    obt_lex();
    izq = ultimo_nodo;
    checa_entero(info->tipo);
    if (nivel13(&info2, 8))
      carga_valor(&info2);
    der = ultimo_nodo;
    checa_entero(info2.tipo);
    if (izq->oper == N_CONST && der->oper == N_CONST)
      crea_nodo(N_CONST, NULL, NULL, izq->esp & der->esp);
    else
      crea_nodo(N_AND, izq, der, 0);
  } while (clave_lex == C_AND);
  return 0;
}

int nivel8(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;

  if (k)
    carga_valor(info);
  do {
    izq = ultimo_nodo;
    if (clave_lex == C_IGUALIGUAL) {
      obt_lex();
      if (nivel13(&info2, 9))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2))
        crea_nodo(N_IGUALPF, izq, der, 0);
      else if ((izq->oper == N_CONST) && (der->oper == N_CONST))
        crea_nodo(N_CONST, NULL, NULL, izq->esp == der->esp);
      else {
        checa_comparable(info->tipo, info2.tipo);
        crea_nodo(N_IGUAL, izq, der, 0);
      }
    } else {
      obt_lex();
      if (nivel13(&info2, 9))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_IGUALPF, izq, der, 0);
        crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
      } else if ((izq->oper == N_CONST) && (der->oper == N_CONST))
        crea_nodo(N_CONST, NULL, NULL, izq->esp != der->esp);
      else {
        checa_comparable(info->tipo, info2.tipo);
        crea_nodo(N_NOIGUAL, izq, der, 0);
      }
    }
    info->tipo = t_sint;
  } while (grupo_lex == GRUPO_2);
  return 0;
}

int nivel9(struct expr *info, int k)
{
  if (k)
    carga_valor(info);
  do {
    if (clave_lex == C_MENORIGUAL) {
      obt_lex();
      nivel9eval(1, info);
    } else if (clave_lex == C_MAYORIGUAL) {
      obt_lex();
      nivel9eval(2, info);
    } else if (clave_lex == C_MENOR) {
      obt_lex();
      nivel9eval(3, info);
    } else {
      obt_lex();
      nivel9eval(4, info);
    }
  } while (grupo_lex == GRUPO_3);
  return 0;
}

int nivel9eval(int k, struct expr *info)
{
  struct expr info2;
  struct nodo *izq, *der;
  struct tipo *tipo;

  izq = ultimo_nodo;
  if (nivel13(&info2, 10))
    carga_valor(&info2);
  der = ultimo_nodo;
  if (haz_compatible(&izq, info, &der, &info2)) {
    if (k == 4)
      crea_nodo(N_MAYORPF, izq, der, 0);
    else if (k == 2)
      crea_nodo(N_MAYORIPF, izq, der, 0);
    else if (k == 3)
      crea_nodo(N_MAYORPF, der, izq, 0);
    else
      crea_nodo(N_MAYORIPF, der, izq, 0);
    info->tipo = t_sint;
    return;
  }
  checa_comparable(info->tipo, info2.tipo);
  tipo = info->tipo;
  if (tipo->tipo == UINT) {
    nivel9op(izq, k);
    info->tipo = t_sint;
    return;
  }
  tipo = info2.tipo;
  if (tipo->tipo == UINT) {
    nivel9op(izq, k);
    info->tipo = t_sint;
    return;
  }
  if (k == 4) {
    if (izq->oper == N_CONST && der->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp > der->esp);
    } else
      crea_nodo(N_MAYOR, izq, der, 0);
  } else if (k == 3) {
    if (izq->oper == N_CONST && der->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp < der->esp);
    } else
      crea_nodo(N_MENOR, izq, der, 0);
  } else if (k == 1) {
    if (izq->oper == N_CONST && der->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp <= der->esp);
    } else
      crea_nodo(N_MENORI, izq, der, 0);
  } else {
    if (izq->oper == N_CONST && der->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp >= der->esp);
    } else
      crea_nodo(N_MAYORI, izq, der, 0);
  }
  info->tipo = t_sint;
}

void nivel9op(struct nodo *izq, int k)
{
  if (k == 4)
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
  else if (k == 3)
    crea_nodo(N_SMENOR, izq, ultimo_nodo, 0);
  else if (k == 1)
    crea_nodo(N_SMENORI, izq, ultimo_nodo, 0);
  else
    crea_nodo(N_SMAYORI, izq, ultimo_nodo, 0);
}

int nivel10(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;
  struct tipo *tipo;

  if (k)
    carga_valor(info);
  do {
    izq = ultimo_nodo;
    if (clave_lex == C_DER) {
      obt_lex();
      checa_entero(info->tipo);
      if (nivel13(&info2, 11))
        carga_valor(&info2);
      der = ultimo_nodo;
      checa_entero(info2.tipo);
      if (izq->oper == N_CONST && der->oper == N_CONST) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp >> der->esp);
      } else {
        tipo = info->tipo;
        crea_nodo(N_CD, izq, der, tipo->tipo == UINT || tipo->tipo == USHORT);
      }
    } else {
      obt_lex();
      checa_entero(info->tipo);
      if (nivel13(&info2, 11))
        carga_valor(&info2);
      der = ultimo_nodo;
      checa_entero(info2.tipo);
      if (izq->oper == N_CONST && der->oper == N_CONST)
        crea_nodo(N_CONST, NULL, NULL, izq->esp << der->esp);
      else
        crea_nodo(N_CI, izq, der, 0);
    }
  } while (grupo_lex == GRUPO_4);
  return 0;
}

int nivel11(struct expr *info, int k)
{
  int tam;
  struct expr info2;
  struct nodo *izq, *der;
  struct tipo *tipo;

  if (k)
    carga_valor(info);
  do {
    izq = ultimo_nodo;
    if (clave_lex == C_MAS) {
      obt_lex();
      if (nivel13(&info2, 12))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_SUMAPF, izq, der, 0);
      } else {
        if (k = dobla(info->tipo, info2.tipo, &der)) {
          if (k == 2 && der->oper == N_CONST) {
            crea_nodo(N_CONST, NULL, NULL, der->esp * multi);
            der = ultimo_nodo;
          } else if (k == 2 && multi != 1) {
            crea_nodo(N_CONST, NULL, NULL, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        } else {
          *info = info2;
          if (k = dobla(info->tipo, info2.tipo, &izq)) {
            if (k == 2 && multi != 1) {
              crea_nodo(N_CONST, NULL, NULL, multi);
              crea_nodo(N_MUL, izq, ultimo_nodo, 0);
              izq = ultimo_nodo;
            }
          }
        }
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp + der->esp);
        } else
          crea_nodo(N_SUMA, izq, der, 0);
      }
    } else {
      obt_lex();
      if (nivel13(&info2, 12))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_RESTAPF, izq, der, 0);
      } else {
        tipo = info->tipo;
        if ((tipo->tipo == APUNTADOR || tipo->tipo == ARREGLO)
         && (info2.tipo->tipo == APUNTADOR || info2.tipo->tipo == ARREGLO)) {
          if (adv_ansi && compara_tipos(tipo->sig, info2.tipo->sig))
            error("Los apuntadores no son del mismo tipo");
          tipo = info2.tipo;
          crea_nodo(N_RESTA, izq, der, 0);
          tam = tam_tipo(tipo->sig);
          if (tam != 1) {
            izq = ultimo_nodo;
            crea_nodo(N_CONST, NULL, NULL, tam);
            crea_nodo(N_DIV, izq, ultimo_nodo, 0);
          }
          info->tipo = t_sint;  /* ptrdiff_t es signed int */
        } else {
          if (k = dobla(info->tipo, info2.tipo, &der)) {
            if (k == 2 && der->oper == N_CONST) {
              crea_nodo(N_CONST, NULL, NULL, der->esp * multi);
              der = ultimo_nodo;
            } else if (k == 2 && multi != 1) {
              crea_nodo(N_CONST, NULL, NULL, multi);
              crea_nodo(N_MUL, der, ultimo_nodo, 0);
              der = ultimo_nodo;
            }
          }
          if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
            crea_nodo(N_CONST, NULL, NULL, izq->esp - der->esp);
          } else {
            crea_nodo(N_RESTA, izq, der, 0);
          }
        }
      }
    }
  } while (grupo_lex == GRUPO_5);
  return 0;
}

int nivel12(struct expr *info, int k)
{
  struct expr info2;
  struct nodo *izq, *der;
  struct tipo *tipo, *tipo2;

  if (k)
    carga_valor(info);
  do {
    izq = ultimo_nodo;
    if (clave_lex == C_MUL) {
      obt_lex();
      if (nivel13(&info2, 13))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_MULPF, izq, der, 0);
      } else {
        checa_entero(info->tipo);
        checa_entero(info2.tipo);
        if (izq->oper == N_CONST && der->oper == N_CONST)
          crea_nodo(N_CONST, NULL, NULL, izq->esp * der->esp);
        else
          crea_nodo(N_MUL, izq, der, 0);
      }
    } else if (clave_lex == C_DIV) {
      obt_lex();
      if (nivel13(&info2, 13))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_DIVPF, izq, der, 0);
      } else {
        checa_entero(info->tipo);
        checa_entero(info2.tipo);
        if (izq->oper == N_CONST && der->oper == N_CONST) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp / der->esp);
        } else {
          tipo = info->tipo;
          tipo2 = info2.tipo;
          if (tipo->tipo == UINT || tipo2->tipo == UINT)
            crea_nodo(N_SDIV, izq, der, 0);
          else
            crea_nodo(N_DIV, izq, der, 0);
        }
      }
    } else {
      obt_lex();
      checa_entero(info->tipo);
      if (nivel13(&info2, 13))
        carga_valor(&info2);
      checa_entero(info2.tipo);
      der = ultimo_nodo;
      if (izq->oper == N_CONST && der->oper == N_CONST) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp % der->esp);
      } else {
        tipo = info->tipo;
        tipo2 = info2.tipo;
        if (tipo->tipo == UINT || tipo2->tipo == UINT)
          crea_nodo(N_SMOD, izq, der, 0);
        else
          crea_nodo(N_MOD, izq, der, 0);
      }
    }
  } while (grupo_lex == GRUPO_6);
  return 0;
}

int nivel13(struct expr *info, int nivel)
{
  int k = 0;
  struct tipo *tipo, *tipo2;
  struct rotulo *temp;
  struct nodo *temp_1;

  if (clave_lex == C_INC) {
    obt_lex();
    if (nivel13(info, 13) == 0)
      req_valorl();
    checa_entero_o_apuntador(info->tipo);
    nivel13inc(info);
  } else if (clave_lex == C_DEC) {
    obt_lex();
    if (nivel13(info, 13) == 0)
      req_valorl();
    checa_entero_o_apuntador(info->tipo);
    nivel13dec(info);
  } else if (clave_lex == C_MENOS) {
    obt_lex();
    if (nivel13(info, 13))
      carga_valor(info);
    checa_numerico(info->tipo);
    tipo = info->tipo;
    if (ultimo_nodo->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, -ultimo_nodo->esp);
    } else if (tipo->tipo == DOUBLE || tipo->tipo == FLOAT)
      crea_nodo(N_NEGPF, ultimo_nodo, NULL, 0);
    else
      crea_nodo(N_NEG, ultimo_nodo, NULL, 0);
  } else if (clave_lex == C_COM) {
    obt_lex();
    if (nivel13(info, 13))
      carga_valor(info);
    checa_entero(info->tipo);
    if (ultimo_nodo->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, ~ultimo_nodo->esp);
    } else {
      if (ultimo_nodo->oper == N_OR) {
        crea_nodo(N_NOR, ultimo_nodo->izq, ultimo_nodo->der, 0);
      } else if (ultimo_nodo->oper == N_AND) {
        crea_nodo(N_NAND, ultimo_nodo->izq, ultimo_nodo->der, 0);
      } else if (ultimo_nodo->oper == N_XOR) {
        crea_nodo(N_NXOR, ultimo_nodo->izq, ultimo_nodo->der, 0);
      } else
        crea_nodo(N_COM, ultimo_nodo, NULL, 0);
    }
  } else if (clave_lex == C_NOT) {
    obt_lex();
    if (nivel13(info, 13))
      carga_valor(info);
    checa_numerico(info->tipo);
    tipo = info->tipo;
    if (ultimo_nodo->oper == N_CONST) {
      crea_nodo(N_CONST, NULL, NULL, !ultimo_nodo->esp);
    } else if (tipo->tipo == DOUBLE || tipo->tipo == FLOAT)
      compara_cero(tipo);
    else
      crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
    info->tipo = t_sint;
  } else if (clave_lex == C_MUL) {
    obt_lex();
    k = nivel13ap(info);
  } else if (clave_lex == C_AND) {
    obt_lex();
    k = nivel13dir(info);
  } else if (clave_lex == C_SIZEOF) {
    obt_lex();
    if (clave_lex == C_PARENI) {
      obt_lex();
      if (p_tipo_1(2)) {
        p_tipo_2(0);
        if (clave_lex == C_PAREND)
          obt_lex();
        else
          error("Falta parentesis derecho");
        info->tipo = t_primero;
      } else
        k = primaria(info, SI);
    } else
      k = nivel13(info, 13);
    crea_nodo(N_CONST, NULL, NULL, tam_tipo(info->tipo));
    info->tipo = t_uint;     /* size_t es unsigned int */
    k = 0;
  } else if (clave_lex == C_PARENI) {
    obt_lex();
    if (p_tipo_1(2)) {
      if (p_tipo_2(0))
        if (clave_lex == C_PAREND)
          obt_lex();
        else
          error("Falta parentesis derecho");
      if (clave_lex == C_PAREND)
        obt_lex();
      else
        error("Falta parentesis derecho");
      tipo = t_primero;
      if (nivel13(info, 13))
        carga_valor(info);
      if (info->tipo->tipo == FUNCION) {
        tipo2 = crea_tipo(APUNTADOR);
        tipo2->sig = info->tipo;
      } else
        tipo2 = info->tipo;
      info->tipo = tipo;
      temp_1 = ultimo_nodo;
      convierte_tipo(&temp_1, tipo2, tipo, NO);
    } else
      k = primaria(info, SI);
  } else
    k = primaria(info, NO);
  if (clave_lex == C_INC) {
    obt_lex();
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info->tipo);
    nivel13pinc(info);
    k = 0;
  }
  if (clave_lex == C_DEC) {
    obt_lex();
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(info->tipo);
    nivel13pdec(info);
    k = 0;
  }
  if (nivel <= 12 && grupo_lex == GRUPO_6)
    k = nivel12(info, k);
  if (nivel <= 11 && grupo_lex == GRUPO_5)
    k = nivel11(info, k);
  if (nivel <= 10 && grupo_lex == GRUPO_4)
    k = nivel10(info, k);
  if (nivel <= 9 && grupo_lex == GRUPO_3)
    k = nivel9(info, k);
  if (nivel <= 8 && grupo_lex == GRUPO_2)
    k = nivel8(info, k);
  if (nivel <= 7 && clave_lex == C_AND)
    k = nivel7(info, k);
  if (nivel <= 6 && clave_lex == C_XOR)
    k = nivel6(info, k);
  if (nivel <= 5 && clave_lex == C_OR)
    k = nivel5(info, k);
  if (nivel <= 4 && clave_lex == C_ANDAND)
    k = nivel4(info, k);
  if (nivel <= 3 && clave_lex == C_OROR)
    k = nivel3(info, k);
  if (nivel <= 2 && clave_lex == C_TRINARIO)
    k = nivel2(info, k);
  if (nivel <= 1 && grupo_lex == GRUPO_1)
    k = nivel1(info, k);
  if (nivel <= 0 && clave_lex == C_COMA)
    k = nivel0(info, k);
  return k;
}

/*
** Las funciones nivel13ap() y nivel13dir() componen una parte fundamental
** del compilador, detectan si se pide la dirección de una variable local
** para determinar si se pone en memoria (msp) o en un registro local.
**
** Este codigo causó muchos dolores de cabeza.
*/
int nivel13ap(struct expr *info)
{
  int k;
  struct tipo *ap;

  k = nivel13(info, 13);
  if (info->tipo->tipo == FUNCION)
    return k;
  if (k) {
    carga_valor(info);
    if (ultimo_nodo->oper == N_DIR)
      virtuales[ultimo_nodo->esp + 1]--;
  }
  ap = info->tipo;
  if (ap->tipo == APUNTADOR || ap->tipo == ARREGLO)
    ap = ap->sig;
  else
    error("No es un apuntador o arreglo");
  info->tipo = ap;
  if (ap->tipo == FUNCION || ap->tipo == ARREGLO || ap->tipo == STRUCT)
    return 0;
  else
    return 1;
}

int nivel13dir(struct expr *info)
{
  struct tipo *tipo;

  if (nivel13(info, 13) == 0) {
    tipo = info->tipo;
    if (tipo->tipo != STRUCT && tipo->tipo != FUNCION)
      error("Dirección ilegal");
  }
  tipo = crea_tipo(APUNTADOR);
  tipo->sig = info->tipo;
  if (ultimo_nodo->oper == N_DIR)
    virtuales[ultimo_nodo->esp + 1]++;
  info->tipo = tipo;
  return 0;
}

void nivel13inc(struct expr *info)
{
  struct tipo *tipo;
  int inc;

  tipo = info->tipo;
  if (tipo->tipo == APUNTADOR) {
    inc = tam_tipo(tipo->sig);
    tipo = t_sint;
  } else
    inc = 1;
  crea_nodo(N_INC, ultimo_nodo, (struct nodo *) inc, (int) tipo->tipo);
}

void nivel13dec(struct expr *info)
{
  struct tipo *tipo;
  int inc;

  tipo = info->tipo;
  if (tipo->tipo == APUNTADOR) {
    inc = tam_tipo(tipo->sig);
    tipo = t_sint;
  } else
    inc = 1;
  crea_nodo(N_INC, ultimo_nodo, (struct nodo *) -inc, (int) tipo->tipo);
}

void nivel13pinc(struct expr *info)
{
  struct tipo *tipo;
  int inc;

  tipo = info->tipo;
  if (tipo->tipo == APUNTADOR) {
    inc = tam_tipo(tipo->sig);
    tipo = t_sint;
  } else
    inc = 1;
  crea_nodo(N_PINC, ultimo_nodo, (struct nodo *) inc, (int) tipo->tipo);
}

void nivel13pdec(struct expr *info)
{
  struct tipo *tipo;
  int inc;

  tipo = info->tipo;
  if (tipo->tipo == APUNTADOR) {
    inc = tam_tipo(tipo->sig);
    tipo = t_sint;
  } else
    inc = 1;
  crea_nodo(N_PINC, ultimo_nodo, (struct nodo *) -inc, (int) tipo->tipo);
}

int primaria(struct expr *info, int sin_parentesis)
{
  char nombre[TAM_NOMBRE];
  struct nombres *ap1;
  struct enumerador *ap2;
  struct rotulo *ap3;
  struct miembro *ap4;
  int k, tam;
  struct nodo *izq, *der, *temp;
  struct tipo *tipo, *tipo1, *tipo2;
  struct expr info2;

  if (!sin_parentesis && clave_lex == C_PARENI) {
    sin_parentesis = 1;
    obt_lex();
  }
  if (sin_parentesis) {
    k = nivel13(info, 0);
    if (clave_lex == C_PAREND)
      obt_lex();
    else
      error("Falta parentesis derecho");
  } else if (clave_lex == C_IDENT) {
    if (((ap1 = busca_loc(cad_lex)) != NULL)
     || ((ap1 = busca_glb(cad_lex)) != NULL)) {
      obt_lex();
      if (ap1->ident == TYPEDEF || ap1->ident == ETIQUETA)
        error("No es una variable o función");
      if (ap1->ident != FUNCION) {
        if (ap1->clase == AUTO)
          dir_var_loc(ap1);
        else
          dir_var_glb(ap1);
      } else
        dir_func(ap1);
      info->tipo = tipo = ap1->tipo;
      if (tipo->tipo == ARREGLO || tipo->tipo == FUNCION || tipo->tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if ((ap2 = busca_enum(cad_lex)) != NULL) {
      obt_lex();
      crea_nodo(N_CONST, NULL, NULL, ap2->valor);
      info->tipo = t_sint;
      k = 0;
    } else {
      dir_func(nueva_glb(cad_lex, FUNCION, STATIC, t_func, FUNC_REF));
      obt_lex();
      info->tipo = t_func;
      k = 0;
    }
  } else if (constante(info)) {
    k = 0;
  } else {
    error("Expresión inválida");
    crea_nodo(N_CONST, NULL, NULL, 0);
    obt_lex();
    info->tipo = t_sint;
    k = 0;
  }
  tipo = info->tipo;
  while (1) {
    if (clave_lex == C_CORCHI) {  /* Detectó un subindice */
      obt_lex();                  /* Según ANSI, a[b] es equivalente */
      if (k)                      /* a (*((a)+(b))), lo que nos dice */
        carga_valor(info);        /* que ¡El subindice puede aparecer */
      izq = ultimo_nodo;          /* de cualquier lado! */
      if (nivel13(&info2, 1))
        carga_valor(&info2);
      tipo1 = info2.tipo;
      der = ultimo_nodo;
      if (clave_lex == C_CORCHD)
        obt_lex();
      else
        error("Falta corchete derecho");
      if (tipo->tipo != APUNTADOR && tipo->tipo != ARREGLO) {
        if (tipo1->tipo != APUNTADOR && tipo1->tipo != ARREGLO) {
          error("Subscripto inválido");
          continue;
        }
        tipo2 = tipo1;
        tipo1 = tipo;
        tipo = tipo2;
        temp = izq;
        izq = der;
        der = temp;
      }
      checa_entero(tipo1);
      tipo = tipo->sig;
      tam = tam_tipo(tipo);
      if (der->oper == N_CONST) {
        if (der->esp == 0) {
          ultimo_nodo = izq;
        } else {
          crea_nodo(N_CONST, NULL, NULL, der->esp * tam);
          der = ultimo_nodo;
          crea_nodo(N_SUMA, izq, der, 0);
        }
      } else if (tam == 1) {
        crea_nodo(N_SUMA, izq, der, 0);
      } else {
        crea_nodo(N_CONST, NULL, NULL, tam);
        crea_nodo(N_MUL, der, ultimo_nodo, 0);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      }
      info->tipo = tipo;
      if (tipo->tipo == ARREGLO || tipo->tipo == STRUCT)
        k = 0;
      else
        k = 1;
      continue;
    }
    if (clave_lex == C_PARENI) {  /* Llamada a función */
      obt_lex();
      if (k)
        carga_valor(info);
      if (tipo->tipo != FUNCION
      && (tipo->tipo != APUNTADOR || tipo->sig->tipo != FUNCION))
        error("El tipo no es de función");
      else if (tipo->tipo == APUNTADOR)
        tipo = tipo->sig;
      tipo = tipo->sig;
      if (ultimo_nodo->oper == N_APFUNC) {
        izq = ultimo_nodo;
        llama_funcion((struct nombres *) ultimo_nodo->esp, tipo);
      } else
        llama_funcion(NULL, tipo);
      info->tipo = tipo;
      k = 0;
      continue;
    }
    if (clave_lex == C_PUNTO) {   /* Acceso a miembro */
      obt_lex();
    } else if (clave_lex == C_APUNTA) { /* Acceso indirecto a miembro */
      obt_lex();
      if (k)
        carga_valor(info);
      if (tipo->tipo == APUNTADOR || tipo->tipo == ARREGLO)
        tipo = tipo->sig;
      else
        error("No es un apuntador o arreglo");
      info->tipo = tipo;
    } else
      break;
    if (tipo->tipo != STRUCT) {
      error("No es una estructura o unión");
      continue;
    }
    ap3 = tipo->especial.est;
    if (ap3->tam == 0) {
      error("Estructura o unión incompleta");
      continue;
    }
    if (clave_lex != C_IDENT) {
      error("Nombre ilegal para el miembro");
      obt_lex();
      continue;
    }
    if ((ap4 = busca_miembro(ap3->lista, cad_lex)) != NULL) {
      info->tipo = tipo = ap4->tipo;
      if (tipo->tipo == FUNCION || tipo->tipo == ARREGLO || tipo->tipo == STRUCT)
        k = 0;
      else
        k = 1;
      if (ap4->posicion) {
        izq = ultimo_nodo;
        crea_nodo(N_CONST, NULL, NULL, ap4->posicion);
        crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
      }
    } else {
      error("Miembro indefinido");
    }
    obt_lex();
  }
  return k;
}

void req_valorl(void)
{
  error("Debe ser un valor-l");
}

/*
** Compila una llamada a una función
**
** Invocada por "primaria", esta función llamará a la función
** nombrada o a una función indirecta, efectua también las
** conversiones indicados por el prototipo.
*/
void llama_funcion(struct nombres *ap, struct tipo *tipo_funcion)
{
  struct expr info;
  int tam;
  struct nodo *izq, *anterior = NULL, *primero = NULL;
  struct tipo *tipo;
  int registros = 0, clase;
  int por_procesar, lista_variable;
  struct proto *prototipo;

  if (tipo_funcion->num_pars == 128) {
    por_procesar = 0;
    lista_variable = SI;
    prototipo = NULL;
  } else {
    por_procesar = tipo_funcion->num_pars & 63;
    if (tipo_funcion->num_pars >= 64)
      lista_variable = SI;
    else
      lista_variable = NO;
    prototipo = tipo_funcion->especial.proto;
  }
  if (ap == NULL)            /* Detecta llamada indirecta */
    izq = ultimo_nodo;
  if (tipo_funcion->tipo == STRUCT) {
    tam = (tam_tipo(tipo_funcion) + 3) / 4;
    crea_nodo(N_RESULTA, NULL, (struct nodo *) tam, 0);
    primero = anterior = ultimo_nodo;
  }                          /* Ya ha sido tomado el parentesis inicial */
  while (clave_lex != C_PAREND) {
    if (por_procesar == 0 && !lista_variable) {
      error("Demasiados parametros para función");
      break;
    }
    if (fin_sentencia()) {
      error("Falta parentesis derecho");
      break;
    }
    tam = 0;
    if (nivel13(&info, 1))
      carga_valor(&info);    /* Obtiene un argumento */
    else {
      tipo = info.tipo;
      if (tipo->tipo == STRUCT)
        tam = (tam_tipo(tipo) + 3) / 4;
      if (tipo->tipo == FUNCION) {
        info.tipo = crea_tipo(APUNTADOR);
        info.tipo->sig = tipo;
      }
    }
    tipo = info.tipo;
    if (por_procesar) {
      por_procesar--;
      convierte_tipo(&ultimo_nodo, prototipo->tipo, tipo, SI);
      tipo = prototipo->tipo;
      prototipo = prototipo->sig;
    }
    if (tipo->tipo == DOUBLE || tipo->tipo == FLOAT) {
      crea_nodo(N_PARF, ultimo_nodo, (struct nodo *) tam, 0);
      registros = (registros + 3) & ~1;
    } else {
      crea_nodo(N_PAR, ultimo_nodo, (struct nodo *) tam, 0);
      if (tipo->tipo != STRUCT)
        registros++;
    }
    if (primero == NULL)
      primero = ultimo_nodo;
    if (anterior != NULL)
      anterior->esp = (int) ultimo_nodo;
    anterior = ultimo_nodo;
    if (clave_lex == C_COMA)
      obt_lex();
    else if (clave_lex != C_PAREND) {
      error("Falta parentesis derecho");
      break;
    }
  }
  if (por_procesar)
    error("Faltan parametros para la función");
  if (clave_lex == C_PAREND)
    obt_lex();
  if (tipo_funcion->tipo == STRUCT)
    clase = 1;
  else if (tipo_funcion->tipo == FLOAT || tipo_funcion->tipo == DOUBLE)
    clase = 2;
  else
    clase = 0;
  if (ap == NULL)
    crea_nodo(N_FUNCI, primero, (struct nodo *) clase, (int) izq);
  else {
    if (prog_grande) {
      for (por_procesar = 0; por_procesar < total_funciones; por_procesar++) {
        if (funciones[por_procesar].func == ap)
          break;
      }
      if (por_procesar == total_funciones) {
        if (total_funciones == MAX_FUNCIONES) {
          error("Demasiadas funciones en bloque");
        } else {
          funciones[por_procesar].func = ap;
          funciones[por_procesar].etiqueta = ++sig_etiq;
          total_funciones++;
        }
      }
    }
    crea_nodo(N_FUNC, primero, (struct nodo *) clase, (int) ap);
  }
  registros = (registros + 1) & ~1;
  if (registros > total_regs)
    total_regs = registros;
}

/*
** Carga el valor de una dirección de memoria.
*/
void carga_valor(struct expr *info)
{
  struct tipo *tipo = info->tipo;

  if (tipo->tipo == SCHAR)
    crea_nodo(N_CCHAR, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == UCHAR)
    crea_nodo(N_CBYTE, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == SSHORT)
    crea_nodo(N_CSHORT, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == USHORT)
    crea_nodo(N_CUSHORT, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == FLOAT)
    crea_nodo(N_CFLOAT, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == DOUBLE)
    crea_nodo(N_CDOUBLE, ultimo_nodo, NULL, 0);
  else if (tipo->tipo == VOID)
    error("Tiene tipo void");
  else
    crea_nodo(N_CPAL, ultimo_nodo, NULL, 0);
}

/*
** Carga la dirección de una variable local
*/
void dir_var_loc(struct nombres *var)
{
  crea_nodo(N_DIR, NULL, NULL, var->posicion);
}

/*
** Carga la dir. de una variable global.
*/
void dir_var_glb(struct nombres *var)
{
  if (var->posicion)
    crea_nodo(N_DIRE, NULL, NULL, var->posicion);
  else
    crea_nodo(N_DIRG, NULL, NULL, (int) var);
}

/*
** Carga la dir. de una función.
*/
void dir_func(struct nombres *ap)
{
  crea_nodo(N_APFUNC, NULL, NULL, (int) ap);
}

/*
** Checa si es necesario doblar para suma o resta con apuntadores.
*/
int dobla(struct tipo *tipo, struct tipo *tipo2, struct nodo **nodo)
{
  int cuanto;

  if (tipo->tipo != APUNTADOR && tipo->tipo != ARREGLO)
    return 0;                         /* No es necesario */
  checa_entero(tipo2);
  cuanto = tam_tipo(tipo->sig);
  if ((*nodo)->oper == N_CONST) {
    crea_nodo(N_CONST, NULL, NULL, (*nodo)->esp * cuanto);
    *nodo = ultimo_nodo;              /* Es una constante */
    return 1;
  }
  multi = cuanto;
  return 2;    /* Optimizar segun sea suma o resta */
}

int constante(struct expr *info)
{
  double valor;

  if (clave_lex == C_NUMF) {
    valor = *((double *) (lits + valor_lex));
    if (valor == 0.0)
      crea_nodo(N_CEROPF, NULL, NULL, 0);
    else {
      crea_nodo(N_NUMPF, NULL, NULL, valor_lex);
      ap_lit += 8;
    }
    info->tipo = tipo_lex;
    obt_lex();
  } else if (clave_lex == C_NUM) {
    crea_nodo(N_CONST, NULL, NULL, valor_lex);
    info->tipo = tipo_lex;
    obt_lex();
  } else if (clave_lex == C_CAD) {
    crea_nodo(N_LIT, NULL, NULL, ap_lit);
    info->tipo = tipo_lex;
    ap_lit += valor_lex;
    obt_lex();
  } else
    return 0;
  return 1;
}

/*
** Funciones de conversión y chequeo de tipos.
*/

void checa_entero(struct tipo *tipo)
{
  if (tipo->tipo > UINT)
    error("No es un tipo entero");
}

void checa_numerico(struct tipo *tipo)
{
  if (tipo->tipo > DOUBLE && tipo->tipo != APUNTADOR)
    error("No es un tipo númerico");
}

void checa_entero_o_apuntador(struct tipo *tipo)
{
  if (tipo->tipo > UINT && tipo->tipo != APUNTADOR)
    error("No es un tipo entero");
}

void compara_no_cero(struct tipo *tipo)
{
  struct nodo *izq;

  if (tipo->tipo == DOUBLE || tipo->tipo == FLOAT) {
    izq = ultimo_nodo;
    crea_nodo(N_CEROPF, NULL, NULL, 0);
    crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
    crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
  }
}

void compara_cero(struct tipo *tipo)
{
  struct nodo *izq;

  izq = ultimo_nodo;
  crea_nodo(N_CEROPF, NULL, NULL, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
}

void convierte_tipo(struct nodo **nodo, struct tipo *tipo_original,
                                        struct tipo *nuevo_tipo, int advertir)
{
  if (tipo_original->tipo == VOID)
    error("No se puede convertir de void");
  else if (tipo_original->tipo == STRUCT && nuevo_tipo->tipo != STRUCT)
    error("No se puede convertir de estructura");
  else if (tipo_original->tipo != STRUCT && nuevo_tipo->tipo == STRUCT)
    error("No se puede convertir a estructura");
  else if (tipo_original->tipo == APUNTADOR &&
          (nuevo_tipo->tipo == DOUBLE || nuevo_tipo->tipo == FLOAT))
    error("No se puede convertir un apuntador a real");
  else if (nuevo_tipo->tipo == APUNTADOR &&
          (tipo_original->tipo == DOUBLE || tipo_original->tipo == FLOAT))
    error("No se puede convertir un real a apuntador");
  else {
    if ((tipo_original->tipo == FLOAT || tipo_original->tipo == DOUBLE)
     && (nuevo_tipo->tipo == FLOAT || nuevo_tipo->tipo == DOUBLE))
      return;
    if (nuevo_tipo->tipo == FLOAT || nuevo_tipo->tipo == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    } else if (tipo_original->tipo == FLOAT || tipo_original->tipo == DOUBLE) {
      crea_nodo(N_PFENT, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    }
  }
  if (advertir) {
    if (adv_cs && tipo_original->tipo == APUNTADOR
    && (nuevo_tipo->tipo != APUNTADOR && nuevo_tipo->tipo != ARREGLO))
      error("Conversión sospechosa de apuntador a tipo aritmético");
    else if ((tipo_original->tipo != APUNTADOR && tipo_original->tipo != ARREGLO)
             && nuevo_tipo->tipo == APUNTADOR && adv_cs)
      error("Conversión sospechosa de tipo aritmético a apuntador");
    else if (tipo_original->tipo == APUNTADOR && nuevo_tipo->tipo == APUNTADOR) {
      if (tipo_original->sig->tipo != VOID && nuevo_tipo->sig->tipo != VOID) {
        if (adv_ansi && compara_tipos(tipo_original->sig, nuevo_tipo->sig))
          error("Los apuntadores no son del mismo tipo");
      }
    }
  }
}

/*
** Se asegura de que dos tipos son iguales.
*/
int compara_tipos(struct tipo *tipo1, struct tipo *tipo2)
{
  while (1) {
    if (tipo1 == NULL && tipo2 == NULL)
      return 0;   /* Los tipos son iguales */
    if (tipo1 == NULL)
      return 1;
    if (tipo2 == NULL)
      return 1;
    if (tipo1->tipo == VOID || tipo2->tipo == VOID)
      return 0;
    if (tipo1->tipo != tipo2->tipo)
      return 1;
    if (tipo1->tipo == ARREGLO) {
      if (tipo1->especial.tam != tipo2->especial.tam)
        return 1;
    } else if (tipo1->tipo == STRUCT) {
/*      if (tipo1->especial.est != tipo2->especial.est)
        return 1; */ /* Esto es demasiado estricto */
    } /* No nos preocupamos por las funciones, nueva_func se encarga de ello */
    tipo1 = tipo1->sig;
    tipo2 = tipo2->sig;
  }
}

void checa_comparable(struct tipo *tipo1, struct tipo *tipo2)
{
  if (tipo1->tipo <= UINT && tipo2->tipo <= UINT)
    return;
  if ((tipo1->tipo == APUNTADOR || tipo1->tipo == ARREGLO)
   && (tipo2->tipo == APUNTADOR || tipo2->tipo == ARREGLO))
    return;
  error("Tipos incompatibles");
}

int haz_compatible(struct nodo **nodo_izq, struct expr *info_izq,
                   struct nodo **nodo_der, struct expr *info_der)
{
  struct tipo *tipo_izq, *tipo_der;

  tipo_izq = info_izq->tipo;
  tipo_der = info_der->tipo;
  if (tipo_izq->tipo == VOID || tipo_der->tipo == VOID)
    error("No se pueden efectuar operaciones con void");
  else if (tipo_izq->tipo == STRUCT || tipo_der->tipo == STRUCT)
    error("No se pueden efectuar operaciones con estructuras");
  if ((tipo_izq->tipo == FLOAT || tipo_izq->tipo == DOUBLE) &&
      (tipo_der->tipo == FLOAT || tipo_der->tipo == DOUBLE)) {
    info_izq->tipo = t_double;
    return 1;
  }
  if (tipo_izq->tipo == FLOAT || tipo_izq->tipo == DOUBLE) {
    crea_nodo(N_ENTPF, *nodo_der, NULL, 0);
    *nodo_der = ultimo_nodo;
    info_izq->tipo = t_double;
    return 1;
  }
  if (tipo_der->tipo == FLOAT || tipo_der->tipo == DOUBLE) {
    crea_nodo(N_ENTPF, *nodo_izq, NULL, 0);
    *nodo_izq = ultimo_nodo;
    info_izq->tipo = t_double;
    return 1;
  }
  if ((tipo_izq->tipo == UINT || tipo_der->tipo == UINT)
  && (tipo_izq->tipo != APUNTADOR && tipo_der->tipo != APUNTADOR)
  && (tipo_izq->tipo != ARREGLO && tipo_der->tipo != ARREGLO))
    info_izq->tipo = info_der->tipo = t_uint;
  return 0;
}

/*
** Crea un nodo del arbol de expresiones, detecta subexpresiones comunes.
**
** Optimiza los casos especiales de multiplicaciones y divisiones por
** multiplos de 2.
*/
void crea_nodo(int op, struct nodo *izq, struct nodo *der, int val)
{
  int bits, dispersion;
  struct nodo *subexpr;

  if (op == N_SUMA && der->oper == N_CONST
  && izq->oper == N_SUMA && izq->der->oper == N_CONST) {
    crea_nodo(N_CONST, NULL, NULL, izq->der->esp + der->esp);
    crea_nodo(N_SUMA, izq->izq, ultimo_nodo, 0);
    return;
  }
  if (op == N_MUL && der->oper == N_CONST && (bits = es_potencia(der->esp))) {
    op = N_CI;
    crea_nodo(N_CONST, NULL, NULL, bits);
    der = ultimo_nodo;
  } else if (op == N_AMUL && izq->oper == N_CONST
          && (bits = es_potencia(izq->esp))) {
    op = N_ACI;
    crea_nodo(N_CONST, NULL, NULL, bits);
    izq = ultimo_nodo;
  } else if ((op == N_DIV || op == N_SDIV) && der->oper == N_CONST
          && (bits = es_potencia(der->esp))) {
    val = (op == N_SDIV);
    op = N_CD;
    crea_nodo(N_CONST, NULL, NULL, bits);
    der = ultimo_nodo;
  } else if (op == N_ADIV && izq->oper == N_CONST
          && (bits = es_potencia(izq->esp))) {
    op = N_ACD;
    crea_nodo(N_CONST, NULL, NULL, bits);
    izq = ultimo_nodo;
  } else if (op == N_SMOD && der->oper == N_CONST
          && (bits = es_potencia(der->esp))) {
    op = N_AND;
    crea_nodo(N_CONST, NULL, NULL, (1 << bits) - 1);
    der = ultimo_nodo;
  }
  ultimo_nodo = NULL;
  dispersion = (((int) izq + (int) der + op + val) & 0x0FFFFFFF) % NUM_PRIMO;
  if (op != N_RESULTA && op != N_PAR && op != N_PARF && op != N_INC && op != N_PINC) {
    subexpr = subexpresion[dispersion];
    while (subexpr != NULL) {
      if (subexpr->izq == izq && subexpr->der == der &&
          subexpr->oper == op && subexpr->esp == val) {
        ultimo_nodo = subexpr;
        break;
      }
      subexpr = subexpr->sig;
    }
  }
  if (ultimo_nodo == NULL) {
    ultimo_nodo = malloc(sizeof(struct nodo));
    if (ultimo_nodo == NULL) {
      error("Expresión muy compleja");
      cancela();
    }
    ultimo_nodo->ap = &subexpresion[dispersion];
    ultimo_nodo->sig = subexpresion[dispersion];
    if (subexpresion[dispersion] != NULL)
      subexpresion[dispersion]->ap = &ultimo_nodo->sig;
    subexpresion[dispersion] = ultimo_nodo;
    ultimo_nodo->izq = izq;
    ultimo_nodo->der = der;
    ultimo_nodo->oper = op;
    ultimo_nodo->esp = val;
    ultimo_nodo->regs = 0;
  }
  ultimo_nodo->usos++;
}

/*
** Libera todas las expresiones
*/
void libera_expr(void)
{
  struct nodo *nodo, *sig;
  int c;

  for (c = 0; c < NUM_PRIMO; c++) {
    nodo = subexpresion[c];
    while (nodo != NULL) {
      sig = nodo->sig;
      free(nodo);
      nodo = sig;
    }
    subexpresion[c] = NULL;
  }
}

/*
** Detecta potencias de 2 y regresa total de bits en caso dado.
*/
int es_potencia(int valor)
{
  int numero, cuenta;

  numero = 2;
  cuenta = 1;
  while (cuenta < 31) {
    if (valor == numero)
      return cuenta;
    cuenta++;
    numero <<= 1;
  }
  return 0;
}
