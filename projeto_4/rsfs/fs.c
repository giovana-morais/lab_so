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

/* Grupo:
 *      Gabriel de Paula
 *      Giovana Morais
 *      Mateus Abreu
 *      Thiago Borges
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
#define QTD_SETOR_DIR 8
#define INICIO_DIR 256

unsigned short fat[65536];

typedef struct {
       char used;
       char name[25];
       unsigned short first_block;
       int size;
} dir_entry;

dir_entry dir[128];

/*
 * Struct com identificadores de leitura, escrita e seus respectivos
 * marcadores internos
 * first_block : -1 para o caso de estar vazio
 * f_mode   :   0 para open
 *              1 para read
 *              2 para write
 *              -1 caso esteja vazio
 * cont_leitura : marcador de leitura
 */
typedef struct {
	  unsigned short first_block;
	  int f_mode;
    int cont_leitura;
} id_arquivo;

id_arquivo id_arq[128];

/* Carrega informações existentes para as estruturas em memória e checa por
 * possíveis inconsistências. Nesse caso, avisa que o disco não está
 * formatado */
int fs_init() {
 	int i = 0, format = 0, j = 0;
 	char *buffer_fat = (char *) fat;
 	char *buffer_dir = (char *) dir;

    // carrega dados do disco
    for (i = 0; i < INICIO_DIR; i++){
		if(!bl_read(i, &buffer_fat[i*SECTORSIZE]))
			return 0;
    }

	// VERIFICA SE O DISCO ESTÁ FORMATADO
	for (i = 0; i < 32; i++){
		if(fat[i] != A_FAT){
			format = 1;
			break;
		}
	}

	if(format)
        printf("O disco não está formatado\n");
    else {
        for (i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++){
            if(!bl_read(j + i, &buffer_dir[i*SECTORSIZE]))
                return 0;
        }
    }

    for(i = 0; i < 128; i++){
        id_arq[i].first_block = DIR_LIVRE;
        id_arq[i].f_mode = -1;
        id_arq[i].cont_leitura = 0;
    }
	return 1;
}

/* Formata o disco */
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
  		dir[i].size = 0;
      dir[i].name[0] = '\0';
      dir[i].first_block = DIR_LIVRE;
  	}

  	// ESCREVE NO ARQUIVO IMAGEM A FAT E O DIRETORIO
	for (i = 0; i < INICIO_DIR; i++)
		if(!bl_write(i, &buffer_fat[i*SECTORSIZE]))
			return 0;

	for (i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++)
		if(!bl_write(j + i, &buffer_dir[i*SECTORSIZE]))
			return 0;

	return 1;
}

/* Faz a contagem da memória livre */
int fs_free() {
    int cont = 0, mem_livre;

    for(int i = 33; i < bl_size(); i++){
        if(fat[i] == LIVRE)
            cont++;
    }

    mem_livre = cont * SECTORSIZE;

    return mem_livre;
}

/* Lista o nome e o tamanho dos arquivos existentes */
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
    return 1;
}

/* Cria um arquivo de tamanho 0 na primeira posição livre encontrada */
int fs_create(char* file_name) {
    int pos_dir = -1, pos_fat, i, j, flag = 0;
    char *buffer_fat = (char*) fat;
    char *buffer_dir = (char*) dir;

    // verifica se o arquivo já existe na memória
    for(i = 0; i < 128; i++){
        if(!strcmp(dir[i].name, file_name)){
            pos_dir = i;
            pos_fat = dir[i].first_block;
            flag = 1;
            break;
        }
    }

    if(flag){
        printf("Arquivo já existe\n");
        return 0;
    }
    /* ----------- MANIPULAÇÃO EM MEMÓRIA ------------- */
    // busca a primeira posição livre na fat
    pos_fat = procura_fat();

    // busca a primeira posição livre no vetor de diretórios
    for(i = 0; i < 128; i++){
        if(dir[i].used == 0){
            pos_dir = i;
            break;
        }
    }

    /* VERIFICAÇÕES */
    if(i == 127 && pos_dir == -1){
        printf("Não há espaço livre!\n");
        return 0;
    }

    if(strlen(file_name) > 25){
        printf("Nome do arquivo precisa ter menos de 25 caracteres!\n");
        return 0;
    }

    // seta infos dir
    dir[pos_dir].used = DIR_USADO;
    dir[pos_dir].size = 0;
    dir[pos_dir].first_block = pos_fat;
    strcpy(dir[pos_dir].name, file_name);

    // marca a fat com o último agrupamento do diretório
    fat[pos_fat] = ULTIMO;

    /* ----------- MANIPULAÇÃO EM DISCO ------------- */
    for(i = 0; i < INICIO_DIR; i++){
        if(!bl_write(i, &buffer_fat[i * SECTORSIZE]))
            return 0;
    }

    for(i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++){
        if(!bl_write(j + i, &buffer_dir[i * SECTORSIZE]))
            return 0;
    }

    return 0;
}

/* Remove um arquivo de nome file_name */
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
    for(i = 0; i < INICIO_DIR; i++) {
        if(!bl_write(i, &buffer_fat[i * SECTORSIZE]))
            return 0;
    }

    for(i = 0, j = INICIO_DIR; i < QTD_SETOR_DIR; i++) {
        if(!bl_write(j + i, &buffer_dir[i * SECTORSIZE]))
            return 0;
    }

    return 0;
}

/*
 *  Manter um buffer interno pra gerenciar os bytes que o usuário quer ler
 *  independente dos 4K que temos que puxar do disco.
 */
int fs_open(char *file_name, int mode) {
	//open prepara estruturas para serem utilizadas no read e write, gerar um identificador pro arquivo.
	int i = 0, flag = 0, pos_dir = procura_fat();
	//Percorre todo o diretório em busca de um arquivo com o nome file_name
	for(i = 0; i < 128; i++){
		if(!strcmp(dir[i].name, file_name)){
		    flag = 1;
        pos_dir = i;							// sinaliza arquivo encontrado
		    break;
		}
	}

	if(mode == FS_R){
    printf("Sera se entra no read\n");
		//Se entrar aqui, significa que o arquivo não existe e um erro deve ser gerado
		if(!flag){
			printf("Arquivo nao existe\n");
			return -1;
        }
        // salva no vetor de identificadores arquivos abertos
        for(i = 0; i < 128; i++){
          //FIZEMOS UMA ALTERCAO AQUI
            if(!id_arq[i].first_block){
                id_arq[i].first_block = dir[pos_dir].first_block;
                id_arq[i].f_mode = FS_R;
                id_arq[i].cont_leitura = 0;
                printf("I retornado: %d\n", i);
                return i;
            }
        }
    } else if (mode == FS_W){
      printf("Sera se entra no uaiti\n");
        // cria arquivo caso não exista
        if(!flag) {
            printf("arquivo não existe\n");
            fs_create(file_name);
        } else {
            printf("arquivo existe\n");
            fs_remove(file_name);
            fs_create(file_name);
        }

        for(i = 0; i < 128; i++){
            if(id_arq[i].first_block == DIR_LIVRE){
              printf("Alteramos aqui hehe\n");
                id_arq[i].first_block = dir[pos_dir].first_block;
                id_arq[i].f_mode = FS_W;
                id_arq[i].cont_leitura = 0;
                printf("que\n");
                return i;
            }
        }
    }
    return -1;
}

/*
 * Procura o arquivo no vetor de identificadores e verifica qual o
 * modo (leitura ou escrita).
 * Retorna erro caso não seja nenhum dos dois
 * Caso contrário, tira o arquivo do vetor.
 */
int fs_close(int file){
/* TODO: PERGUNTAR:
      - precisa reescrever fat e diretório quando fechar o arquivo?
*/
    if(file < 0 || file > 128){
        printf("Não existe o arquivo\n");
        return -1;
    }

    if(id_arq[file].f_mode != 0 && id_arq[file].f_mode != 1){
        printf("Arquivo não está aberto para leitura ou escrita\n");
        return -1;
    }

    id_arq[file].f_mode = -1;
    id_arq[file].first_block = DIR_LIVRE;
    id_arq[file].cont_leitura = 0;
    return 0;
}

/*
 *  Caso o buffer tiver cheio, ir na fat e procurar o próximo bloco livre.
 *  Aí é só recarregar o buffer e continuar a escrita.
 *  A posição de escrita é sempre na última posição porque não tem seek.
 */
int fs_write(char *buffer, int size, int file) {
    //printf("Função não implementada: fs_write\n");
    char buffer_interno[CLUSTERSIZE];
    int cont_interno = 0, cont = 0, total = 0, pos_dir, prox_bloco;
    int qtd, qtd_aux = 0; //i, j;
    printf("Entrou no write\n");
    memset(buffer_interno, '\0', CLUSTERSIZE);
    /* VERIFICAÇÃO DE MODO */
    if(id_arq[file].f_mode != FS_W){
        printf("Erro ao abrir o arquivo para escrita\n");
        return -1;
    }

    pos_dir = id_arq[file].first_block;

    /* calcula o número de blocos que vão ser necessários pra escrita */
    if(size <= CLUSTERSIZE)
        qtd = 1;
    else if(size%CLUSTERSIZE != 0)
        qtd = 1 + (size / CLUSTERSIZE);
    else
        qtd = size / CLUSTERSIZE;

    //printf("qtd: %d\n", qtd);
    //printf("size: %d  qtd_aux: %d  size do file: %d\n", size, qtd_aux, dir[file].size);
    while(qtd_aux < qtd || size >= 0){
        while(cont_interno < size && cont_interno < CLUSTERSIZE && size > 0){
            buffer_interno[cont_interno] = buffer[cont];
            cont_interno++;
            cont++;
            total++;
        }

        if(!bl_write(INICIO_DIR + pos_dir*8, &buffer_interno[qtd_aux*SECTORSIZE]))
            return -1;
        qtd_aux++;
        cont_interno = 0;
        // procura a prox posição livre na fat
        prox_bloco = procura_fat();
        if(prox_bloco == -1){
            printf("Disco cheio!\n");
            return -1;
        }
        fat[dir[pos_dir].first_block] = prox_bloco;
        fat[prox_bloco] = ULTIMO;
        size -= CLUSTERSIZE;
    }
    dir[pos_dir].size = total;
    printf("Total de bytes escritos: %d\n", total);
    return total;
}

int procura_fat(){
    int pos_fat = -1;
    for(int i = 33; i < TAM_FAT; i++){
        if(fat[i] == LIVRE){
            pos_fat = i;
            break;
        }
    }
    return pos_fat;
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
    char buffer_interno[4096];
    int pos_dir, cont_interno = 0, cont = 0, aux, qtd, qtd_aux = 0, prox, total = 0;
    printf("Entrou no read\n");

    /* VERIFICAÇÃO DE MODO DO ARQUIVO */
    if(id_arq[file].f_mode != FS_R){
        printf("Erro ao abrir o arquivo para leitura\n");
        return -1;
    }

    /* TODO: verificar se o pos_dir tem que ser o first_block*8 ou não*/
    pos_dir = id_arq[file].first_block;
    total += cont_interno;

    /* calcula o número de blocos que vão ser carregados pra memória */
    if(size <= CLUSTERSIZE)
        qtd = 1;
    else if(size % CLUSTERSIZE != 0)
        qtd = 1 + (size / CLUSTERSIZE);
    else
        qtd = size / CLUSTERSIZE;

    printf("Antes do bl_read\n");
    printf("INICIO_DIR: %d\n", INICIO_DIR);
    printf("pos_dir: %d\n", pos_dir);
    printf("FILE: %d\n", file);

    /* carrega o primeiro bloco pra memória */
    if(!bl_read(INICIO_DIR + pos_dir*8, &buffer_interno[qtd_aux*SECTORSIZE]))
        return -1;

    while(qtd_aux < qtd || size >= 0){
        while(cont_interno < size && cont_interno < CLUSTERSIZE && size > 0){
           buffer[cont] = buffer_interno[cont_interno];
           cont++;
           cont_interno++;
           total++;
        }
            dir[pos_dir].size = total;printf("buffer_interno fs_read: %s\n", buffer_interno);
        printf("buffer fs_read: %s\n", buffer);
        if(cont_interno >= CLUSTERSIZE){
        	aux = dir[pos_dir].first_block;
          id_arq[pos_dir].first_block = fat[aux];
      		prox = fat[aux];
        }

    /* TODO: deveríamos DE NOVO fazer um aux*8 ou prox*8? */
		if(!bl_read(INICIO_DIR + prox*8, &buffer[prox*SECTORSIZE]))
		    return -1;

		qtd_aux++;
		cont_interno = 0;
    size -= CLUSTERSIZE;
    dir[pos_dir].size = total;
    }
    printf("Total de bytes lidos: %d\n", total);
    return total;
}
