#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX 256

int main() {
	char comando[MAX], *token, **args, *nome_arq_in, *nome_arq_out;
	int pid, i, retorno = 0, flag_bg, flag_in, flag_out;
    FILE *arq_in, *arq_out;

	args = malloc(MAX * sizeof(char *));

	while (1) {
		printf("> ");
		__fpurge(stdin);

		fgets(comando, MAX, stdin);
    comando[strlen(comando) - 1] = '\0'; // ignora o \n
		token =  strtok(comando, " ");//obtem palavra até o espaço
		i = 0, flag_bg = 0, flag_in = 0, flag_out = 0;

		while(token != NULL){
            if(!strcmp(token, "<")){
                // redireção de entrada
                token = strtok(NULL, " ");//obtem o filename do arquivo
                nome_arq_in = token;//salva o filename em um ponteiro de string
                flag_in = 1;//flag utilizada para identificar se há redirecionamento de entrada
            } else if(!strcmp(token, ">")) {
                // redireção de saída
                token = strtok(NULL, " ");//obtem o filename do arquivo
                nome_arq_out = token;//salva o filename em um ponteiro de string
                flag_out = 1;//flag utilizada para identificar se há redirecionamento de saida
            } else {
                args[i] = token;//armazena a palavra no vetor de argumentos
                i++;//incrementa o indice do vetor de argumentos
            }
            token =  strtok(NULL, " ");//obtem palavra até o espaço
		}
		args[i] = '\0';//"fecha" o vetor de argumentos

		if (!strcmp(comando, "exit")) {//sai da shell se for um comando de saida
			exit(EXIT_SUCCESS);
		}

        if(!strcmp(args[--i], "&")){//verificação de execução em background
            args[i] = '\0';//retira o & do vetor de argumentos
            flag_bg = 1;//flag de identificação de execução em background
        }

		pid = fork();//cria um novo processo
		if(flag_bg && pid){//em caso de execução em background
      // execução em background
      waitpid(pid, &retorno, WNOHANG);//comando de execução de processo sem precisar o proximo esperar ele terminar
    } else if (pid) {
			waitpid(pid, NULL, 0);
		} else {
    // redirecionamento de entrada apenas no processo filho
	    if(flag_in)//identifica redirecionamento de entrada
	        arq_in = freopen(nome_arq_in, "r", stdin);

	    if(flag_out)//identifica redirecionamento de saida
	        arq_out = freopen(nome_arq_out, "w", stdout);

			if(execvp(comando, args) < 0){//execução de comando com argumentos
				printf("Erro ao executar comando!\n");
				exit(EXIT_FAILURE);
	    }
		}
	}
}
