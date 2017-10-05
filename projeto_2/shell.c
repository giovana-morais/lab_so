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
		token =  strtok(comando, " ");
		i = 0, flag_bg = 0, flag_in = 0, flag_out = 0;

		while(token != NULL){
            if(!strcmp(token, "<")){
                // redireção de entrada
                token = strtok(NULL, " "); 
                nome_arq_in = token;
                flag_in = 1;
            } else if(!strcmp(token, ">")) {
                // redireção de saída
                token = strtok(NULL, " "); 
                nome_arq_out = token;
                flag_out = 1;
            } else {
                args[i] = token;
                i++;
            }
            token =  strtok(NULL, " ");
		}
		args[i] = '\0';

		if (!strcmp(comando, "exit")) {
			exit(EXIT_SUCCESS);
		}

        if(!strcmp(args[--i], "&")){
            args[i] = '\0';
            flag_bg = 1;
        }

		pid = fork();
		if(flag_bg && pid){
            // execução em background
            waitpid(pid, &retorno, WNOHANG);
        } else if (pid) {
			waitpid(pid, NULL, 0); 
		} else {
            // redirecionamento de entrada apenas no processo filho
            if(flag_in)
                arq_in = freopen(nome_arq_in, "r", stdin); 
            
            if(flag_out)
                arq_out = freopen(nome_arq_out, "w", stdout);

			if(execvp(comando, args) < 0){
				printf("Erro ao executar comando!\n");
				exit(EXIT_FAILURE);
            }
		}
	}
}
