// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
     {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}},
    {{{2, 0, 0, 0}, {2, 2, 2, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 2, 2, 0}, {0, 2, 0, 0}, {0, 2, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {2, 2, 2, 0}, {0, 0, 2, 0}, {0, 0, 0, 0}},
     {{0, 2, 0, 0}, {0, 2, 0, 0}, {2, 2, 0, 0}, {0, 0, 0, 0}}},
    {{{0, 0, 3, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 3, 0, 0}, {0, 3, 0, 0}, {0, 3, 3, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {3, 3, 3, 0}, {3, 0, 0, 0}, {0, 0, 0, 0}},
     {{3, 3, 0, 0}, {0, 3, 0, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}}},
    {{{0, 4, 4, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 4, 4, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 4, 4, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 4, 4, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
    {{{0, 5, 5, 0}, {5, 5, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 5, 0, 0}, {0, 5, 5, 0}, {0, 0, 5, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 5, 5, 0}, {5, 5, 0, 0}, {0, 0, 0, 0}},
     {{5, 0, 0, 0}, {5, 5, 0, 0}, {0, 5, 0, 0}, {0, 0, 0, 0}}},
    {{{0, 6, 0, 0}, {6, 6, 6, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 6, 0, 0}, {0, 6, 6, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {6, 6, 6, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}},
     {{0, 6, 0, 0}, {6, 6, 0, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}}},
    {{{7, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 7, 0}, {0, 7, 7, 0}, {0, 7, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {7, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}},
     {{0, 7, 0, 0}, {7, 7, 0, 0}, {7, 0, 0, 0}, {0, 0, 0, 0}}}};

SDL_Color colors[8] = {{30, 30, 30, 255},  {0, 255, 255, 255}, {0, 0, 255, 255},
                       {255, 165, 0, 255}, {255, 255, 0, 255}, {0, 255, 0, 255},
                       {128, 0, 128, 255}, {255, 0, 0, 255}};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS];
    struct RowNode *next;
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type;
    struct PieceQueueNode *next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode *next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;
    int rotation;
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode *first_row;
    PieceQueueNode *queue_head;
    PieceQueueNode *queue_tail;
    ParticleNode *first_particle;
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
GameBoard *game_board = NULL;

// ========= FUNCIONES AUXILIARES =========
// Devuelve la longitud del string sin contar el '\0' final
int len(char *src1) {
    int i = 0;
    while (src1[i] != '\0')
        i++;
    return i;
}

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char *strDuplicate(char *src) {
    int cont = len(src);
    // Reservamos len(src) + 1 bytes: el lugar extra es para el '\0', que
    // marca el fin del string.
    char *dupli = (char *)malloc(sizeof(char) * (cont + 1));
    // Si malloc falla devolvemos NULL para que se pueda detectar.
    if (dupli == NULL) {
        return NULL;
    }
    for (int i = 0; i < cont; i++) {
        dupli[i] = src[i];
    }
    dupli[cont] = '\0';
    return dupli;
}

int strCompare(char *s1, char *s2) {
    // Devuelve 0 si son iguales, 1 si s1 < s2 y -1 si s1 > s2
    // Avanzamos mientras coincidan; si uno es prefijo del
    // otro, el corte cae en su '\0', que es menor que cualquier caracter.
    int i = 0;
    while (s1[i] != 0 && s1[i] == s2[i]) {
        i++;
    }
    if (s1[i] < s2[i]) {
        return 1;
    }
    if (s1[i] > s2[i]) {
        return -1;
    }
    return 0;
}

char *strConcatenate(char *src1, char *src2) {
    // src1 y src2 vienen de malloc y son punteros distintos,
    // porque al final se les hace free.
    int i;
    int largo1 = len(src1);
    int largo2 = len(src2);
    int largo = largo1 + largo2;

    // Un solo bloque con lugar para los dos strings mas el '\0'. Hace falta
    // memoria nueva porque src1 no tiene espacio para agregarle src2.
    char *new = (char *)malloc(sizeof(char) * (largo + 1));
    if (new == NULL) {
        return NULL;
    }
    for (i = 0; i < largo1; i++) {
        new[i] = src1[i];
    }
    for (i = largo1; i < largo; i++) {
        new[i] = src2[i - largo1];
    }
    new[largo] = '\0';
    // Liberamos los originales recien despues de copiarlos: si los
    // liberaramos antes, tendriamos un acceso invalido a la memoria.
    free(src1);
    free(src2);

    return new;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

GameBoard *gameBoardNew() {
    GameBoard *new = (GameBoard *)malloc(sizeof(GameBoard));
    if (new == NULL) {
        return NULL;
    }
    // Todas las listas arrancan vacias (NULL), sin piezas y en 0.
    // Esto es necesario antes del bucle: si falla un malloc de fila, la
    // limpieza recorre first_row y tiene que encontrar un NULL al final.
    new->first_row = NULL;
    new->queue_head = NULL;
    new->queue_tail = NULL;
    new->first_particle = NULL;
    new->score = 0;
    new->tick_timer = 0;

    new->current_piece.col = 0;
    new->current_piece.row = 0;
    new->current_piece.type = 0;
    new->current_piece.rotation = 0;
    new->current_piece.active = 0;

    // Una fila por nodo, cada una con su malloc. Insertamos cada fila nueva
    // adelante de first_row: asi no hace falta recorrer la lista para
    // agregar al final, y como todas son iguales el orden no importa.
    for (int i = GRID_ROWS; i > 0; i--) {
        RowNode *fila = (RowNode *)malloc(sizeof(RowNode));
        if (fila == NULL) {
            // Si falla a mitad de camino liberamos las filas ya creadas y el
            // tablero, para no dejar memoria perdida, y devolvemos NULL.
            while (new->first_row != NULL) {
                RowNode *sig = new->first_row->next;
                free(new->first_row);
                new->first_row = sig;
            }
            free(new);
            return NULL;
        }
        for (int j = 0; j < GRID_COLS; j++) {
            fila->cells[j] = 0;
        }
        fila->next = new->first_row;
        new->first_row = fila;
    }
    return new;
}

void gameBoardDelete(GameBoard *board) {
    if (board == NULL) {
        return;
    }

    // Las tres listas se liberan igual: guardamos el siguiente ANTES del
    // free, porque despues de liberar un nodo ya no se puede leer su next.
    while (board->first_row != NULL) {
        RowNode *sig = board->first_row->next;
        free(board->first_row);
        board->first_row = sig;
    }

    while (board->queue_head != NULL) {
        PieceQueueNode *sig = board->queue_head->next;
        free(board->queue_head);
        board->queue_head = sig;
    }
    while (board->first_particle != NULL) {
        ParticleNode *sig = board->first_particle->next;
        free(board->first_particle);
        board->first_particle = sig;
    }
    // El tablero va al final, si lo liberaramos primero perderiamos
    // los punteros a las listas.
    free(board);
}

void gameBoardAddParticle(GameBoard *board, float x, float y, int color) {
    // Cada particula es un nodo propio en el heap, asi la cantidad no es
    // fija como el MAX_PARTICLES.
    ParticleNode *new = (ParticleNode *)malloc(sizeof(ParticleNode));
    if (new == NULL) {
        return;
    }
    new->x = x;
    new->y = y;
    new->color = color;
    new->life_timer = 30 + (rand() % 30);
    new->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    new->vel_y = ((rand() % 50) / -10.0f) - 2.0f;

    // Agregamos la particula en la cabeza por lo que no hay que recorrer
    // la lista. Primero enganchamos el nodo a la lista vieja y despues
    // movemos la cabeza.
    new->next = board->first_particle;
    board->first_particle = new;
}

int gameBoardCheckAndClearLines(GameBoard *board) {
    // Borrar una fila es solo volver a enlazar los punteros: no se copia
    // ninguna celda. Por cada fila borrada se agrega una vacia arriba, asi el
    // tablero siempre tiene GRID_ROWS filas.
    int lineas = 0;       // cuántas líneas eliminamos
    RowNode *atras = NULL; // fila anterior a "actual"
    RowNode *actual = board->first_row;
    int i = 0; // número de fila para las partículas

    // Recorremos la lista de RowNode
    while (actual != NULL) {
        // Guardamos la próxima fila ANTES de que "actual" pueda ser liberada
        RowNode *siguiente = actual->next;

        // Suponemos que todas las celdas son mayores a 0.
        int completa = 1;
        for (int j = 0; j < GRID_COLS && completa == 1; j++) {
            if (actual->cells[j] <= 0) {
                completa = 0;
            }
        }

        if (completa == 1) {
             // Reservamos la fila nueva ANTES de tocar la lista: si
             // el malloc falla, salimos sin cambios y el tablero
             // sigue con GRID_ROWS filas
            RowNode *nueva = (RowNode *)malloc(sizeof(RowNode));
            if (nueva == NULL) {
                return lineas;
            }
            for (int j = 0; j < GRID_COLS; j++) {
                nueva->cells[j] = 0;
            }

            // Guardamos los colores ANTES del free: las partículas
            // se crean al final, cuando la fila ya no existe
            int colores[GRID_COLS];
            for (int j = 0; j < GRID_COLS; j++) {
                colores[j] = actual->cells[j];
            }

            // Desenlazamos y hacemos free.
            // Si no hay fila anterior, el que apuntaba a "actual" era
            // first_row.
            if (atras == NULL) {
                board->first_row = siguiente;
            } else {
                atras->next = siguiente;
            }
            free(actual);

            // Primero enganchamos, después movemos la cabeza
            nueva->next = board->first_row;
            board->first_row = nueva;

            // Si eliminamos la primera fila, ahora la de antes de
            // "siguiente" es la nueva
            if (atras == NULL) {
                atras = nueva;
            }

            // Una partícula por cada celda destruida, con el color guardado
            for (int j = 0; j < GRID_COLS; j++) {
                gameBoardAddParticle(
                    board, OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2,
                    OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2, colores[j]);
            }

            lineas++;
        } else {
            // La fila se queda: pasa a ser la anterior de la próxima
            atras = actual;
        }

        // Avanzamos con lo que guardamos (nunca leemos "actual" después del
        // free)
        actual = siguiente;
        i++;
    }

    return lineas;
}

void gameBoardEnqueuePiece(GameBoard *board, int type) {
    // Un nodo por pieza en el heap: la cola crece sin el tope fijo del
    // QUEUE_SIZE.
    PieceQueueNode *new = (PieceQueueNode *)malloc(sizeof(PieceQueueNode));
    if (new == NULL) {
        return;
    }
    new->type = type;
    new->next = NULL; // va a ser el ultimo, no tiene siguiente

    // Guardamos queue_tail para insertar al final sin recorrer la cola.
    // Si la cola esta vacia, el nodo nuevo es a la vez cabeza y cola; si
    // no, lo enganchamos detras del tail actual y pasa a ser el tail.
    if (board->queue_head == NULL) {
        board->queue_head = new;
        board->queue_tail = new;
    } else {
        board->queue_tail->next = new;
        board->queue_tail = new;
    }
}

int gameBoardDequeuePiece(GameBoard *board, int *out_type) {
    // Cola vacia: devolvemos 0 sin tocar *out_type.
    if (board->queue_head == NULL) {
        return 0;
    }
    // Copiamos el tipo y guardamos el siguiente ANTES del free: despues de
    // liberar la cabeza ya no se pueden leer sus campos.
    *out_type = board->queue_head->type;
    PieceQueueNode *aux = board->queue_head->next;
    free(board->queue_head);
    board->queue_head = aux;
    // Si sacamos el ultimo nodo, tail apuntaria a memoria liberada: lo
    // ponemos en NULL para que el proximo Enqueue vea la cola vacia.
    if (board->queue_head == NULL) {
        board->queue_tail = NULL;
    }
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode *getRowAt(GameBoard *board, int index) {
    RowNode *curr = board->first_row;
    for (int i = 0; i < index && curr != NULL; i++)
        curr = curr->next;
    return curr;
}

int canMove(GameBoard *board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS ||
                    board_row >= GRID_ROWS)
                    return 0;
                if (board_row >= 0) {
                    RowNode *row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0)
                        return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard *board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type]
                                [board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode *row_node =
                    getRowAt(board, board->current_piece.row + r);
                if (row_node)
                    row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard *board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row,
                     board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0; // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard *board) {
    if (!board)
        return 0;
    if (!board->current_piece.active && !spawnNextPiece(board))
        return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col,
                    board->current_piece.row + 1,
                    board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode *curr_part = board->first_particle;
    ParticleNode *prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode *to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard *board) {
    if (!board)
        return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2,
                           GRID_COLS * CELL_SIZE + 4,
                           GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode *current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE,
                                      OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1,
                                      CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for (int pr = 0; pr < 4; pr++) {
            for (int pc = 0; pc < 4; pc++) {
                int cell_val =
                    TETROMINOS[board->current_piece.type]
                              [board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {
                        OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE,
                        OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE,
                        CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode *curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8; // se satura: SDL recibe un Uint8
        if (alpha > 255)
            alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
        return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS =========
// Casos minimos del enunciado. La memoria que piden los tests (strings y
// tableros) se libera, asi make valgrind no reporta perdidas.

void correrTests() {
    // strDuplicate

    // String vacío
    char *d1 = strDuplicate("");
    assert(d1 != NULL && strCompare(d1, "") == 0);
    free(d1);

    // String de un caracter
    char *d2 = strDuplicate("a");
    assert(d2 != NULL && strCompare(d2, "a") == 0);
    free(d2);

    // String con distintos caracteres
    char *original = "Hola, TD2! 123";
    char *d3 = strDuplicate(original);
    assert(d3 != NULL && strCompare(d3, original) == 0);
    assert(d3 != original); // tiene que ser memoria nueva, no el mismo puntero
    free(d3);

    // strCompare

    // Dos strings vacíos
    assert(strCompare("", "") == 0);

    // Dos strings de un caracter
    assert(strCompare("a", "a") == 0);
    assert(strCompare("a", "b") == 1);
    assert(strCompare("b", "a") == -1);

    // Dos strings distintos
    assert(strCompare("casa", "casas") == 1);
    assert(strCompare("casas", "casa") == -1);

    // strConcatenate

    // Los argumentos se crean con strDuplicate porque strConcatenate les hace free
    // String vacío y string de 3 caracteres
    char *c1 = strConcatenate(strDuplicate(""), strDuplicate("abc"));
    assert(c1 != NULL && strCompare(c1, "abc") == 0);
    free(c1); // solo el resultado: los argumentos ya los liberó strConcatenate

    // Dos strings de 5 caracteres
    char *c2 = strConcatenate(strDuplicate("hola "), strDuplicate("mundo"));
    assert(c2 != NULL && strCompare(c2, "hola mundo") == 0);
    free(c2);

    // Cola (push/pop)
    GameBoard *board = gameBoardNew();
    assert(board != NULL);

    int n = 5;
    int tipo;
    int r;

    for (int i = 0; i < n; i++) {
        gameBoardEnqueuePiece(board, i);
    }

    for (int i = 0; i < n; i++) {
        r = gameBoardDequeuePiece(board, &tipo);
        assert(r == 1 && tipo == i);
    }

    // 6to pop: la cola está vacía
    r = gameBoardDequeuePiece(board, &tipo);
    assert(r == 0);
    assert(board->queue_head == NULL);
    assert(board->queue_tail == NULL);

    // Un push más después de vaciarla: tiene que poder volver a extraerse
    gameBoardEnqueuePiece(board, 6);
    r = gameBoardDequeuePiece(board, &tipo);
    assert(r == 1 && tipo == 6);

    gameBoardDelete(board);

    // Test 2: llenar la fila 19
    board = gameBoardNew();
    assert(board != NULL);

    RowNode *fila19 = getRowAt(board, 19);
    for (int j = 0; j < GRID_COLS; j++) {
        fila19->cells[j] = 1;
    }
    RowNode *primeraVieja = board->first_row;

    r = gameBoardCheckAndClearLines(board);
    assert(r == 1);
    assert(board->first_row != primeraVieja);
    for (int j = 0; j < GRID_COLS; j++) {
        assert(board->first_row->cells[j] == 0);
    }

    int cant = 0;
    RowNode *fila = board->first_row;
    while (fila != NULL) {
        cant++;
        fila = fila->next;
    }
    assert(cant == 20);

    gameBoardDelete(board);

    // Test 3: llenar las filas 0 y 1
    board = gameBoardNew();
    assert(board != NULL);

    RowNode *fila0 = getRowAt(board, 0);
    RowNode *fila1 = getRowAt(board, 1);
    for (int j = 0; j < GRID_COLS; j++) {
        fila0->cells[j] = 2;
        fila1->cells[j] = 3;
    }

    r = gameBoardCheckAndClearLines(board);
    assert(r == 2);

    cant = 0;
    fila = board->first_row;
    while (fila != NULL) {
        cant++;
        fila = fila->next;
    }
    assert(cant == 20);

    gameBoardDelete(board);

    printf("Todos los tests pasaron.\n");
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char *arg) {
    char *flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i])
        i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char *args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind
    // solo reporta la memoria del TP (SDL tiene bloques propios que no se
    // liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1]))
        return 0;

    if (!inicializar())
        return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el "
               "juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for (int i = 0; i < 3; i++)
        gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT &&
                    canMove(game_board, game_board->current_piece.col - 1,
                            game_board->current_piece.row,
                            game_board->current_piece.rotation))
                    game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT &&
                    canMove(game_board, game_board->current_piece.col + 1,
                            game_board->current_piece.row,
                            game_board->current_piece.rotation))
                    game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN &&
                    canMove(game_board, game_board->current_piece.col,
                            game_board->current_piece.row + 1,
                            game_board->current_piece.rotation))
                    game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col,
                                game_board->current_piece.row, next_rot))
                        game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                    while (canMove(game_board, game_board->current_piece.col,
                                   game_board->current_piece.row + 1,
                                   game_board->current_piece.rotation))
                        game_board->current_piece.row++;
                    lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board))
            game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
