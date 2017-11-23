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
 	int i = 0, format = 0;
 	char *buffer_fat = (char *) fat;
 	char op;
   
    // carrega dados do disco
    for (i = 0; i < 256; i++)
		if(!bl_read(i, &buffer_fat[i*512]))
			return 0;
	
	// VERIFICA SE O DISCO ESTÁ FORMATADO
	for (i = 0; i < 32; i++){
		if(fat[i] != A_FAT){
			format = 1;
			break;
		}
	}
	
	if(fat[32] != A_DIR)
		format = 1;
	
	for (i = 33; i < 65536; i++){
		if(fat[i] != LIVRE){
			format = 1;
			break;
		}
	}
	
	if(format){
		printf("O disco nao esta formatado.\nDeseja formata-lo? [y]=yes [n]=no\n");
		scanf("%c", &op);
		if(op == 'y')
			fs_format();
	}	
	return 1;
}

int fs_format() {
 	int i = 0, j;
 	char *buffer_fat = (char *) fat;
 	char *buffer_dir = (char *) dir;
  
  	// 70-77 SETA AS POSIÇÕES DA FAT, DO DIRETORIO E DOS ESPAÇOS LIVRES EM MEMÓRIA
  	for (i = 0; i < 32; i++)
  		fat[i] = A_FAT;
  	
  	fat[32] = A_DIR;
  
  	for (i = 33; i < 65536; i++)
  		fat[i] = LIVRE;
  		
	// INICIALIZA A STRUCT DIRETORIO
	for(i = 0; i < 128; i++){
	  	dir[i].used = DIR_LIVRE;
  		//dir[i].first_block = ULTIMO;
  		dir[i].size = 0;
  	}
  	
  	// ESCREVE NO ARQUIVO IMAGEM A FAT E O DIRETORIO
	for (i = 0; i < 256; i++)
		if(!bl_write(i, &buffer_fat[i*512]))
			return 0;

	for (i = 0, j = 256; i < 128; i++)
		if(!bl_write(j + i, &buffer_dir[i*512]))
			return 0;
	
	return 1;
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

