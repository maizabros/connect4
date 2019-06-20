#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include "libnet.h"

#define BUFF_SIZE 4096
#define FILE_ROW 64
#define OPTION_LENGTH 128
#define NAME_LENGTH 32

FILE * exchange;
void juego();
char adversario[NAME_LENGTH];
char jugador[NAME_LENGTH];
int r, column;
int sock;
/* **********************  FUNCIONES DEL BOT  ************************ */
int encuentraTres(char board[r][column]);
int encuentraDos(char board[r][column]);
void colocarFicha(char board[r][column], int columna, char ficha);
int buscarEspacio(char board[r][column]);
/* ******************************************************************* */
void inicialise_board();
int good_move(int row, int col, char board[r][column]);
int make_move(int jug, int col, char board[r][column]);
int number_gen();
void show_board(char board[r][column]);
int check_end(char board[r][column]);

int main(int argc, char * argv[]) {

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (void * ) & server, sizeof(server)) != 0) {
        printf("No se pudo conectar");
        return EXIT_FAILURE;
    }

    //Mensaje de bienvenida
    printf("╔════════════╗\n");
    printf("║ Bienvenido ║\n");
    printf("╚════════════╝\n\n");
    printf("Conectado a servidor con IP %s y puerto %s\n\n", argv[1], argv[2]);

    FILE * file = fopen("usuario.txt", "r");
    char * fileId = malloc(sizeof(idStruct));
    char buffer[BUFF_SIZE];
    char ** fragBuffer;
    char msg[BUFF_SIZE];
    int res;
    int terminado = -1;

    while (terminado == -1) {
        read(sock, buffer, BUFF_SIZE);
        buffTreat(buffer);
        if (strstr(buffer, "WELCOME") != NULL) {
            terminado = 0;
        }
        if (strstr(buffer, "FULL") != NULL) {
            printf("Servidor lleno\nSaliendo\n\n");
            return EXIT_FAILURE;
        }
    }

    if (file != NULL) {
        fgets(fileId, FILE_ROW, file);
        fgets(jugador, NAME_LENGTH, file);
        fclose(file);
        buffTreat(fileId);
        buffTreat(jugador);
        printf("Autentificacion con ID y respuesta %s\n", fileId);
        sprintf(msg, "LOGIN %s\n", fileId);
        write(sock, msg, sizeof("LOGIN\n") + strlen(fileId));
        memset(msg, '\0', BUFF_SIZE);
    } else {
        printf("El usuario no esta registrado en ningun servidor\n");
        printf("Enviando solicitud de registro\n");
        write(sock, "REGISTRAR\n", sizeof("REGISTRAR\n"));
    }

    terminado = -1;
    while (terminado == -1) {
        memset(buffer, '\0', BUFF_SIZE);
        read(sock, buffer, BUFF_SIZE);
        buffTreat(buffer);

        if (strstr(buffer, "LOGIN ERROR") != NULL) {
            memset(buffer, '\0', BUFF_SIZE);
            printf("Ha habido un error en la autentificacion\n");
            return EXIT_FAILURE;
        }
        if (strstr(buffer, "LOGIN OK") != NULL) {
            fragBuffer = fragBufferExtract(fileId);
            printf("Cliente conectado correctamente\n");
            printf("Id cliente: %s\n", fragBuffer[0]);
            memset(buffer, '\0', BUFF_SIZE);
            delArg(fragBuffer);
            terminado = 0;
        }
        if (strstr(buffer, "RESUELVE") != NULL) {
            fragBuffer = fragBufferExtract(buffer);
            res = atoi(fragBuffer[1]) + atoi(fragBuffer[2]);
            sprintf(msg, "RESPUESTA %d\n", res);
            printf("RESPUESTA %d\n", res);
            write(sock, msg, sizeof("RESPUESTA %d\n"));
            memset(buffer, '\0', BUFF_SIZE);
            memset(msg, '\0', BUFF_SIZE);
            delArg(fragBuffer);
        }
        if (strstr(buffer, "REGISTRADO OK") != NULL) {
            fragBuffer = fragBufferExtract(buffer);
            printf("Cliente registrado correctamente\n");
            printf("Id cliente: %s\n", fragBuffer[2]);
            strcpy(jugador, argv[3]);
            file = fopen("usuario.txt", "w+");
            fprintf(file, "%s %d\n", fragBuffer[2], res);
            fprintf(file, "%s\n", argv[3]);
            fclose(file);
            memset(buffer, '\0', BUFF_SIZE);
            delArg(fragBuffer);
            sprintf(msg, "SETNAME %s\n", argv[3]);
            write(sock, msg, sizeof("SETNAME \n") + strlen(argv[3]));
        }
        if (strstr(buffer, "REGISTRADO ERROR") != NULL) {
            memset(buffer, '\0', BUFF_SIZE);
            printf("Ha habido un error en el registro\n");
            return (1);
        }

        if (strstr(buffer, "SETNAME OK") != NULL) {
            memset(buffer, '\0', BUFF_SIZE);
            printf("Nombre cambiado correctamente\n");
            terminado = 0;
        }
        if (strstr(buffer, "SETNAME ERROR") != NULL) {
            memset(buffer, '\0', BUFF_SIZE);
            printf("Ha habido un error en el SETNAME\n");
        }
    }
    juego();
    printf("Servidor desconectado\nSaliendo...\n");
}

/// FUNCIONES DEL JUEGO

void juego() {

    int fin_partida = -1;
    int g_move = -1;
    int player;
    char msg[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char ** fragBuffer;
    int colContr;
    int columSel;
    int first;
    int terminado = -1;
    int serBot;

    printf("BIENVENIDO AL JUEGO DE CONECTA 4\n\n");

    while (terminado == -1) {
        memset(buffer, '\0', BUFF_SIZE);
        read(sock, buffer, BUFF_SIZE);
        buffTreat(buffer);

        if (strstr(buffer, "START ") != NULL) {
            buffTreat(buffer);
            fragBuffer = fragBufferExtract(buffer);
            sprintf(adversario, "%s", fragBuffer[1]);
            r = atoi(fragBuffer[2]);
            column = atoi(fragBuffer[3]);
            delArg(fragBuffer);
            terminado = 0;
        }
    }
    char board[r][column];
    inicialise_board(board);
    show_board(board);

    while (fin_partida == -1) {
        read(sock, buffer, BUFF_SIZE);
        buffTreat(buffer);

        if (strstr(buffer, "URTURN") != NULL && first != 1) {
            player = 0;
            first = 0;
            if (strlen(buffer) > strlen("URTURN")) {
                player = 1;
                first = 1;
                printf("Desea utilizar un bot o quiere jugar?\nJugar: 0\nBot: 1\nOpcion escogida: ");
                scanf("%d", & serBot);
            }
            if (first != 1)
                do {
                    show_board(board);
                    printf("Adversario coloca ficha en columna %d\n", colContr);
                    printf("Introduce la columna en la que quieres colocar tu pieza: ");
                    scanf("%d", & columSel);
                    columSel--;
                    sprintf(msg, "COLUMN %d\n", columSel);
                    write(sock, msg, sizeof("COLUMN %d\n"));
                    read(sock, buffer, BUFF_SIZE);
                    if (strstr(buffer, "COLUMN OK") != NULL) {
                        g_move = 0;
                        make_move(player, columSel, board);
                        show_board(board);
                    } else if (strstr(buffer, "COLUMN ERROR") != NULL) {
                        g_move = -1;
                        printf("Jugada incorrecta.\n\n");
                    }
                } while (g_move == -1);
        }

        if (strstr(buffer, "URTURN") != NULL && first == 1) {
            fragBuffer = fragBufferExtract(buffer);
            buffTreat(fragBuffer[1]);
            colContr = atoi(fragBuffer[1]);
            if (player == 0)
                make_move(1, colContr, board);
            else if (player == 1)
                make_move(0, colContr, board);

            do {
                show_board(board);
                // AQUÍ EMPIEZA EL BOT
                if (player == 1 && serBot == 1) {

                    int hayTres = encuentraTres(board);
                    int hayDos = encuentraDos(board);

                    if (hayTres != -1) {
                        columSel = hayTres;
                    } else if (hayDos != -1) {
                        columSel = hayDos;
                    } else {
                        columSel = buscarEspacio(board);
                    }
                    // HASTA AQUÍ EL BOT
                } else {
                    printf("Introduce la columna en la que quieres colocar tu pieza: ");
                    scanf("%d", & columSel);
                    columSel--;
                }
                sprintf(msg, "COLUMN %d\n", columSel);
                write(sock, msg, sizeof("COLUMN %d\n"));
                read(sock, buffer, BUFF_SIZE);
                if (strstr(buffer, "COLUMN OK") != NULL) {
                    g_move = 0;
                    make_move(player, columSel, board);
                    show_board(board);
                } else if (strstr(buffer, "COLUMN ERROR") != NULL) {
                    g_move = -1;
                    printf("Jugada incorrecta.\n\n");
                }
            } while (g_move == -1);
        }

        if (strstr(buffer, "VICTORY") != NULL) {
            fin_partida = 0;
        }
        if (strstr(buffer, "DEFEAT") != NULL) {
            fin_partida = 1;
        }
        if (strstr(buffer, "TIE") != NULL) {
            fin_partida = 2;
        }
        first = 1;
    }

    show_board(board);
    if (fin_partida == 1)
        printf("Enhorabuena!! Has ganado!!\n\n");
    else if (fin_partida == 0)
        printf("%s ha ganado la partida!!\n\n", adversario);
    else if (fin_partida == 2)
        printf("Habeis quedado empatados\n\n");
    getchar();
}

void inicialise_board(char board[r][column]) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < column; j++)
            board[i][j] = ' ';
}

int make_move(int jug, int col, char board[r][column]) {
    int row; //Para guardar la row y columna en la que se introduce la piece.
    int i; //Para el contador.
    char piece; //Para guardar la piece del player actual. 0- Para humano, X- Para la máquina.
    int move_result;
    move_result = 1;

    if (jug == 0) {
        piece = '0';
        if (move_result == 0) printf("Jugada incorrecta.\n");
        i = r - 1;
        if (board[r - 1][col] != ' ') {
            while (i < r && (board[i][col] == 'X' || board[i][col] == '0'))
                i--;
        }
        row = i;
        move_result = good_move(row, col, board);
        if (move_result == 1) board[row][col] = piece;
    } else {
        piece = 'X';
        if (move_result == 0) printf("Jugada incorrecta.\n");
        i = r - 1;
        if (board[r - 1][col] != ' ') {
            while (i < r && (board[i][col] == 'X' || board[i][col] == '0'))
                i--;
        }
        row = i;
        move_result = good_move(row, col, board);

        if (move_result == 1) board[row][col] = piece;
    }
    return move_result;
}

int good_move(int row, int col, char board[][column]) {
    int g_move; //1-Jugada correcta. 0-Jugada errónea
    g_move = 1;
    if (row >= r || row < 0 || col >= column || col < 0) g_move = 0;
    if (g_move == 1) {
        if (board[row][col] == '0' || board[row][col] == 'X') g_move = 0;
    }
    if (g_move == 1) {
        if (row != r - 1 && board[row + 1][col] == ' ') g_move = 0;
    }
    return g_move;
}

void show_board(char board[][column]) {

    system("clear");
    char tit[16000];
    FILE * titulo = fopen("titulo.txt", "r");
    while (fgets(tit, 160000, titulo) != NULL) {
        printf("%s", tit);
    }
    fclose(titulo);
    printf("\n");
    for (int j = 0; j < column * 5 / 2; j++)
			printf(" ");
    printf("\n   -");
    for (int j = 0; j < column + 1; j++)
			printf("----");
    printf("\n");
    for (int i = 0; i < r; i++) {
        printf("   %d |", r - i);
        for (int j = 0; j < column; j++) {
            printf(" %c |", board[i][j]);
        }
        printf(" \n   -");
        for (int j = 0; j < column + 1; j++)
					printf("----");
        printf("\n");
    }
    printf("       ");
    for (int j = 0; j < column; j++) printf("%d   ", j + 1);
    printf("  Adversario: %s\n\n", adversario);
}

int encuentraTres(char board[r][column]) {
    int i, j;
    int posicion = -1;

    // Horizontal
    for (i = 0; i < r; i++) {
        for (j = 0; j < column - 2; j++) {
            if (board[i][j] == '0' && board[i][j + 1] == '0' && board[i][j + 2] == '0' && j > 0) {
                if (board[i][j - 1] != '0' && board[i][j - 1] != 'X') {
                    posicion = j - 1;
                }
            } else if (board[i][j] == '0' && board[i][j + 1] == '0' && board[i][j + 2] == '0') {
                if (board[i][j + 3] != '0' && board[i][j + 3] != 'X') {
                    posicion = j + 3;
                }
            }
        }
    }
    if (posicion == -1) {
        // Vertical
        for (i = 0; i < r - 2; i++) {
            for (j = 0; j < column; j++) {
                if (board[i][j] == '0' && board[i + 1][j] == '0' && board[i + 2][j] == '0' && i > 0) {
                    if (board[i - 1][j] != '0' && board[i - 1][j] != 'X') {
                        posicion = j;
                    }
                }
            }
        }
    }

    if (posicion == -1) {
        // Diagonal *
        //            *
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 3 < r && j + 3 < column) {
                    if (board[i][j] == '0' && board[i + 1][j + 1] == '0' && board[i + 2][j + 2] == '0') {
                        if (board[i + 3][j + 3] != '0' && board[i + 3][j + 3] != 'X') {
                            posicion = j + 3;
                        }
                    }
                }
            }
        }
    }

    if (posicion == -1) {
        // Diagonal   *
        //          *
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 3 < r && j - 3 >= 0) {
                    if (board[i][j] == '0' && board[i + 1][j - 1] == '0' && board[i + 2][j - 2] == '0') {
                        if (board[i + 3][j - 3] != '0' && board[i + 3][j - 3] != 'X') {
                            posicion = j - 3;
                        }
                    }
                }
            }
        }
    }
    return posicion;
}

int encuentraDos(char board[r][column]) {

    int posicion = -1;
    int i, j;

    // Horizontal
    for (i = 0; i < r; i++) {
        for (j = 0; j < column - 1; j++) {
            if (board[i][j] == '0' && board[i][j + 1] == '0' && j > 0) {
                if (board[i][j - 1] != '0' && board[i][j - 1] != 'X') {
                    posicion = j - 1;
                }
            } else if (board[i][j] == '0' && board[i][j + 1] == '0') {
                if (board[i][j + 2] != '0' && board[i][j + 2] != 'X') {
                    posicion = j + 2;
                }
            }
        }
    }
    if (posicion == -1) {
        // Vertical
        for (i = 0; i < r - 1; i++) {
            for (j = 0; j < column; j++) {
                if (board[i][j] == '0' && board[i + 1][j] == '0' && i > 0) {
                    if (board[i - 1][j] != '0' && board[i - 1][j] != 'X') {
                        posicion = j;
                    }
                }
            }
        }
    }

    if (posicion == -1) {
        // Diagonal *
        //            *
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 2 < r && j + 2 < column) {
                    if (board[i][j] == '0' && board[i + 1][j + 1] == '0') {
                        if (board[i + 2][j + 2] != '0' && board[i + 2][j + 2] != 'X') {
                            posicion = j + 2;
                        }
                    }
                }
            }
        }
    }

    if (posicion == -1) {
        // Diagonal   *
        //          *
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 2 < r && j - 2 >= 0) {
                    if (board[i][j] == '0' && board[i + 1][j - 1] == '0') {
                        if (board[i + 2][j - 2] != '0' && board[i + 2][j - 2] != 'X') {
                            posicion = j - 2;
                        }
                    }
                }
            }
        }
    }
    return posicion;
}

void colocarFicha(char board[r][column], int columna, char ficha) {

    int i;
    for (i = r - 1; i >= 0; i--) {
        if (board[i][columna] == '-') {
            break;
        }
    }

    if (i >= 0) {
        board[i][columna] = ficha;
    }
}

int buscarEspacio(char board[r][column]) {

    int i, j;
    for (i = r; i > 0; i--) {
        for (j = 0; j < column; j++) {
            if (board[i][j] == ' ') {
                return j;
            }
        }
    }
}
