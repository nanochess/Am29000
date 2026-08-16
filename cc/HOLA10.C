color(col)
  int col;
{
  putchar(0x33);
  if (col >= 8) {
    putchar(0x30 + (col - 8));
  } else
    putchar(0x30 + col);
}
