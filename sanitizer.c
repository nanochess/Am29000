/*
** Disk image sanitizer
**
** by Oscar Toledo G.
** https://nanochess.org/
**
** Creation date: Jul/29/2026.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE     512

unsigned char image[1474560];

int total_sectors;
int block_size;
int max_blocks;
int directory_block;

char global_path[256];

/*
 ** Read a FAT entry
 */
int read_fat(int block)
{
    unsigned char *p;
    
    p = image + 1 * block_size * SECTOR_SIZE + block * 2;
    return (p[0] << 8) | p[1];
}

/*
 ** Optimize a directory
 */
void optimize_directory(int block, char *path)
{
    unsigned char *p;
    unsigned char *p1;
    unsigned char *p2;
    int size;
    int c;
    
    p = image + block * block_size * SECTOR_SIZE;
    p1 = p;
    p2 = p + block_size * SECTOR_SIZE;
    if (read_fat(block) != 0xffff) {
        fprintf(stderr, "Cannot optimize multi-block directories\n");
        exit(1);
    }
    p1 = p;
    while (p[0]) {
        if (p[0] == 0x80) {   /* Deleted entry */
            p += 64;
            continue;
        }
        strcpy(path, (char *) p);
        if (p1 != p)    /* Displace old directory entry backward */
            memcpy(p1, p, 64);
        p1 += 64;
        block = (p[0x38] << 24) | (p[0x39] << 16) | (p[0x3a] << 8) | p[0x3b];
        if (p[0x33] == 0x08) { /* Subdirectory */
            strcat(path, "/");
            fprintf(stderr, "%s\n", global_path);
            optimize_directory(block, path + strlen(path));
        } else {    /* File */
            size = (p[0x3c] << 24) | (p[0x3d] << 16) | (p[0x3e] << 8) | p[0x3f];
            fprintf(stderr, "%s (%d) block %d", global_path, size, block);
            while (size) {
                if (size <= block_size * SECTOR_SIZE) { /* Last block */
                    if (size < block_size * SECTOR_SIZE) {
                        c = block_size * SECTOR_SIZE - size;
                        memset(image + block * block_size * SECTOR_SIZE + size, 0xfc, c);
                    }
                    block = read_fat(block);
                    fprintf(stderr, ",%d", block);
                    if (block != 0xffff) {
                        fprintf(stderr, "Last block not marked as end. Consistency error\n");
                        exit(1);
                    }
                    break;
                }
                block = read_fat(block);
                fprintf(stderr, ",%d", block);
                if (block >= max_blocks) {
                    fprintf(stderr, "File shorter. Consistency error\n");
                    exit(1);
                }
                size -= block_size * SECTOR_SIZE;
            }
            fprintf(stderr, "\n");
        }
        p += 64;
    }
    
    /*
     ** Erase the remaining of the block.
     */
    while (p1 < p2) {
        memset(p1, 0, 64);
        p1 += 64;
    }
}

/*
** Main program
*/
int main(int argc, char *argv[])
{
	FILE *disk;
    int c;
    
    total_sectors = 80 * 2 * 18;
    block_size = 8;
	if (argc < 2) {
		fprintf(stderr, "Usage: sanitizer disk.img\n");
		exit(1);
	}
	disk = fopen(argv[1], "rb+");
    if (fread(image, 1, sizeof(image), disk) < sizeof(image)) {
        fprintf(stderr, "Disk image too small\n");
        fclose(disk);
        exit(1);
    }
    
    total_sectors = 80 * 2 * 18;
    block_size = 8;
    max_blocks = total_sectors / block_size;
    directory_block = (block_size * SECTOR_SIZE + max_blocks * 2 + (block_size * SECTOR_SIZE - 1)) / (block_size * SECTOR_SIZE);
    
    /*
     ** Erase unused blocks.
     */
    for (c = 0; c < max_blocks; c++) {
        if (read_fat(c) == 0) {
            memset(image + c * block_size * SECTOR_SIZE, 0xfc, block_size * SECTOR_SIZE);
        }
    }
    
    /*
     ** Optimize root directory.
     */
    global_path[0] = '/';
    optimize_directory(directory_block, global_path + 1);
    
    fseek(disk, 0, SEEK_SET);
	fwrite(image, 1, sizeof(image), disk);
	fclose(disk);
	exit(0);
}
