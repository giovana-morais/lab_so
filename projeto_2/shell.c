#include <stdio.h>
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

int main() {
  char comando[MAX], *token, **args;
  int pid, i = 0;

  args = malloc(MAX * sizeof(char *));
  while (1) {
    printf("> ");
    fgets(comando, MAX, stdin);
    strtok(comando, "\n");
    token =  strtok(comando, " ");
    while(token != NULL){
        printf("token: %s\n", token);
        args[i] = token;
        printf("args[i]: %s\n", args[i]);
        token =  strtok(NULL, " ");
        i++;
    }

    
    if (!strcmp(comando, "exit")) {
      exit(EXIT_SUCCESS);
    }

    pid = fork();
    if (pid) {
      waitpid(pid, NULL, 0); 
    } else {
      execlp(comando, comando, NULL);
      printf("Erro ao executar comando!\n");
      exit(EXIT_FAILURE);
    }
  }
}
