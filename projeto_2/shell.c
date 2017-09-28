#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX 256

/* 
 * 1. descobrir como receber argumentos YESSSSSSSSSSSSSS
 *  1.1 - pegar a linha inteira 
 *  1.2 - dividir a linha em argumentos (strtok)
 *  1.3 - interpretar cada um deles (execvp)
 * 2. comandos em segundo plano YESSSSSSSSSSSSSS
 * 3. entrada e saída padrão 
 */

int main() {
	char comando[MAX], *token, **args, *nome_arq_in, *nome_arq_out;
	int pid, i, retorno = 0, flag = 0, flag_arq_in = 0, flag_arq_out;
    FILE *arq_in, *arq_out;

	args = malloc(MAX * sizeof(char *));
	while (1) {
		printf("> ");
		__fpurge(stdin);
		fgets(comando, MAX, stdin);
		strtok(comando, "\n");
		token =  strtok(comando, " ");
		i = 0;
        flag = 0;
		while(token != NULL){
            if(token == "<")
                nome_arq_in = token;
            else if(token == ">")
                nome_arq_out = token;
            else 
                args[i] = token;
            token =  strtok(NULL, " ");
            i++;
            
		}
		args[i] = '\0';

		if (!strcmp(comando, "exit")) {
			exit(EXIT_SUCCESS);
		}

        if(!strcmp(args[--i], "&")){
            args[i] = '\0';
            flag = 1;
        }

		pid = fork();
		if(flag && pid){
            // execução em background
            waitpid(pid, &retorno, WNOHANG);
        } else if (pid) {
			waitpid(pid, NULL, 0); 
		} else {
			if(execvp(comando, args) < 0) {
				for(i = 0; args[i] != NULL; i++){
					free(args[i]);
				}
				free(args);
				printf("Erro ao executar comando!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
