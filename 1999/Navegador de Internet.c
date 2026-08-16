/*
** Navegador de Internet
**
** por Oscar Toledo Gutiérrez
**
** © Copyright Oscar Toledo G.1996-1999
**
** Creación: 22-mar-1999.
*/

#include <Sistema.h>

#define DEPURACION_HTML

#define NOMBRE_CONFIGURACION L"Navegador de Internet"

#include "Iconos.c"            /* Pelotas de la barra de herramientas */

#include "Tipos de letra.c"    /* Información sobre los tipos de letra */

void *ventana_raiz;
void *ventana_interfaz;
void *ventana_documento;

#include "Rutinas de apoyo.c"  /* Rutinas de apoyo */

#include "HTML.c"              /* Procesamiento de documento HTML */

#include "Visualización.c"     /* Visualización del texto formateado */

int hay_archivero;             /* Indica si hay un archivero activo */

wchar_t buffer[512];           /* Buffer de trabajo */

/*
** Programa principal
*/
void main(void)
{
  void *clase;

  ventana_raiz = (void *) lee_sistema(S_VENTANA_RAIZ);
  clase = crea_clase(L"Base", interfaz, NULL, 64, 0, 0);
  if (clase == NULL)
    aviso_error(1);
  ventana_interfaz = ventana_estandar(clase, ventana_raiz,
     NOMBRE_CONFIGURACION, 720, 480,
     E_BOTON_CERRAR | E_BOTON_AYUDA | E_BOTON_REDIM |
     E_REDIM_HOR | E_REDIM_VER | E_SIN_TECLADO);
  if (ventana_interfaz == NULL)
    aviso_error(1);
  while (1)
    multitarea();
}

/*
** Crea la interfaz
*/
int crea_interfaz(void *ventana)
{
  void *clase, *v;
  int tx = leer_variable(ventana, V_TAMX);
  int ty = leer_variable(ventana, V_TAMY);

  clase = busca_clase(L"Boton", 0);
  v = crea_ventana(clase, ventana, 1, L"Archivos", 0, 4, 96, 28, 8);
  if (v == NULL)
    return 0;
  v = crea_ventana(clase, ventana, 2, L"Opciones", 96, 4, 96, 28, 8);
  if (v == NULL)
    return 0;
  clase = busca_clase(L"Boton", 0);
  v = crea_ventana(clase, ventana, 9, L"Dirección", 400, 4, 96, 28, 5);
  if (v == NULL)
    return 0;
  clase = busca_clase(L"Multiple", 0); /* !!! */
  v = crea_ventana(clase, ventana, 10, NULL, 492, 6, tx - 492, 24, 0);
  if (v == NULL)
    return 0;
  clase = crea_clase(L"Internet", interfaz_internet, NULL,
                     sizeof(struct contexto), 0, 0);
  if (clase == NULL)
    return 0;
  v = crea_ventana(clase, ventana, 12, NULL, 0, 36, tx, ty - 64, 8);
  if (v == NULL)
    return 0;
  ventana_documento = v;
  clase = busca_clase(L"Estado", 0);
  v = crea_ventana(clase, ventana, 15, NULL, 0, ty - 24, 128, 24, 0);
  if (v == NULL)
    return 0;
  v = crea_ventana(clase, ventana, 16, NULL, 128, ty - 24, tx - 144, 24, 0);
  if (v == NULL)
    return 0;
  return 1;
}

/*
** Interfaz principal
*/
int interfaz(void *ventana, int mensaje, int par1, int par2)
{
  void *clase, *v;
  int config, a;

  switch (mensaje) {
    case M_CREAR:
      prepara_pelotas();
      return crea_interfaz(ventana);
    case M_PINTAR:
      sel_color(ventana, lee_sistema(S_V_COLOR_FONDO));
      rellena(ventana, par1 & 0xffff, (par1 >> 16) & 0xffff,
                       (par2 - par1) & 0xffff, ((par2 - par1) >> 16) & 0xffff);
      dibuja_bitmap_16(ventana, dibujo_pelotas, 160, 32, 208, 2);
      break;
    case M_BORRAR:
      break;
    case M_INTERFAZ:
      switch (par1) {
        case 1:
          switch (par1) {
            case 1:
              /*
              ** Crea un archivero
              */
              clase = busca_clase(L"Archivero", 0);
              if (clase == NULL)
                break;
              v = ventana_estandar(clase, ventana_raiz,
                                   L"Leer publicación", 288, 264,
                                   E_BOTON_CERRAR | E_BOTON_REDIM |
                                   E_REDIM_HOR | E_REDIM_VER |
                                   E_SIN_TECLADO | E_ARCHIVERO_LEE);
              if (v == NULL)
                break;
              hay_archivero = 1;
              escribe_variable(v, V_PROPIETARIA, (int) ventana);

              /*
              ** Selecciona la última carpeta que se uso
              */
              config = abre_configuracion(NOMBRE_CONFIGURACION, 0);
              if (config >= 0) {
                if (lee_configuracion(config, L"Archivero",
                                      buffer, sizeof(buffer)) != 1)
                  carpeta_por_omision(buffer);
                cierra_configuracion(config);
              } else
                carpeta_por_omision(buffer);
              a = 2;
              while (buffer[a] != 0)
                a++;
              while (a != 2 && buffer[a] != '/')
                a--;
              buffer[a + 1] = 0;
              mensaje_urgente(v, M_ARCHIVERO, buffer + 2, *((int *) buffer));
              break;
          }
          break;
      }
      break;
    case M_CERRAR:
      termina_tarea(leer_variable(ventana, V_TAREA));
      break;
    case M_AVISO:  /* Interacción con un aviso */
      switch (par1) {
        case 'EXP0':  /* Aviso del archivero, lectura */
          /*
          ** Comodidad para el usuario: guarda el camino a la carpeta.
          */
          if ((void *) par2 != NULL) {
            config = abre_configuracion(NOMBRE_CONFIGURACION, 0);
            if (config >= 0) {
              escribe_configuracion(config, L"Archivero", (void *) par2, 1024);
              cierra_configuracion(config);
            }
            memcpy(buffer, (char *) par2 + 4, 1024);
            envia_mensaje(ventana_documento, M_RELOJ, 0, 0);
            free((void *) par2);
          }
          hay_archivero = 0;
          break;
      }
      break;
  }
  return 1;
}

/*
** Interfaz con Internet
*/
int interfaz_internet(void *ventana, int mensaje, int par1, int par2)
{
  struct contexto *contexto = ventana;
  int archivo;
  void *clase, *v;
  int tx, ty;

  switch (mensaje) {
    case M_CREAR:
      if (inicia_html(ventana, 1))
        return 0;
      contexto = ventana;
      tx = leer_variable(ventana, V_TAMX);
      ty = leer_variable(ventana, V_TAMY);
      clase = busca_clase(L"Barra", 0);
      v = crea_ventana(clase, ventana, 13, NULL, tx - 16, 0, 16, ty, 1);
      if (v == NULL)
        return 0;
      contexto->barra_ver = v;
      v = crea_ventana(clase, ventana, 14, NULL, 0, ty - 16, tx - 16, 16, 0);
      if (v == NULL)
        return 0;
      contexto->barra_hor = v;
      break;
    case M_PINTAR:
      optimiza_ventana(ventana, 1, par1, par2);
      if (contexto->imagen_fondo == NULL) {
        sel_color(ventana, contexto->color_fondo);
        rellena(ventana, par1 & 0xffff, (par1 >> 16) & 0xffff,
                         (par2 - par1) & 0xffff, ((par2 - par1) >> 16) & 0xffff);
      } else {
        /* !!! Rellena el fondo con imagen */
      }
      visualizar_html(contexto);
      optimiza_ventana(ventana, 0, 0, 0);
      break;
    case M_BORRAR:
      elimina_lista_visual(contexto);
      libera_cache_tipos();
      break;
    case M_RELOJ:
      archivo = open(buffer, 0);
      if (archivo >= 0) {
        procesa_html(ventana, archivo, -1);
        close(archivo);
        envia_mensaje(ventana, M_PINTAR, 0, -1);
      }
      break;
  }
  return 1;
}
