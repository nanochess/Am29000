/*
** Visualización de texto formateado
**
** por Oscar Toledo Gutiérrez
**
** © Copyright Oscar Toledo G.1999
**
** Creación: 09-abr-1999.
*/

void *cache_tipos[56];

void visualizar_html(struct contexto *contexto)
{
  void *ventana = contexto->ventana;
  struct elemento_generico *elemento;
  struct elemento_texto *texto;
  struct elemento_linea *linea;
  struct elemento_imagen *imagen;
  struct elemento_tabla *tabla;
  int base_x = 0; /* !!! */
  int base_y = 0; /* !!! */
  int x, y;
  int numero_de_tipo;

  elemento = contexto->lista_visual;
  while (elemento != NULL) {
    switch (elemento->tipo) {
      case E_TEXTO:
        texto = elemento;
        x = texto->x - base_x;
        y = texto->y - base_y;
        numero_de_tipo = (texto->tipo_de_letra & T_MASCARA) |
                         (texto->tam << 3);
        if (cache_tipos[numero_de_tipo] == NULL) {
          cache_tipos[numero_de_tipo] =
            crea_tipo(tipos_de_letra[texto->tipo_de_letra & T_MASCARA],
                      tam_de_letra[texto->tam], 100, 100, NULL, 0);

        }
        if (cache_tipos[numero_de_tipo] != NULL) {
          sel_tipo(ventana, cache_tipos[numero_de_tipo]);
          sel_color(ventana, texto->color);
          /* !!! Dibujar subrayado */
          /* !!! Dibujar línea sobrepuesta */
          ilustra_texto(ventana, texto->texto, x, y);
        }
        break;
      case E_LINEA:
        linea = elemento;
        break;
      case E_IMAGEN:
        imagen = elemento;
        break;
      case E_TABLA:
        tabla = elemento;
        break;
    }
    elemento = elemento->siguiente;
  }
}

/*
** Libera los tipos del cache
*/
void libera_cache_tipos(void)
{
  int a;

  for (a = 0; a < 56; a++) {
    if (cache_tipos[a] != NULL) {
      elimina_tipo(cache_tipos[a]);
      cache_tipos[a] = NULL;
    }
  }
}
