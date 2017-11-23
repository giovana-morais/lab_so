/*
 * RSFS - Really Simple File System
 *
 * Copyright © 2010 Gustavo Maciel Dias Vieira
 * Copyright © 2010 Rodrigo Rocco Barbieri
 *
 * This file is part of RSFS.
 *
 * RSFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define CLUSTERSIZE 4096
#define LIVRE 	1
#define ULTIMO	2
#define A_FAT	3
#define A_DIR	4
#define DIR_LIVRE 0
#define DIR_USADO 1

unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];

int fs_init() {
    //printf("Função não implementada: fs_init\n");
    // checa se está formatado
    // carrega dados do disco
    char *buffer; 
    int i, setor = 0;

    buffer = (char *) fat;
    bl_read(0, buffer);

    // nada dá certo 
    printf("alo: %ld", sizeof(fat));

    return 1;
}

int fs_format() {
 	// printf("Função não implementada: fs_format\n");
 	int i = 0;
  
  	for (i = 0; i < 32; i++)
  		fat[i] = A_FAT;
  	
  	fat[32] = A_DIR;
  
  	for (i = 33; i < 65536; i++)
  		fat[i] = LIVRE;
  
	for(i = 0; i < 128; i++){
	  	dir[i].used = DIR_LIVRE;
  		dir[i].first_block = ULTIMO;
  		dir[i].size = 0;
  	}
  	
  	char *buffer_fat = (char *) fat;
	for (i = 0; i < 256; i++)
		bl_write(i,&buffer_fat[i*512]);

	char *buffer_dir = (char *) dir;
	for (i = 0; i < sizeof(dir); i++)
		bl_write(i,&buffer_dir[i*512]);
	
  
  return 0;
}

int fs_free() {
  printf("Função não implementada: fs_free\n");
  return 0;
}

int fs_list(char *buffer, int size) {
  printf("Função não implementada: fs_list\n");
  return 0;
}

int fs_create(char* file_name) {
  printf("Função não implementada: fs_create\n");
  return 0;
}

int fs_remove(char *file_name) {
  printf("Função não implementada: fs_remove\n");
  return 0;
}

int fs_open(char *file_name, int mode) {
  printf("Função não implementada: fs_open\n");
  return -1;
}

int fs_close(int file)  {
  printf("Função não implementada: fs_close\n");
  return 0;
}

int fs_write(char *buffer, int size, int file) {
  printf("Função não implementada: fs_write\n");
  return -1;
}

int fs_read(char *buffer, int size, int file) {
  printf("Função não implementada: fs_read\n");
  return -1;
}

