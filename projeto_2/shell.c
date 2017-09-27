#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX 256

/* 
 * 1. descobrir como receber argumentos (fç de string)
 *  1.1 - pegar a linha inteira 
 *  1.2 - dividir a linha em argumentos (strtok)
 *  1.3 - interpretar cada um deles (execvp)
 * 2. comandos em segundo plano
 * 3. entrada e saída padrão 
 */

/* DUVIDAS
	1. o que fazer no caso do cd? (cd não roda)
	2. comandos que rodam em segundo plano podem receber argumentos?
	2.1 aparentemente, os comandos já estão sendo rodados em segundo plano. o firefox, por exemplo, funciona. a calculadora não funciona.
	3. devemos permitir uma sequência de comandos? ex: firefox & ls -al 
*/
int main() {
	char comando[MAX], *token, **args;
	int pid, i;

	args = malloc(MAX * sizeof(char *));
	while (1) {
		printf("> ");
		__fpurge(stdin);
		fgets(comando, MAX, stdin);
		strtok(comando, "\n");
		token =  strtok(comando, " ");
		i = 0;
		while(token != NULL){
			args[i] = token;
			token =  strtok(NULL, " ");
			i++;
		}
		args[i++] = '\0';

		for(i = 0; args[i] != NULL; i++)
			printf("arg[i] %s\n", args[i]);


		if (!strcmp(comando, "exit")) {
			exit(EXIT_SUCCESS);
		}

		pid = fork();
		if (pid) {
			waitpid(pid, NULL, 0); 
		} else {
			//execlp(comando, comando, NULL);
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
