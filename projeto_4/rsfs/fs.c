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
	
	if(format){
        printf("O disco não está formatado\n");
    } else {
        for (i = 0, j = 256; i < 8; i++){
            if(!bl_read(j + i, &buffer_dir[i*512]))
                return 0;
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

	for (i = 0, j = 256; i < 8; i++)
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
    char buffer_mem[size];
    memset(buffer_int, '\0', 100);
    strcpy(buffer_mem, "\0");

    for(int i = 0; i < 128; i++){
        if(dir[i].used){
            strcat(buffer_mem, dir[i].name);
            strcat(buffer_mem, "\t\t");
            sprintf(buffer_int, "%d", dir[i].size);
            strcat(buffer_mem, buffer_int);
            strcat(buffer_mem, "\n");
        }
    }

    strcpy(buffer, buffer_mem);
    // TODO: TIRAR ESSE PRINTF DEPOIS
    printf("%s", buffer);
    return 0;
}

int fs_create(char* file_name) {
    int pos_dir, pos_fat, i, j;    
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;

    /* ----------- MANIPULAÇÃO EM MEMÓRIA ------------- */
    // busca a primeira posição livre na fat
    for(i = 33; i < TAM_FAT; i++){
        if(fat[i] == LIVRE){
            pos_fat = i;
            break;
        }
    }
    
    // busca a primeira posição livre no vetor de diretórios
    for(i = 0; i < 128; i++){
        if(dir[i].used == 0){
            pos_dir = i;
            break;
        }
    }
    
    // seta infos dir 
    dir[pos_dir].used = DIR_USADO;
    dir[pos_dir].size = 0;
    dir[pos_dir].first_block = pos_fat;
    strcpy(dir[pos_dir].name, file_name);

    // marca a fat com o último agrupamento do diretório
    fat[pos_fat] = ULTIMO;

    /* ----------- MANIPULAÇÃO EM DISCO ------------- */
    for(i = 0; i < 256; i++){
        if(!bl_write(i, &buffer_fat[i * 512]))
            return 0;
    }

    for(i = 0, j = 256; i < 8; i++){
        if(!bl_write(j + i, &buffer_dir[i * 512]))
            return 0;
    }

    return 0;
}

int fs_remove(char *file_name) {
    int pos_dir, pos_fat, i, j, flag = 0;
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;
    
    /* ----------- MANIPULAÇÃO EM MEMÓRIA ------------- */
    for(i = 0; i < 128; i++){
        if(!strcmp(dir[i].name, file_name)){
            pos_dir = i;
            pos_fat = dir[i].first_block;
            flag = 1;
            break;
        }
    }
    
    if(!flag){
        printf("Arquivo não encontrado\n");
        return 0;
    }
     
    memset(dir[pos_dir].name, '\0', strlen(dir[pos_dir].name));
    dir[pos_dir].first_block = 0;
    dir[pos_dir].used = DIR_LIVRE;
    dir[pos_dir].size = 0;
    
    fat[pos_fat] = LIVRE;

    /* ----------- MANIPULAÇÃO EM DISCO ------------- */
    for(i = 0; i < 256; i++) {
        if(!bl_write(i, &buffer_fat[i * 512]))
            return 0;
    }

    for(i = 0, j = 256; i < 8; i++) {
        if(!bl_write(j + i, &buffer_dir[i * 512]))
            return 0;
    }

    return 0;
}

/*
 *  Manter um buffer interno pra gerenciar os bytes que o usuário quer ler
 *  independente dos 4K que temos que puxar do disco.
 */
int fs_open(char *file_name, int mode) {
  printf("Função não implementada: fs_open\n");
  return -1;
}

int fs_close(int file)  {
  printf("Função não implementada: fs_close\n");
  return 0;
}

/*  Caso o buffer tiver cheio, ir na fat e procurar o próximo bloco livre.
 *  Aí é só recarregar o buffer e continuar a escrita.
 *  A posição de escrita é sempre na última posição porque não tem seek.
 */
int fs_write(char *buffer, int size, int file) {
  printf("Função não implementada: fs_write\n");
  return -1;
}

/*  > Lê dados do disco
 *  > Guarda no buffer
 *  > Cria um marcador de leitura pra esse buffer pq cada leitura vai
 *  retornar esses bytes e ver se existem
 *      > Ficar atento pra quando estiver perto do fim pq se passar do número
 *      de bytes do final, vai ter que recarregar o buffer com os prox 4096 
 *      bytes do disco
 *      > Checar fim do buffer do usuário, fim do arquivo e fim do buffer do 
 *      usuário
 */
int fs_read(char *buffer, int size, int file) {
  printf("Função não implementada: fs_read\n");
  return -1;
}

