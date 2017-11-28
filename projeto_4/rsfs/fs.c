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
#define TAM_FAT 65536

unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];

int fs_init() {
 	int i = 0, format = 0, j = 0;
 	char *buffer_fat = (char *) fat;
 	char *buffer_dir = (char *) dir;
 	char op;
   
    // carrega dados do disco
    for (i = 0; i < 256; i++){
		if(!bl_read(i, &buffer_fat[i*512]))
			return 0;
    }
	

	// VERIFICA SE O DISCO ESTÁ FORMATADO
	for (i = 0; i < 32; i++){
		if(fat[i] != A_FAT){
			format = 1;
			break;
		}
	}
	
	if(fat[32] != A_DIR)
		format = 1;
	
	for (i = 33; i < TAM_FAT; i++){
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
        else {
            for (i = 0, j = 256; i < 128; i++){
                if(!bl_read(j + i, &buffer_dir[i*512]))
                    return 0;
            }
        }
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
    //printf("Função não implementada: fs_free\n");
    int cont = 0, mem_livre;

    for(int i = 33; i < bl_size(); i++){
        if(fat[i] == LIVRE)
            cont++;
    }

    mem_livre = cont * 512;

    return mem_livre;
}

int fs_list(char *buffer, int size) {
    char buffer_int[100];
    memset(buffer_int, '\0', 100);
    strcpy(buffer, "\0");

    // pensar nisso depois 
    for(int i = 0; i < 128; i++){
        if(dir[i].used){
            strcat(buffer, dir[i].name);
            strcat(buffer, "\t\t");
            sprintf(buffer_int, "%d", dir[i].size);
            strcat(buffer, buffer_int);
            strcat(buffer, "\n");
            
            // TODO: TIRAR ESSE PRINTF DEPOIS
            //printf("%s\n", dir[i].name);
            printf("%s", buffer);
        }
    }
    return 0;
}

// tratar o caso de strlen(file_name) > 25
int fs_create(char* file_name) {
    int flag = 0, pos_dir, cont_ocupado = 0, pos_fat;    
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;


    // procura primeira posição livre no vetor de diretórios
    for(int i = 0; i < 128; i++){
        if(!strcmp(file_name, dir[i].name))
            return 0;
        if(dir[i].used == DIR_LIVRE && flag != 1){
            pos_dir = i;
            flag = 1;
        } else if(dir[i].used == DIR_USADO)
            cont_ocupado++;
    }

    if(cont_ocupado == 127){
        printf("Não há espaço!\n");
        return 0;
    }

    // procura o primeiro espaço livre na FAT
    for(int i = 33; i < TAM_FAT; i++){
        if(fat[i] == LIVRE){
            pos_fat = i;
            fat[i] = pos_dir; 
            // escreve no disco
            printf("buffer_fat = %s\n", &buffer_fat[pos_fat*512]);
            printf("buffer_dir = %s\n", &buffer_dir[pos_dir*512]);
            if(!bl_write(pos_fat*8, &buffer_fat[pos_fat*512]))
                return 0;
            break;
        }
    }

    printf("pos_fat = %d, pos_dir = %d\n", pos_fat, pos_dir);
    // adiciona as infos do diretório
    strcpy(dir[pos_dir].name, file_name);
    dir[pos_dir].size = 0;
    dir[pos_dir].used = DIR_USADO;
    dir[pos_dir].first_block = pos_fat;

    if(!bl_write(pos_dir, &buffer_dir[pos_dir*512]))
        return 0;

    return 0;
}

int fs_remove(char *file_name) {
  printf("Função não implementada: fs_remove\n");
  return 0;
}

/* a partir daqui é pra prox fase */
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

