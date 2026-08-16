/*
** Bloques (un Tetris mexicano)
**
** por Oscar Toledo Gutiérrez
**
** © Copyright Oscar Toledo G.1998
**
** Creación: 21-sep-1998.
** Revisión: 16-mar-1999. Corrección de algunos defectos.
** Revisión: 09-abr-1999. Corrección de un defecto al completar líneas.
*/

/*
** Poner tablas de puntuación
** Poner indicador de estadísticas
** Ruidos raros
*/
#include <Sistema.h>

#define COLOR_FONDO 0x000000
#define TAM_X 16
#define TAM_Y 16

int bloques[20][10];     /* Arreglo de bloques */

int nivel;               /* Nivel actual de juego */
int rotaciones[7];       /* Rotaciones permitidas */
int formas[7][4][4][4];  /* Forma Línea Columna Rotación */
int puntuacion[7][4];    /* Puntuaciones por posicionamiento */
int generador;           /* Generador de números aleatorios */
int lineas;              /* Contador de líneas hechas */
int f, x, y, r, c;       /* Figura en movimiento, color, rotación, etc. */
int puntos;              /* Puntuación acumulada */
int pausa;               /* 0= Jugando, 1= Pausa, 2= Fin de juego */
int sigue, sigue_c;      /* Forma que sigue o -1 si aún no empieza */
int juegos;              /* Total de juegos */
int parpadeando;         /* Indica si esta parpadeando una línea */

void *ventana;

/*
** El programa empieza aquí
*/
void main(void)
{
  void *clase, *ventana;

  generador = lee_sistema(0) + lee_sistema(15);
  clase = crea_clase(L"Base", interfaz_base, NULL, 64, 0, 0);
  if (clase == NULL)
    aviso_error(1);
  ventana = ventana_estandar(clase, (void *) lee_sistema(S_VENTANA_RAIZ),
                             L"Bloques", TAM_X * 15, TAM_Y * 20 + 8, 1);
  if (ventana == NULL)
    aviso_error(1);
  while (1)
    multitarea();
}

/*
** En esta interfaz se encuentra la visualización de las
** puntuaciones y el codigo para cerrar la aplicación.
*/
int interfaz_base(void *v, int mensaje, int par1, int par2)
{
  void *clase, *extra;
  wchar_t buffer[20];

  switch (mensaje) {
    case M_CREAR:
      clase = crea_clase(L"Juego", interfaz, NULL, 64, 0, 0);
      if (clase == NULL)
        return 0;
      ventana = crea_ventana(clase, v, 0, NULL, 4, 4,
                             TAM_X * 10, TAM_Y * 20, 0);
      if (ventana == NULL)
        return 0;
      break;
    case M_PINTAR:
      sel_color(v, lee_sistema(S_V_COLOR_FONDO));
      rellena(v, par1 & 0xffff, (par1 >> 16) & 0xffff,
                (par2 - par1) & 0xffff, ((par2 - par1) >> 16) & 0xffff);
      dibuja_borde(v, 2, 2, TAM_X * 10 + 4, TAM_Y * 20 + 4,
                   0, lee_sistema(S_B_COLOR_CLARO),
                   lee_sistema(S_B_COLOR_OBSCURO), 2);
      sel_color(v, 0x000000);
      sel_tipo(v, lee_sistema(S_B_TIPO_LETRA));
      ilustra_texto(v, L"Puntos:", TAM_X * 11, 70);
      ilustra_texto(v, L"Sigue:", TAM_X * 11, 120);
      ilustra_texto(v, L"Juego:", TAM_X * 11, 239);
      ilustra_texto(v, L"Nivel:", TAM_X * 11, 25);
    case 103:
      sel_color(v, 0x000000);
      rellena(v, TAM_X * 11, 244, TAM_X * 7 / 2, 20);
      rellena(v, TAM_X * 11, 30, TAM_X * 7 / 2, 20);
      dibuja_borde(v, TAM_X * 11, 30, TAM_X * 7 / 2, 20,
                   0, lee_sistema(S_B_COLOR_CLARO),
                   lee_sistema(S_B_COLOR_OBSCURO), 1);
      dibuja_borde(v, TAM_X * 11, 244, TAM_X * 7 / 2, 20,
                   0, lee_sistema(S_B_COLOR_CLARO),
                   lee_sistema(S_B_COLOR_OBSCURO), 1);
      sel_color(v, 0x00FF00);
      sel_tipo(v, lee_sistema(S_B_TIPO_LETRA));
      decimal(juegos, buffer);
      ilustra_texto(v, buffer, TAM_X * 11 + 5, 260);
      decimal(nivel, buffer);
      ilustra_texto(v, buffer, TAM_X * 11 + 5, 46);
    case 102:
      if (sigue != -1) {
        sel_color(v, 0x000000);
        rellena(v, TAM_X * 11 - 4, 144 / TAM_Y * TAM_Y - 4,
                   TAM_X * 3 + 16, TAM_Y * 4 + 8);
        dibuja_borde(v, TAM_X * 11 - 4, 144 / TAM_Y * TAM_Y - 4,
                     TAM_X * 3 + 16, TAM_Y * 4 + 8,
                     0, lee_sistema(S_B_COLOR_CLARO),
                     lee_sistema(S_B_COLOR_OBSCURO), 2);
        extra = ventana;
        ventana = v;
        visualiza(sigue, 11, 144 / TAM_Y, sigue == 5 ? 5 : 4, sigue_c);
        ventana = extra;
      }
    case 101:
      sel_color(v, lee_sistema(S_V_COLOR_FONDO));
      rellena(v, TAM_X * 11, TAM_Y * 19, TAM_X * 4, TAM_Y);
      sel_color(v, 0x000000);
      if (pausa == 1)
        ilustra_texto(v, L"Pausa", TAM_X * 11, TAM_Y * 19 + (TAM_Y / 4 * 3));
    case 100:
      sel_color(v, 0x000000);
      rellena(v, TAM_X * 11, 75, TAM_X * 7 / 2, 20);
      dibuja_borde(v, TAM_X * 11, 75, TAM_X * 7 / 2, 20,
                   0, lee_sistema(S_B_COLOR_CLARO),
                   lee_sistema(S_B_COLOR_OBSCURO), 1);
      sel_color(v, 0x00FF00);
      sel_tipo(v, lee_sistema(S_B_TIPO_LETRA));
      decimal(puntos, buffer);
      ilustra_texto(v, buffer, TAM_X * 11 + 5, 91);
      break;
    case M_CERRAR:
      termina_tarea(leer_variable(v, V_TAREA));
      break;
    case M_FOCO:
      if (par1)
        foco_teclado(ventana);
      break;
  }
  return 1;
}

/*
** Conversión de un número décimal a una cadena de wchar_t
*/
wchar_t *decimal(int valor, wchar_t *ap)
{
  if (valor >= 10)
    ap = decimal(valor / 10, ap);
  *ap++ = (valor % 10) + '0';
  *ap = 0;
  return ap;
}

/*
** Dibuja un bloque de color en la ventana en 3-D
*/
void dibuja_bloque(void *ventana, int x, int y, int color)
{
  int col1, col2, col3;

  if (color == 0) {
    sel_color(ventana, COLOR_FONDO);
    rellena(ventana, x * TAM_X, y * TAM_Y, TAM_X, TAM_Y);
  } else {
    switch (color) {
      case 1: col1 = 0x0000FF; col2 = 0x0000C0; col3 = 0x000080; break;
      case 2: col1 = 0x00FF00; col2 = 0x00C000; col3 = 0x008000; break;
      case 3: col1 = 0xFF0000; col2 = 0xC00000; col3 = 0x800000; break;
      case 4: col1 = 0x00FFFF; col2 = 0x00C0C0; col3 = 0x008080; break;
      case 5: col1 = 0xFFFF00; col2 = 0xC0C000; col3 = 0x808000; break;
      case 6: col1 = 0xFF00FF; col2 = 0xC000C0; col3 = 0x800080; break;
      case 7: col1 = 0xFFFFFF; col2 = 0xC0C0C0; col3 = 0x808080; break;
    }
    dibuja_borde(ventana, x * TAM_X, y * TAM_Y,
                 TAM_X, TAM_Y, 0, col3, col1, 2);
    sel_color(ventana, col2);
    rellena(ventana, x * TAM_X + 2, y * TAM_Y + 2, TAM_X - 4, TAM_Y - 4);
    dibuja_borde(ventana, x * TAM_X + (TAM_X / 2 - 2), y * TAM_Y + (TAM_Y / 2 - 2),
            4, 4, 0, col1, col3, 2);
  }
}

/*
** Bloques diseñados, identicos al Tetris estándar,
** las puntuaciones asignadas son un tanto arbitrarias.
*/
void prepara_bloques(void)
{
  rotaciones[0] = 2;
  puntuacion[0][0] = 14;
  formas[0][0][0][0] = 1; formas[0][0][1][0] = 1;
  formas[0][1][1][0] = 1; formas[0][1][2][0] = 1;
  puntuacion[0][1] = 17;
  formas[0][0][1][1] = 1; formas[0][1][0][1] = 1;
  formas[0][1][1][1] = 1; formas[0][2][0][1] = 1;
  rotaciones[1] = 2;
  puntuacion[1][0] = 14;
  formas[1][0][1][0] = 1; formas[1][0][2][0] = 1;
  formas[1][1][0][0] = 1; formas[1][1][1][0] = 1;
  puntuacion[1][1] = 17;
  formas[1][0][0][1] = 1; formas[1][1][0][1] = 1;
  formas[1][1][1][1] = 1; formas[1][2][1][1] = 1;
  rotaciones[2] = 4;
  puntuacion[2][0] = 9;
  formas[2][0][0][0] = 1; formas[2][1][0][0] = 1;
  formas[2][1][1][0] = 1; formas[2][1][2][0] = 1;
  puntuacion[2][1] = 15;
  formas[2][0][0][1] = 1; formas[2][0][1][1] = 1;
  formas[2][1][0][1] = 1; formas[2][2][0][1] = 1;
  puntuacion[2][2] = 13;
  formas[2][0][0][2] = 1; formas[2][0][1][2] = 1;
  formas[2][0][2][2] = 1; formas[2][1][2][2] = 1;
  puntuacion[2][3] = 11;
  formas[2][0][1][3] = 1; formas[2][1][1][3] = 1;
  formas[2][2][0][3] = 1; formas[2][2][1][3] = 1;
  rotaciones[3] = 4;
  puntuacion[3][0] = 9;
  formas[3][0][2][0] = 1; formas[3][1][0][0] = 1;
  formas[3][1][1][0] = 1; formas[3][1][2][0] = 1;
  puntuacion[3][1] = 15;
  formas[3][0][0][1] = 1; formas[3][0][1][1] = 1;
  formas[3][1][1][1] = 1; formas[3][2][1][1] = 1;
  puntuacion[3][2] = 13;
  formas[3][0][0][2] = 1; formas[3][0][1][2] = 1;
  formas[3][0][2][2] = 1; formas[3][1][0][2] = 1;
  puntuacion[3][3] = 11;
  formas[3][0][0][3] = 1; formas[3][1][0][3] = 1;
  formas[3][2][0][3] = 1; formas[3][2][1][3] = 1;
  rotaciones[4] = 1;
  puntuacion[4][0] = 12;
  formas[4][0][1][0] = 1; formas[4][0][2][0] = 1;
  formas[4][1][1][0] = 1; formas[4][1][2][0] = 1;
  rotaciones[5] = 2;
  puntuacion[5][0] = 15;
  formas[5][0][0][0] = 1; formas[5][0][1][0] = 1;
  formas[5][0][2][0] = 1; formas[5][0][3][0] = 1;
  puntuacion[5][1] = 11;
  formas[5][0][1][1] = 1; formas[5][1][1][1] = 1;
  formas[5][2][1][1] = 1; formas[5][3][1][1] = 1;
  rotaciones[6] = 4;
  puntuacion[6][0] = 11;
  formas[6][0][0][0] = 1; formas[6][0][1][0] = 1;
  formas[6][0][2][0] = 1; formas[6][1][1][0] = 1;
  puntuacion[6][1] = 13;
  formas[6][0][0][1] = 1; formas[6][1][0][1] = 1;
  formas[6][1][1][1] = 1; formas[6][2][0][1] = 1;
  puntuacion[6][2] = 15;
  formas[6][0][1][2] = 1; formas[6][1][0][2] = 1;
  formas[6][1][1][2] = 1; formas[6][1][2][2] = 1;
  puntuacion[6][3] = 13;
  formas[6][0][1][3] = 1; formas[6][1][0][3] = 1;
  formas[6][1][1][3] = 1; formas[6][2][1][3] = 1;
}

#define TECLA_ARRIBA  0x18
#define TECLA_IZQ     0x14
#define TECLA_DER     0x16
#define TECLA_ABAJO   0x12

/*
** Interfaz principal del juego
*/
int interfaz(void *ventana, int mensaje, int par1, int par2)
{
  int a, b, x1, y1, x2, y2;

  switch (mensaje) {
    case M_CREAR:
      prepara_bloques();
      mensaje_urgente(ventana, 100, 0, 0);
      break;
    case M_PINTAR:
      x1 = par1 & 0xffff;
      x2 = par2 & 0xffff;
      y1 = par1 >> 16;
      y2 = par2 >> 16;
      x2 += TAM_X - 1;
      y2 += TAM_Y - 1;
      x1 = x1 / TAM_X;
      y1 = y1 / TAM_Y;
      x2 = x2 / TAM_X;
      y2 = y2 / TAM_Y;
      for (a = x1; a < x2; a++)
        for (b = y1; b < y2; b++)
          dibuja_bloque(ventana, a, b, bloques[b][a]);
      break;
    case M_RELOJ:
      if (pausa)
        break;
      if (f != -1)
        baja_bloque();
      pone_alarma(ventana, 50 - nivel * 3);
      break;
    case M_TECLADO:
      switch (par1) {
        case 'P':
        case 'p':
          if (parpadeando)
            break;
          if (pausa == 1) {
            pausa = 0;
            pone_alarma(ventana, 50 - nivel * 3);
            envia_mensaje(leer_variable(ventana, V_MADRE), 101, 0, 0);
          } else if (pausa == 0) {
            pausa = 1;
            pone_alarma(ventana, 0);
            envia_mensaje(leer_variable(ventana, V_MADRE), 101, 0, 0);
          }
          break;
        case 0x15:
          if (pausa || parpadeando)
            break;
          rotacion();
          break;
        case TECLA_IZQ:
          if (pausa || parpadeando)
            break;
          izquierda();
          break;
        case TECLA_DER:
          if (pausa || parpadeando)
            break;
          derecha();
          break;
        case TECLA_ABAJO:
          if (pausa || parpadeando)
            break;
          tira();
          break;
      }
      break;
    case 100:  /* Inicia un nuevo juego */
      for (a = 0; a < 20; a++)
        for (b = 0; b < 10; b++)
          bloques[a][b] = 0;
      nivel = 1;
      lineas = 0;
      pausa = 0;
      puntos = 0;
      sigue = -1;
      parpadeando = 0;
      juegos++;
      actualiza_juegos();
      actualiza_puntos();
      nuevo_bloque();
      pone_alarma(ventana, 50 - nivel * 3);
      break;
  }
  return 1;
}

/*
** Actualiza el contador de juegos
*/
void actualiza_juegos(void)
{
  envia_mensaje(leer_variable(ventana, V_MADRE), 103, 0, 0);
}

/*
** Actualiza el marcador de puntos
*/
void actualiza_puntos(void)
{
  envia_mensaje(leer_variable(ventana, V_MADRE), 100, 0, 0);
}

/*
** "Tira" un bloque
*/
void tira(void)
{
  if (y == 0)
    baja_bloque();
  while (y)
    baja_bloque();
  pone_alarma(ventana, 50 - nivel * 3);
}

/*
** Gira o rota un bloque
*/
void rotacion(void)
{
  int ra;

  ra = r;
  visualiza(f, x, y, r, 0);
  r = (r + 1) % rotaciones[f];
  if (choque(f, x, y, r))
    r = ra;
  visualiza(f, x, y, r, c);
}

/*
** Mueve a la izquierda el bloque
*/
void izquierda(void)
{
  int xa;

  xa = x;
  visualiza(f, x, y, r, 0);
  x--;
  if (choque(f, x, y, r))
    x = xa;
  visualiza(f, x, y, r, c);
}

/*
** Mueve a la derecha el bloque
*/
void derecha(void)
{
  int xa;

  xa = x;
  visualiza(f, x, y, r, 0);
  x++;
  if (choque(f, x, y, r))
    x = xa;
  visualiza(f, x, y, r, c);
}

/*
** Baja una línea el bloque
*/
void baja_bloque(void)
{
  visualiza(f, x, y, r, 0);
  y++;
  if (choque(f, x, y, r)) {
    y--;
    visualiza(f, x, y, r, c);
    puntos += puntuacion[f][r];
    actualiza_puntos();
    f = -1;
    checa_lineas();
    nuevo_bloque();
  } else {
    visualiza(f, x, y, r, c);
  }
}

/*
** Checa si se ha hecho una línea y la hace parpadear.
*/
void checa_lineas(void)
{
  int a, b, c, d;
  int *linea, *origen, *destino, cuenta;

  parpadeando = 1;
  for (b = 0; b < 20; b++) {
    d = 0;
    for (a = 0; a < 20; a++) {
      linea = bloques[a];
      for (c = 0; c < 10; c++) {
        if (*linea++ == 0)
          break;
      }
      if (c == 10) {
        d = 1;
        if (b == 19) {
          destino = bloques[a] + 9;
          origen = destino - 10;
          cuenta = a * 10;
          while (cuenta--)
            *destino-- = *origen--;
          destino = bloques;
          for (cuenta = 0; cuenta < 10; cuenta++)
            *destino++ = 0;
          lineas++;
          if (lineas % 20 == 0) {
            if (nivel < 12) {
              nivel++;
              actualiza_juegos();
            }
          }
          puntos += 15;
        } else {
          for (c = 0; c < 10; c++) {
            if (b & 1)
              dibuja_bloque(ventana, c, a, bloques[a][c]);
            else
              dibuja_bloque(ventana, c, a, 7);
          }
        }
      }
    }
    if (d == 0)
      break;
    multitarea();
  }
  parpadeando = 0;
  if (d) {
    mensaje_urgente(ventana, M_PINTAR, 0, -1);
    actualiza_puntos();
  }
}

/*
** Un nuevo bloque
*/
void nuevo_bloque(void)
{
  f = sigue;
  c = sigue_c;
  if (f == -1) {
    f = aleatorio(7);
    c = aleatorio(6) + 1;
  }
  sigue = aleatorio(7);
  sigue_c = aleatorio(6) + 1;
  envia_mensaje(leer_variable(ventana, V_MADRE), 102, 0, 0);
  x = 3;
  y = 0;
  r = 0;
  if (choque(f, x, y, r)) {
    visualiza(f, x, y, r, c);
    fin_de_juego();
  } else {
    visualiza(f, x, y, r, c);
  }
}

/*
** Visualiza la forma
*/
void visualiza(int f, int x, int y, int r, int c)
{
  int a, b;

  for (a = 0; a < 4; a++) {
    for (b = 0; b < 4; b++) {
      if (formas[f][a][b][r & 3]) {
        if (r < 4) {
          if (y + a >= 0 && y + a <= 19
           && x + b >= 0 && x + b <= 9)
            bloques[y + a][x + b] = c;
        }
        dibuja_bloque(ventana, x + b, y + a, c);
      }
    }
  }
}

/*
** Averigua si una forma choca con bloques ya puestos
*/
int choque(int f, int x, int y, int r)
{
  int a, b;

  for (a = 0; a < 4; a++) {
    for (b = 0; b < 4; b++) {
      if (formas[f][a][b][r]) {
        if (y + a < 0)
          return 1;
        if (y + a > 19)
          return 1;
        if (x + b < 0)
          return 1;
        if (x + b > 9)
          return 1;
        if (bloques[y + a][x + b])
          return 1;
      }
    }
  }
  return 0;
}

/*
** Fin del juego, muestra un aviso
*/
void fin_de_juego(void)
{
  envia_mensaje(ventana, 100, 0, 0);
  envia_mensaje(ventana, M_PINTAR, 0, -1);
}

/*
** Generador de números aleatorios, funciona muy bien.
*/
int aleatorio(int rango)
{
  generador = generador * 1103515245 + 12345;
  return (unsigned) (generador >> 16) % rango;
}
