#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "libnet.h"

#define BUFF_SIZE 4096
#define FILE_ROW 64
#define OPTION_LENGTH 16
#define NAME_LENGTH 32

char * random_id(size_t length);
FILE * fichero_cliente[2];
int cliente[2];
char jugador0[NAME_LENGTH];
char jugador1[NAME_LENGTH];
int sock;
int r, column;
struct num_struct {
    int num;
};

int i;

/* **********************  FUNCIONES DEL BOT  ************************ */
int encuentraTres(char board[r][column]);
int encuentraDos(char board[r][column]);
void colocarFicha(char board[r][column], int columna, char ficha);
int buscarEspacio(char board[r][column]);
/* ******************************************************************* */

void * aceptarConexion(void * args);
void * rechazarConexion(void * args);

void juego();
void inicialise_board();
int good_move(int row, int col, char board[r][column]);
int make_move(int jug, int col, char board[r][column]);
int number_gen();
void show_board(char board[][column]);
int check_end(char board[r][column]);

int main(int argc, char * argv[]) {
    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(atoi(argv[1]));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int activado = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    if (bind(sock, (void *) &servidor, sizeof(servidor)) != 0) {
        perror("Falló el bind");
        return EXIT_FAILURE;
    }

    FILE * file = fopen("usuarios_servidor.txt", "r");
    r = atoi(argv[2]);
    column = atoi(argv[3]);
    if (r < 6 || column < 7) {
        printf("Dimensiones minimas 6 filas y 7 columnas\n");
        return EXIT_FAILURE;
    }

    if (file == NULL) {
        printf("No hay fichero de Id del servidor\nEsperando registro de cliente\n");
    } else {
        printf("Existe fichero de Id\nEsperando instruccion\n\n");
        fclose(file);
    }

    while (1) {

        printf("Escuchando conexiones en el puerto\n");
        listen(sock, 1000);
        struct sockaddr_in direccionCliente;
        unsigned int tam_direc;
        i = 0;
        cliente[0] = accept(sock, (void * ) & direccionCliente, & tam_direc);

        pthread_t acceptThread_1, refuseThread;
        pthread_create( & acceptThread_1, NULL, aceptarConexion, & i);
        pthread_join(acceptThread_1, NULL);
        i++;
        pthread_create( & refuseThread, NULL, rechazarConexion, & sock);

        juego();

        pthread_cancel(refuseThread);

    }

    printf("Cerrando...\nQue pase un buen día.");
    return EXIT_SUCCESS;
}

void * aceptarConexion(void * args) {

    struct num_struct * num_j = args;
    int j = num_j-> num;
    FILE * file = fopen("usuarios_servidor.txt", "r");
    char * fileId = malloc(sizeof(idStruct));
    char buffer[BUFF_SIZE];
    char ** fragBuffer;
    char ** auxFragBuffer;
    struct idStruct * idUser = (idStruct * ) malloc(sizeof(idStruct));
    int res;

    write(cliente[j], "WELCOME\n", sizeof("WELCOME\n"));

    int terminado = 1;
    while (terminado == 1) {
        read(cliente[j], buffer, BUFF_SIZE);
        if (strcmp(buffer, "REGISTRAR\n") == 0) {
            printf("Procediendo al registro del cliente\n");
            srand(time(NULL));
            int x = rand() % 100;
            int y = rand() % 100;
            res = x + y;
            printf("Prueba para el cliente: %d + %d = %d\n", x, y, res);
            char msg[BUFF_SIZE];
            sprintf(msg, "RESUELVE %d %d\n", x, y);
            write(cliente[j], msg, sizeof("RESULEVE %d %d\n"));
            memset(buffer, '\0', BUFF_SIZE);
        }
        if (strstr(buffer, "RESPUESTA") != NULL) {
            fragBuffer = fragBufferExtract(buffer);
            if (atoi(fragBuffer[1]) == res) {
                printf("Respuesta correcta por parte del cliente\n");
                file = fopen("usuarios_servidor.txt", "a+");
                memset(idUser, '\0', sizeof(idStruct));
                sprintf(idUser->clientId, "%s", random_id(8));
                sprintf(idUser->clientResult, "%d", res);
                sprintf(idUser->clientName, "%s", "Invitado");
                fprintf(file, "%s %s %s\n", idUser->clientId, idUser->clientResult, idUser->clientName);
                fclose(file);
                char msg[BUFF_SIZE];
                sprintf(msg, "REGISTRADO OK %s\n", idUser->clientId);
                write(cliente[j], msg, sizeof("REGISTADO OK\n") + sizeof(idUser->clientId));
            } else {
                printf("Respuesta incorrecta\nCerrando la conexion\n");
                write(cliente[j], "REGISTRADO ERROR\n", sizeof("REGISTRADO ERROR\n"));
                return EXIT_SUCCESS;
            }
        }
        if (strstr(buffer, "LOGIN") != NULL) {
            int comp = 1;
            fragBuffer = fragBufferExtract(buffer);
            file = fopen("usuarios_servidor.txt", "r");
            memset(buffer, '\0', BUFF_SIZE);
            char name[18];
            while (fgets(fileId, BUFF_SIZE, file) != NULL) {
                auxFragBuffer = fragBufferExtract(fileId);
                if (strcmp(auxFragBuffer[0], fragBuffer[1]) == 0 && strcmp(auxFragBuffer[1], fragBuffer[2]) == 0) {
                    comp = 0;
                    sprintf(name, "%s", auxFragBuffer[2]);
                }
            }
            fclose(file);
            if (comp == 0) {
                printf("Identificacion correcta\n");
                write(cliente[j], "LOGIN OK\n", sizeof("LOGIN OK\n"));
                sprintf(idUser->clientId, "%s", fragBuffer[1]);
                sprintf(idUser->clientName, "%s", name);
                if (i == 0) {
                    strcpy(jugador0, name);
                } else {
                    strcpy(jugador1, name);
                }

                break;

            } else {
                printf("Identificacion incorrecta\nCerrando la conexion\n");
                write(cliente[j], "LOGIN ERROR\n", sizeof("LOGIN ERROR\n"));
                return NULL;
            }
        }

        if (strstr(buffer, "SETNAME") != NULL) {
            fragBuffer = fragBufferExtract(buffer);
            file = fopen("usuarios_servidor.txt", "a+");
            memset(buffer, '\0', BUFF_SIZE);
            sprintf(idUser->clientName, "%s", fragBuffer[1]);
            fprintf(file, "%s %s %s\n", idUser-> clientId, idUser->clientResult, idUser->clientName);
            printf("El usuario ha cambiado de nombre: %s\n", fragBuffer[1]);
            if (i == 0) {
                strcpy(jugador0, fragBuffer[1]);
            } else {
                strcpy(jugador1, fragBuffer[1]);
            }
            write(cliente[j], "SETNAME OK\n", sizeof("SETNAME OK\n"));
            fclose(file);
            break;
        }
    }
    printf("AUTENTIFICACION O REGISTRO TERMINADO\n\n");

    return NULL;
}

void * rechazarConexion(void * args) {

    struct num_struct * num_j = args;
    int j = num_j-> num;
    while (1) {
        struct sockaddr_in direccionCliente;
        unsigned int tam_direc;
        cliente[j] = accept(sock, (void * ) &direccionCliente, &tam_direc);
        write(cliente[j], "FULL\n", sizeof("FULL\n"));
    }
}

void juego() {

    char buffer[BUFF_SIZE];
    char ** fragBuffer;
    char board[r][column];
    char msg[BUFF_SIZE];
    int total_moves;
    int player;
    int columnPlayer;
    int columSel;

    printf("Jugador 1: %s %d\nJugador 2: %s %d\n", jugador0, cliente[0], jugador1, cliente[1]);

    sprintf(msg, "START servidor %d %d\n", r, column);
    write(cliente[0], msg, sizeof("START servidor %d %d\n"));
    memset(msg, '\0', BUFF_SIZE);

    printf("START %s %d %d\n", jugador0, r, column);
    sleep(1);

    srand(time(NULL));
    int x = 0;
    if (x == 0) {
        printf("Empieza %s\n", jugador0);
        total_moves = 0;
        write(cliente[0], "URTURN\n", sizeof("URTURN\n"));
    } else {
        printf("Empieza el servidor\n");
        total_moves = 1;
    }

    inicialise_board(board);
    do {
        int g_move = 0;
        if (total_moves % 2 == 0)
            player = 0;
        else
            player = 1;
        total_moves++;
        show_board(board);
        do {
            if (player == 0) {
                read(cliente[player], buffer, BUFF_SIZE);
                buffTreat(buffer);

                if (strstr(buffer, "COLUMN") != NULL) {
                    fragBuffer = fragBufferExtract(buffer);
                    columnPlayer = atoi(fragBuffer[1]);
                    if (make_move(player, columnPlayer, board) == 0) {
                        printf("Jugada incorrecta.\n\n");
                        write(cliente[0], "COLUMN ERROR\n", sizeof("COLUMN ERROR\n"));
                    } else {
                        g_move = 1;
                        write(cliente[0], "COLUMN OK\n", sizeof("COLUMN OK\n"));
                        if (check_end(board) != -1) break;
                    }
                }

            } else if (player == 1) {

                int hayTres = encuentraTres(board);
                int hayDos = encuentraDos(board);

                if (hayTres != -1) {
                    columSel = hayTres;
                } else if (hayDos != -1) {
                    columSel = hayDos;
                } else {
                    columSel = buscarEspacio(board);
                    if (total_moves == 1)
                        columSel = 1;

                }
                printf("Columna Server: %d\n", columSel);
                make_move(player, columSel, board);
                printf("Envio URTURN %d a cliente[0]: %d\n", columSel, cliente[0]);
                sprintf(msg, "URTURN %d\n", columSel);
                write(cliente[0], msg, sizeof("URTURN %d\n"));
                g_move = 1;
                if (check_end(board) != -1) break;
            }
        } while (g_move == 0);
    } while (check_end(board) == -1);

    show_board(board);
    sleep(1);
    //show_board(board);
    int ck_end = check_end(board);

    if (ck_end == 1) {
        printf("%s ha ganado la partida\n\n", jugador0);
        write(cliente[0], "VICTORY\n", sizeof("VICTORY\n"));
    } else if (ck_end == 0) {
        printf("Servidor ha ganado la partida\n\n");
        write(cliente[0], "DEFEAT\n", sizeof("DEFEAT\n"));
    } else if (ck_end == 2) {
        printf("Resultado: Empate\n\n");
        write(cliente[0], "TIE\n", sizeof("TIE\n"));
    }

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
    printf("PANEL DEL SERVIDOR\n\n");
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
    printf("  Adversario: %s\n\n", jugador0);
}

int check_end(char board[r][column]) {
    int i, j;
    int winner;
    winner = -1;

    //HORIZONTAL
    for (i = 0; i < r; i++) {
        for (j = 0; j < column - 3; j++) {
            if (board[i][j] == 'X' && board[i][j + 1] == 'X' && board[i][j + 2] == 'X' && board[i][j + 3] == 'X')
                winner = 0;
            else if (board[i][j] == '0' && board[i][j + 1] == '0' && board[i][j + 2] == '0' && board[i][j + 3] == '0')
                winner = 1;
        }
    }
    if (winner == -1) {
        //VERTICAL
        for (i = 0; i < r - 3; i++) {
            for (j = 0; j < column; j++) {
                if (board[i][j] == 'X' && board[i + 1][j] == 'X' && board[i + 2][j] == 'X' && board[i + 3][j] == 'X')
                    winner = 0;
                else if (board[i][j] == '0' && board[i + 1][j] == '0' && board[i + 2][j] == '0' && board[i + 3][j] == '0')
                    winner = 1;
            }
        }
    }

    if (winner == -1) {
        //DIAGONAL 1
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 3 < r && j + 3 < column) {
                    if (board[i][j] == 'X' && board[i + 1][j + 1] == 'X' && board[i + 2][j + 2] == 'X' && board[i + 3][j + 3] == 'X')
                        winner = 0;
                    else if (board[i][j] == '0' && board[i + 1][j + 1] == '0' && board[i + 2][j + 2] == '0' && board[i + 3][j + 3] == '0')
                        winner = 1;
                }
            }
        }
    }

    if (winner == -1) {
        //DIAGONAL 2
        for (i = 0; i < r; i++) {
            for (j = 0; j < column; j++) {
                if (i + 3 < r && j - 3 >= 0) {
                    if (board[i][j] == 'X' && board[i + 1][j - 1] == 'X' && board[i + 2][j - 2] == 'X' && board[i + 3][j - 3] == 'X')
                        winner = 0;
                    else if (board[i][j] == '0' && board[i + 1][j - 1] == '0' && board[i + 2][j - 2] == '0' && board[i + 3][j - 3] == '0')
                        winner = 1;
                }
            }
        }
    }
    //EMPATE
    if (winner == -1) {
        int full = 0;
        for (i = 0; i < column; i++) {
            if (board[0][i] != ' ')
                full++;
        }
        if (full == column)
            winner = 2;
    }
    return winner;
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
