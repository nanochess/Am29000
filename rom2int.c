/*
** Convert a binary ROM into 32-bit words.
**
** by Oscar Toledo G.
** https://nanochess.org/
**
** Creation date: Aug/10/2026.
*/

#include <stdio.h>

/*
** Main program
*/
int main(void)
{
	FILE *input;
	FILE *output;
	unsigned char buffer[4];
	int c;

	input = fopen("1999/rom.bin", "rb");
	output = fopen("am29000/rom.c", "w");
	c = 0;
	fprintf(output, "#include <stdint.h>\n");
	fprintf(output, "\n");
	fprintf(output, "uint32_t rom_1999[] = {\n");
	while (fread(buffer, 1, 4, input)) {
		if ((c & 3) == 0) {
			fprintf(output, "\t");
		}
		fprintf(output, "0x%08x,", (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0]);
		if ((c & 3) == 3) {
			fprintf(output, "\n");
		} else {
			fprintf(output, " ");
		}
		c++;
	}
	fprintf(output, "};\n");
	fclose(output);
	fclose(input);
}
