/*
** Libreria estándar del sistema de ventanas <Sistema.h>
**
** por Oscar Toledo Gutiérrez
**
** © Copyright Oscar Toledo G.1998
*/

#define NULL             ((void *) 0)

typedef unsigned short wchar_t;
typedef int size_t;

/*
** Mensajes estándar del sistema de ventanas
*/
#define M_CREAR          0x01  /* Ventana creada */
#define M_PINTAR         0x02  /* Pinta ventana */
#define M_BORRAR         0x03  /* Ventana borrandose */
#define M_RATON          0x04  /* Raton desplazado */
#define M_OPRIMEIZQ      0x05  /* Boton izquierdo oprimido */
#define M_SUELTAIZQ      0x06  /* Boton izquierdo soltado */
#define M_OPRIMEDER      0x07  /* Boton derecho oprimido */
#define M_SUELTADER      0x08  /* Boton derecho soltado */
#define M_RELOJ          0x09  /* Mensaje del reloj */
#define M_TECLADO        0x0A  /* Mensaje del teclado */
#define M_FOCO           0x0B  /* Obtiene o pierde foco del teclado */
#define M_SUELTA         0x0C  /* Se soltaron archivos */
#define M_ARRASTRE       0x0D  /* Se están arrastrando archivos */
#define M_OPRIMECEN      0x0E  /* Boton central oprimido */
#define M_SUELTACEN      0x0F  /* Boton central soltado */
#define M_DIALOGO        0x10  /* Diálogo recien creado */
#define M_MINIMIZA       0x11  /* Ventana minimizada */
#define M_RESTAURA       0x12  /* Ventana restaurada */
#define M_LIMITE         0x13  /* Limite de tamaño de la ventana */
#define M_GLOBO          0x14  /* Cursor fijo por un rato */
#define M_DESPLAZA       0x15  /* Ventana desplazada */

#define M_INTERFAZ       0x20  /* Mensaje de elemento de interfaz */
#define M_INTERFAZ1      0x21  /* Mensaje indicador de arrastre de barra */
#define M_CERRAR         0x22  /* Cerrar la aplicación */
#define M_REDIMENSION    0x23  /* Redimensión de la ventana */
#define M_AYUDA          0x24  /* Pedido de ayúda */
#define M_RENOMBRADO     0x25  /* Elemento renombrado en lista */
#define M_CAMBIO_SEL     0x26  /* Cambio de selección en lista */

#define M_AVISO          0x40  /* Aviso */

#define M_BARRA_ACTIVA   0x20  /* Activa/Desactiva la barra */
#define M_BARRA_AMPLITUD 0x21  /* Selecciona la amplitud de la barra */
#define M_BARRA_POS      0x22  /* Selecciona la posición de la barra */
#define M_BARRA_COLOR    0x23  /* Selecciona el color de la barra */

#define M_BOTON_ACTIVA   0x20  /* Activa/Desactiva el boton */
#define M_BOTON_SEL      0x21  /* Selecciona/Deselecciona el boton */
#define M_BOTON_ESTADO   0x22  /* Pregunta el estado del boton */

#define M_ESTADO_TEXTO   0x20  /* Cambia el texto del estado */
#define M_ESTADO_MINMAX  0x21  /* Selecciona los rangos min..max */
#define M_ESTADO_POS     0x22  /* Posición actual del indicador */
#define M_ESTADO_COLOR1  0x23
#define M_ESTADO_COLOR2  0x24
#define M_ESTADO_BORDE   0x25
#define M_ESTADO_TIPO    0x26

#define M_EDITABLE_TAM   0x20
#define M_EDITABLE_TEXTO 0x21
#define M_EDITABLE_POS   0x22
#define M_EDITABLE_FUNC  0x23

#define M_MENU_DESCRIP   0x20  /* Selecciona descripción del menú */

#define M_MULTIPLE_PREG  0x21  /* Pregunta el elemento seleccionado */
#define M_MULTIPLE_SEL   0x22  /* Selecciona un elemento */
#define M_MULTIPLE_TIPO  0x23  /* Selecciona el tipo de letra */
#define M_MULTIPLE_DESC  0x24  /* Describe el menú */

#define M_LISTA_DESC     0x20  /* Describe la lista */
#define M_LISTA_TIPO     0x21  /* Tipo de letra de la lista */
#define M_LISTA_CARPETA  0x22  /* Carpeta que muestra la lista */
#define M_LISTA_TOTAL    0x23  /* Pregunta total de visualizados */
#define M_LISTA_DESPLAZA 0x24  /* Desplaza la lista */
#define M_LISTA_EDICION  0x25
#define M_LISTA_COLUMNAS 0x26  /* Marca las columnas de la lista */

#define M_HERRAMIENTAS   0x21  /* Selecciona el conjunto de herramientas */

#define M_ARCHIVERO      0x40  /* Selecciona la carpeta mostrada */

#define M_COLOR_SEL      0x20  /* Selecciona el color mostrado */
#define M_COLOR_PREGUNTA 0x21  /* Pregunta el color actual */

/*
** Estilos de ventana estándar
*/
#define E_BOTON_CERRAR   0x0001 /* Con boton para cerrar */
#define E_BOTON_AYUDA    0x0002 /* Con boton para ayúda */
#define E_BOTON_REDIM    0x0004 /* Con boton para redimensionar */
#define E_REDIM_HOR      0x0008 /* Acepta redimensión horizontal */
#define E_REDIM_VER      0x0010 /* Acepta redimensión vertical */
#define E_NO_ACOMODAR    0x0020 /* No acomoda en la pantalla */
#define E_IZQ            0x0080 /* Alineación a la izquierda */
#define E_DER            0x00C0 /* Alineación a la derecha */
#define E_ARRIBA         0x0200 /* Alineación hacia arriba */
#define E_ABAJO          0x0300 /* Alineación hacia abajo */
#define E_SIN_TECLADO    0x0400 /* No enfoca el teclado */
#define E_SIN_TITULO     0x0800 /* Sin barra de título */

#define E_ARCHIVERO_LEE   0x00050000
#define E_ARCHIVERO_GRABA 0x00020000

#define E_IMPRIME_NORMAL  0x00000000
#define E_IMPRIME_PAGINA  0x00010000
#define E_IMPRIME_PAGS    0x00020000

/*
** Variables definidas de ventanas
*/
#define V_PUERTA         (-29)  /* Puerta de impresión */
#define V_REDIR          (-24)  /* Función de redirección para impresora */
#define V_TEXTO          (-20)  /* Texto asociado */
#define V_MODO           (-19)  /* Modo de gráficos */
#define V_TICKS          (-18)  /* Ticks para alarma del reloj */
#define V_ESTILO         (-17)  /* Estilo de la ventana */
#define V_TAREA          (-16)  /* Tarea que creó la ventana */
#define V_TAMY           (-15)  /* Tamaño Y */
#define V_TAMX           (-14)  /* Tamaño X */
#define V_POSY           (-13)  /* Posición Y */
#define V_POSX           (-12)  /* Posición X */
#define V_ID             (-11)  /* Número de ID */
#define V_RECORTE        (-10)  /* Apuntador a información de recorte */
#define V_CLASE          ( -9)  /* Apuntador a clase */
#define V_PINCEL         ( -8)  /* Color 32 bits asociado */
#define V_TIPO           ( -7)  /* Tipo de letra asociado */
#define V_VIDEOY         ( -6)  /* Posición Y en el video */
#define V_VIDEOX         ( -5)  /* Posición X en el video */
#define V_HIJA           ( -4)  /* Apuntador a primera ventana hija */
#define V_HERMANA        ( -3)  /* Apuntador a siguiente ventana hermana */
#define V_PROPIETARIA    ( -2)  /* Apuntador a ventana propietaria */
#define V_MADRE          ( -1)  /* Apuntador a ventana madre */

/*
** Variables especiales de una ventana estándar
*/
#define V_MAESTRA        (  0)  /* Apuntador a ventana maestra */
#define V_TITULO         (  1)  /* Apuntador a ventana de título */
#define V_CERRAR         (  2)  /* Apuntador a boton de cerrar */
#define V_AYUDA          (  3)  /* Apuntador a boton de ayuda */
#define V_REDIMENSION    (  4)  /* Apuntador a boton de redimensión */

/*
** Variables definidas del sistema
*/
#define S_TAREA_ACTUAL    0x00
#define S_VENTANA_RAIZ    0x03
#define S_TICKS           0x0F
#define S_VIDEO_COLORES   0x1B
#define S_V_COLOR_CLARO   0x2D
#define S_TIPO_SISTEMA    0x2E
#define S_V_COLOR_OBSCURO 0x2F
#define S_VIDEO_TX        0x37
#define S_VIDEO_TY        0x38
#define S_B_COLOR_FONDO   0x3A
#define S_V_COLOR_FONDO   0x3D
#define S_B_COLOR_LETRAS  0x3E
#define S_E_COLOR_FONDO   0x93
#define S_E_TIPO_LETRA    0x94
#define S_E_COLOR_LETRAS  0x95
#define S_B_COLOR_CLARO   0x96
#define S_B_COLOR_OBSCURO 0x97
#define S_B_TIPO_LETRA    0x9C
#define S_L_TIPO_LETRA    0x9E  /* Tipo de letra de las listas */

/*
** Estructura de cada elemento en una lista
*/
struct control_lista {
  char *icono;                  /* Apuntador a un icono 16x16 en 64k colores */
  wchar_t *texto;               /* Apuntador a una cadena de texto */
  int banderas;                 /* Indica el tipo de elemento o su estado */
  int usuario;                  /* Disponible para el usuario */
};

/*
** Estructura dada por la ventana de impresión
*/
struct impresion {
  int puerta_salida;            /* Puerta de salida */
  int tipo_de_papel;            /* Tipo de papel */
  int copias;                   /* Número de copias */
  int orientacion;              /* Orientación (0= Normal, 1= Apaisado) */
  int pagina_inicial;           /* Página inicial */
  int pagina_final;             /* Página final */
  int reservado[2];             /* No utilizados */
  wchar_t impresora[1];         /* Nombre de la impresora */
};

struct info_impresora {
  int reservado[1];
  int resolucion;               /* Resolución X y Y (combinada) */
};

extern void reubica_raton(void *, int, int);
extern void dibuja_rectangulo(void *, int, int, int, int);
extern void *crea_ventana(void *, void *, int, wchar_t *,
                          int, int, int, int, int);
extern void borra_ventana(void *);
extern void pone_bitmap_16t(void *, void *, int, int, int, int);
extern void foco_teclado(void *);
extern void dibuja_linea(void *, int, int, int, int);
extern void dibuja_bitmap(void *, void *, int, int, int, int);
extern void dibuja_bitmap_16(void *, void *, int, int, int, int);
extern void pone_alarma(void *, int);
extern int dibuja_letra(void *, int, int, int);
extern void medidas_letra(void *, int, int *);
extern void bitblt(int, int, void *, int, int, void *, int, int);
extern void escribe_sistema(int, int);
extern void captura_raton(void *);
extern void ventana_al_frente(void *);
extern void *busca_clase(wchar_t *, int);
extern void *crea_clase(wchar_t *texto, int (*func)(void *, int, int, int),
                        void *, int, int, int);
extern void redimensiona_ventana(void *, int, int, int);
extern void posiciona_ventana(void *, int, int);
extern void borra_clase(void *);
extern void escribe_variable(void *, int, int);
extern int leer_variable(void *, int);
extern void dibuja_rectangulo_invertido(void *, int, int, int, int);
extern void *ventana_estandar(void *, void *, wchar_t *, int, int, int);
extern void activa_ventana(void *, int);
extern void dibuja_bitmap_32(void *, void *, int, int, int, int);
extern void selecciona_cursor(int, void *);
extern void dibuja_elipse(void *, int, int, int, int);
extern void dibuja_elipse_rellena(void *, int, int, int, int);
extern void dibuja_linea_gruesa(void *, int, int, int, int, int);
extern void selecciona_modo(void *, int);
extern void obtener_xy(void *, int, int, void *, int *, int *);
extern void *busca_ventana(void *, int);
extern wchar_t *cambia_texto(void *, wchar_t *);
extern void *info_impresora(void *);
extern int prepara_impresora(void *);
extern int inicia_pagina(void *, int, int, int);
extern int termina_pagina(void *);
extern int libera_impresora(void *);
extern void dibuja_borde(void *, int, int, int, int, int, int, int, int);
extern int crea_dialogo(void *, int (*func)(void *, int, int, int), wchar_t *,
                        int, int, wchar_t *, void *, void *);
extern void dibuja_bitmap_expansible(void *, int, int, int, int, int, int, int,
                                     int, int, int, int, void *);
extern void optimiza_ventana(void *, int, int, int);
extern int ejecuta_tarea(wchar_t *);
extern int memoria_libre(void);
extern void *malloc(int);
extern void free(void *);
extern void optimiza_memoria(void);
extern void *realloc(void *, int);
extern int mkdir(wchar_t *);
extern int unlink(wchar_t *);
extern int open(wchar_t *, int);
extern void obtener_hora(int *);
extern int close(int);
extern int read(int, void *, int);
extern int write(int, void *, int);
extern wchar_t *temporal();
extern void poner_hora(int, int);
extern void *lista_tipos(int *);
extern void *crea_tipo(wchar_t *, int, int, int, void *, int);
extern void elimina_tipo(void *);
extern void aviso_problema(int, wchar_t *);
extern void crea_aviso(wchar_t *, wchar_t *, void *, int, void *);
extern void *ventana_impresora(void *, int, int);
extern int abre_configuracion(wchar_t *, int);
extern int lee_configuracion(int, wchar_t *, void *, int);
extern int escribe_configuracion(int, wchar_t *, void *, int);
extern void cierra_configuracion(int);
extern void *portapapeles();
extern int crea_recorte(int);
extern int lee_recorte(int *);
extern int strlen(char *);
extern void strcpy(char *, char *);
extern void strcat(char *, char *);
extern int memcmp(void *, void *, int);
extern void *info_tarea(int);

#asm
.global _reubica_raton
_reubica_raton:
sub gr96,lr2,20
load 0,4,gr96,gr96
add lr3,lr3,gr96
sub gr96,lr2,24
load 0,4,gr96,gr96
add lr4,lr4,gr96
const gr96,-1073743816
consth gr96,-1073743816
load 0,4,gr96,gr96
srl gr97,gr96,16
consth gr96,0
sub gr92,lr3,gr96
jmpi lr0
sub gr91,lr4,gr97

.global _dibuja_rectangulo
_dibuja_rectangulo:
const gr121,5
asneq 66,gr1,gr1

.global _crea_ventana
_crea_ventana:
const gr121,6
asneq 66,gr1,gr1

.global _borra_ventana
_borra_ventana:
const gr121,7
asneq 66,gr1,gr1

.global _pone_bitmap_16t
_pone_bitmap_16t:
const gr121,8
asneq 66,gr1,gr1

.global _foco_teclado
_foco_teclado:
const gr121,9
asneq 66,gr1,gr1

.global _dibuja_linea
_dibuja_linea:
const gr121,13
asneq 66,gr1,gr1

.global _dibuja_bitmap
_dibuja_bitmap:
const gr121,14
asneq 66,gr1,gr1

.global _dibuja_bitmap_16
_dibuja_bitmap_16:
const gr121,15
asneq 66,gr1,gr1

.global _pone_alarma
_pone_alarma:
const gr121,18
asneq 66,gr1,gr1

.global _dibuja_letra
_dibuja_letra:
const gr121,19
asneq 66,gr1,gr1

.global _medidas_letra
_medidas_letra:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,36
add lr2,lr6,0
add lr3,lr7,0
call lr0,__sv
const gr121,21
store 0,4,gr96,lr8
add lr8,lr8,4
store 0,4,gr97,lr8
add lr8,lr8,4
store 0,4,gr98,lr8
add lr8,lr8,4
store 0,4,gr99,lr8
add lr8,lr8,4
store 0,4,gr100,lr8
add lr8,lr8,4
store 0,4,gr101,lr8
add lr8,lr8,4
store 0,4,gr102,lr8
add lr8,lr8,4
store 0,4,gr103,lr8
add gr1,gr1,16
nop
jmpi lr0
asleu 65,lr1,gr127

.global _bitblt
_bitblt:
const gr121,25
asneq 66,gr1,gr1

.global _escribe_sistema
_escribe_sistema:
const gr121,28
asneq 66,gr1,gr1

.global _captura_raton
_captura_raton:
const gr121,29
asneq 66,gr1,gr1

.global _ventana_al_frente
_ventana_al_frente:
const gr121,30
asneq 66,gr1,gr1

.global _busca_clase
_busca_clase:
const gr121,31
asneq 66,gr1,gr1

.global _redimensiona_ventana
_redimensiona_ventana:
const gr121,33
asneq 66,gr1,gr1

.global _posiciona_ventana
_posiciona_ventana:
const gr121,34
asneq 66,gr1,gr1

.global _borra_clase
_borra_clase:
const gr121,35
asneq 66,gr1,gr1

.global _escribe_variable
_escribe_variable:
const gr121,36
asneq 66,gr1,gr1

.global _leer_variable
_leer_variable:
const gr121,37
asneq 66,gr1,gr1

.global _dibuja_rectangulo_invertido
_dibuja_rectangulo_invertido:
const gr121,38
asneq 66,gr1,gr1

.global _activa_ventana
_activa_ventana:
const gr121,41
asneq 66,gr1,gr1

.global _dibuja_bitmap_32
_dibuja_bitmap_32:
const gr121,44
asneq 66,gr1,gr1

.global _selecciona_cursor
_selecciona_cursor:
const gr121,46
asneq 66,gr1,gr1

.global _dibuja_elipse
_dibuja_elipse:
const gr121,48
asneq 66,gr1,gr1

.global _dibuja_elipse_rellena
_dibuja_elipse_rellena:
const gr121,49
asneq 66,gr1,gr1

.global _dibuja_linea_gruesa
_dibuja_linea_gruesa:
const gr121,50
asneq 66,gr1,gr1

.global _selecciona_modo
_selecciona_modo:
const gr121,51
asneq 66,gr1,gr1

.global _obtener_xy
_obtener_xy:
sub gr1,gr1,32
asgeu 64,gr1,gr126
add lr1,gr1,64
add lr2,lr10,0
add lr3,lr11,0
add lr4,lr12,0
add lr5,lr13,0
call lr0,__sv
const gr121,53
store 0,4,gr96,lr14
nop
store 0,4,gr97,lr15
add gr1,gr1,32
nop
jmpi lr0
asleu 65,lr1,gr127

.global _busca_ventana
_busca_ventana:
const gr121,54
asneq 66,gr1,gr1

.global _cambia_texto
_cambia_texto:
const gr121,55
asneq 66,gr1,gr1

.global _info_impresora
_info_impresora:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,28
add lr2,lr6,0
const lr3,0
call lr0,__sv
const gr121,56
add gr1,gr1,16
nop
jmpi lr0
asleu 65,lr1,gr127

.global _prepara_impresora
_prepara_impresora:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,28
add lr2,lr6,0
const lr3,1
call lr0,__sv
const gr121,56
add gr1,gr1,16
nop
jmpi lr0
asleu 65,lr1,gr127

.global _inicia_pagina
_inicia_pagina:
sub gr1,gr1,32
asgeu 64,gr1,gr126
add lr1,gr1,56
add lr2,lr10,0
const lr3,2
add lr4,lr11,0
add lr5,lr12,0
add lr6,lr13,0
call lr0,__sv
const gr121,56
add gr1,gr1,32
nop
jmpi lr0
asleu 65,lr1,gr127

.global _termina_pagina
_termina_pagina:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,28
add lr2,lr6,0
const lr3,3
call lr0,__sv
const gr121,56
add gr1,gr1,16
nop
jmpi lr0
asleu 65,lr1,gr127

.global _libera_impresora
_libera_impresora:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,28
add lr2,lr6,0
const lr3,4
call lr0,__sv
const gr121,56
add gr1,gr1,16
nop
jmpi lr0
asleu 65,lr1,gr127

.global _dibuja_borde
_dibuja_borde:
const gr121,58
asneq 66,gr1,gr1

.global _crea_dialogo
_crea_dialogo:
const gr121,60
asneq 66,gr1,gr1

.global _dibuja_bitmap_expansible
_dibuja_bitmap_expansible:
const gr121,63
asneq 66,gr1,gr1

.global _optimiza_ventana
_optimiza_ventana:
const gr121,65
asneq 66,gr1,gr1

.global _ejecuta_tarea
_ejecuta_tarea:
const gr121,66
asneq 66,gr1,gr1

.global _memoria_libre
_memoria_libre:
sub gr1,gr1,16
asgeu 64,gr1,gr126
add lr1,gr1,24
call lr0,__am
const gr121,1
add gr1,gr1,16
add gr96,gr97,0
jmpi lr0
asleu 65,lr1,gr127

.global _optimiza_memoria
_optimiza_memoria:
const gr121,5
asneq 69,gr1,gr1

.global _realloc
_realloc:
const gr121,6
__am:
asneq 69,gr1,gr1
nop

.global _mkdir
_mkdir:
const gr121,4
asneq 72,gr1,gr1

.global _unlink
_unlink:
const gr121,6
asneq 72,gr1,gr1

.global _link
_link:
const gr121,7
asneq 72,gr1,gr1

.global _open
_open:
const gr121,8
asneq 72,gr1,gr1

.global _obtener_hora
_obtener_hora:
const gr121,10
asneq 72,gr1,gr1

.global _close
_close:
const gr121,11
asneq 72,gr1,gr1

.global _read
_read:
const gr121,13
asneq 72,gr1,gr1

.global _write
_write:
const gr121,14
asneq 72,gr1,gr1

.global _temporal
_temporal:
const gr121,24
asneq 72,gr1,gr1

.global _poner_hora
_poner_hora:
const gr121,27
asneq 72,gr1,gr1

.global _lista_tipos
_lista_tipos:
const gr121,6
asneq 74,gr1,gr1

.global _crea_tipo
_crea_tipo:
const gr121,7
asneq 74,gr1,gr1

.global _elimina_tipo
_elimina_tipo:
const gr121,8
asneq 74,gr1,gr1

.global _aviso_problema
_aviso_problema:
const gr121,1
asneq 75,gr1,gr1

.global _crea_aviso
_crea_aviso:
const gr121,2
asneq 75,gr1,gr1

.global _portapapeles
_portapapeles:
const gr121,1
asneq 76,gr1,gr1

.global _crea_recorte
_crea_recorte:
const gr121,2
asneq 76,gr1,gr1

.global _lee_recorte
_lee_recorte:
const gr121,3
asneq 76,gr1,gr1

.global _abre_configuracion
_abre_configuracion:
const gr121,1
asneq 77,gr1,gr1

.global _lee_configuracion
_lee_configuracion:
const gr121,2
asneq 77,gr1,gr1

.global _escribe_configuracion
_escribe_configuracion:
const gr121,3
asneq 77,gr1,gr1

.global _cierra_configuracion
_cierra_configuracion:
const gr121,4
asneq 77,gr1,gr1

.global _ventana_impresora
_ventana_impresora:
const gr121,1
asneq 78,gr1,gr1

.global _strlen
_strlen:
or gr97,lr2,0
_strlen1:
load 0,20,gr96,gr97
exbyte gr96,gr96,0
cpeq gr96,gr96,0
jmpf gr96,_strlen1
add gr97,gr97,1
sub gr97,gr97,1
jmpi lr0
sub gr96,gr97,lr2

.global _strcat
_strcat:
load 0,20,gr96,lr2
exbyte gr96,gr96,0
cpeq gr97,gr96,0
jmpf gr97,_strcat
add lr2,lr2,1
sub lr2,lr2,1
.global _strcpy
_strcpy:
load 0,20,gr96,lr3
exbyte gr96,gr96,0
load 0,20,gr97,lr2
inbyte gr97,gr97,gr96
store 0,4,gr97,lr2
add lr2,lr2,1
cpeq gr96,gr96,0
jmpf gr96,_strcpy
add lr3,lr3,1
jmpi lr0
nop

.global _memcmp
_memcmp:
sub lr4,lr4,2
_memcmp1:
load 0,20,gr96,lr2
exbyte gr96,gr96,0
load 0,20,gr97,lr3
exbyte gr97,gr97,0
cpeq gr98,gr96,gr97
jmpt gr98,_memcmp2
add lr2,lr2,1
cpgt gr98,gr96,gr97
jmpti gr98,lr0
const gr96,1
jmpi lr0
const gr96,-1
_memcmp2:
jmpfdec lr4,_memcmp1
add lr3,lr3,1
jmpi lr0
const gr96,0
#endasm
