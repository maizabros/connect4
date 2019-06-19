/****************************************************/
/* Libreria de funciones para Redes de Computadores */
/****************************************************/
/* Last update: 05/04/2019                          */
/* Version: 0.3.7                                   */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define LENGTH_LETTER 26
#define LENGTH_NUMBER 10
#define LENGTH_BUFFER 40

typedef struct idStruct {
    char clientId[13];
    char clientName[18];
	 char clientResult[3];
    struct idStruct *next;
    FILE *file; 
} idStruct;

typedef struct list {
    struct idStruct *first;
    struct idStruct *last;
} list;


char *bufferExtract(char *buffer);
char **fragBufferExtract(const char *buffer);
char **fragBufferExtract2(const char *buffer, int *argNumber);
idStruct *changeName (list *header, char *id, char *name);
void delArg(char **arg);
char *buffTreat (char *buffer);
char *random_id(size_t length);


char **fragBufferExtract(const char *buffer) {

	char **arg, *copy, *token;
	int words=0, i;
	int stringLength = strlen(buffer);
	copy = (char*) calloc(stringLength+1, sizeof(char));
	strcpy(copy, buffer);
	words = 1;
	token = strtok((char*)buffer," ");
	while(token!=NULL) {
		if (strlen(token) > 0)
			words++;
		token = strtok(NULL," ");
	}
	arg = (char**) malloc(words*sizeof(char*));
	i = 0;
	token = strtok(copy," ");
	while(token != NULL) {
		if (strlen(token) > 0) {
			arg[i] = (char*)calloc(strlen(token)+1, sizeof(char));
			strcpy(arg[i],token);
			i++;
		}
		token = strtok(NULL," ");
	}
	arg[i] = NULL;
	buffTreat(arg[i-1]);
	free(copy);
	return(arg);
}


char **FragBufferExtract2(const char *buffer, int *argNumber) {

	char **arg, *copy, *token;
	int words = 0, i;
	int stringLength = strlen(buffer);
	copy = (char*) calloc(stringLength + 1, sizeof(char));
	strcpy(copy, buffer);
	words = 1;
	token = strtok((char*)buffer," ");
	while(token != NULL)
	{
		 if (strlen(token) > 0)
			words++;
		token = strtok(NULL," ");
	}
	arg = (char**) malloc(words*sizeof(char*));
	i = 0;
	token = strtok(copy," ");
	while(token != NULL)
	{
		if (strlen(token) > 0)
		{
			arg[i] = (char*)calloc(strlen(token)+1, sizeof(char));
			strcpy(arg[i],token);
			i++;
		}
		token = strtok(NULL," ");
	}
	arg[i] = NULL;
	free(copy);
	*argNumber = words;
	return(arg);
}


char *bufferExtract(char *buffer) {

	char surplus[2000];
	char *array = (char*)malloc(sizeof(char)*2000);
	char *aux = (char*)malloc(sizeof(char));
	int i;
	i = 0;
	*aux = buffer[i];
	memset(surplus,'\0',2000);
	while ((*aux != ' ') && (i < strlen(buffer))) {
		i++;
		*aux = buffer[i];
	}
	while ((*aux == ' ') && (i < strlen(buffer))) {
		i++;
		*aux = buffer[i];
	}
	int j = 0;
	for(i; i < strlen(buffer); i++) {
		surplus[j] = buffer[i];
		j++;
	}
	free(aux);
	array = surplus;
	return array;
}


void insertList (list *header, char *id, char *result, char* defName) {

	idStruct *aux = (idStruct*)malloc(sizeof(idStruct));
	sprintf(aux -> clientId,"%s", id);
	sprintf(aux -> clientResult, "%s", result);
	sprintf(aux -> clientName,"%s",defName);
	aux -> next = NULL;
	if (header -> first == NULL) {
		header -> first = aux;
		header-> last = aux;
	} else {
		header -> last -> next = aux;
		header -> last = aux;
	}
	return;
}


idStruct *findId (list *header, char *id, char *result) {

	idStruct *aux;
	if (header == 0) {
		return NULL;
	} else {
		aux = header -> first;
		while ((aux -> next != NULL) && (strstr(id,  aux -> clientId) == NULL)) {
			 aux = aux -> next;
		}
		if (strcmp(id, aux -> clientId) == 0) {
			 if (strstr(result, aux -> clientResult) != NULL) {
					return aux;
			}
		}
		return NULL;
	}
}


idStruct *changeName (list *header, char *id, char *name) {

	idStruct *aux;
	if (header == 0) {
		printf("ERRRROR\n");
		return NULL;
	} else {
		aux = header -> first;
		while ((aux -> next!= NULL) && (strstr(id,  aux -> clientId) == NULL)) {
			aux = aux -> next;
		}
		if (strcmp(id, aux -> clientId) == 0) {
			memset(aux ->  clientName, '\0', 17);
			sprintf(aux -> clientName,"%s", name);
			return aux;
		} else {
			return NULL;
		}
	}
}


void showList (list *header) {

	idStruct *aux;
	if (header == NULL){
		 perror("Empty list\n");
	} else {
		aux = header -> first;
		while (aux -> next != NULL) {
			  printf("%s ", aux -> clientId);
			  aux = aux -> next;
		}
		printf("%s", header -> last -> clientId);
		printf("\n");
	}
	return;
}


void delArg(char **arg) {

	int i=0;
	while((arg)[i] != NULL) {
		free((arg)[i]);
		i++;
	}
	free(arg);
}


char *buffTreat(char *buffer) {

	for(int i = 0; i < strlen(buffer)+1; i++) {
		if(buffer[i] == '\n'){
			buffer[i] = '\0';
		}
	}
}


char *random_id(size_t length) {
	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";        
	char *randomId = NULL;
	if (length) {
		randomId = (char*)malloc(sizeof(char) * (length +1));
		if (randomId) {            
			for (int n = 0;n < length;n++) {            
				int key = rand() % (int)(sizeof(charset) -1);
				randomId[n] = charset[key];
			}
			randomId[length] = '\0';
		}
	}
	return randomId;
}
