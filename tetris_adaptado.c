
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <stdio.h>

#define BASE 20
#define ALTURA 20
#define NUM_PECAS 10
#define TEMPO_INICIAL 30
#define TEMPO_INCREMENTO 10

typedef enum {
    I, O, T, S, Z, J, L
} peca_tipo;

typedef struct {
    int x, y;
    int rotacao;
    peca_tipo tipo;
} tetris;

typedef struct {
    int x, y;
} jogador;

typedef struct {
    int x, y;
} deus_tetris;

typedef struct {
    int x, y;
    int ativo;
} tiro;


int tela[ALTURA][BASE];  
tetris pecas[NUM_PECAS]; 
jogador player;          
deus_tetris deus;        
tiro tiro_jogador;       
int pontuacao = 0;       
int game_over = 0;       
int tempo_jogo = TEMPO_INICIAL; 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int tetris_formas[7][4][4][4] = {
    
    {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
     {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}},
    
    {{{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}},
    
    {{{0,0,0,0},{0,1,1,1},{0,0,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,0,1,0},{0,1,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,0,1,0},{0,1,1,1},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,1,0,0}}},
    
    {{{0,0,0,0},{0,0,1,1},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0}},
     {{0,0,0,0},{0,0,1,1},{0,1,1,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0}}},
    
    {{{0,0,0,0},{0,1,1,0},{0,0,1,1},{0,0,0,0}},
     {{0,0,0,0},{0,0,1,0},{0,1,1,0},{0,1,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,0,1,1},{0,0,0,0}},
     {{0,0,0,0},{0,0,1,0},{0,1,1,0},{0,1,0,0}}},
    
    {{{0,0,0,0},{0,1,0,0},{0,1,1,1},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,1,0,0},{0,1,0,0}},
     {{0,0,0,0},{0,1,1,1},{0,0,0,1},{0,0,0,0}},
     {{0,0,0,0},{0,0,1,0},{0,0,1,0},{0,1,1,0}}},
    
    {{{0,0,0,0},{0,0,0,1},{0,1,1,1},{0,0,0,0}},
     {{0,0,0,0},{0,1,0,0},{0,1,0,0},{0,1,1,0}},
     {{0,0,0,0},{0,1,1,1},{0,1,0,0},{0,0,0,0}},
     {{0,0,0,0},{0,1,1,0},{0,0,1,0},{0,0,1,0}}}
};


void init_tela() {
    int i, j;
    for (i = 0; i < ALTURA; i++) {
        for (j = 0; j < BASE; j++) {
            tela[i][j] = 0;
        }
    }
}

void desenhar_deus(deus_tetris* deus, int valor) {
    if (tela[deus->y][deus->x] != 2) {  
        tela[deus->y][deus->x] = valor;
    }
}

void print_tela() {
    int i, j;
    printf("\033[H\033[J");  

    for (i = 0; i < BASE + 2; i++) {
        printf("#");
    }
    printf("\n");

    for (i = 0; i < ALTURA; i++) {
        printf("#");
        for (j = 0; j < BASE; j++) {
            char display_char = '.';
            if (tela[i][j] == 1) {
                display_char = '#';
            } else if (tela[i][j] == 2) {
                display_char = 'P';
            } else if (tela[i][j] == 3) {
                display_char = 'T';
            } else if (tela[i][j] == 4) {
                display_char = '|';
            }
            printf("%c", display_char);
        }
        printf("#\n");
    }

    for (i = 0; i < BASE + 2; i++) {
        printf("#");
    }
    printf("\n");

    printf("Pontuação: %d\n", pontuacao);
    printf("Tempo: %d s\n", tempo_jogo);
}

void desenhar_peca(tetris* peca, int valor) {
    int i, j;
    int x, y;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetris_formas[peca->tipo][peca->rotacao][i][j]) {
                x = peca->x + j;
                y = peca->y + i;
                if (y >= 0 && x >= 0 && y < ALTURA && x < BASE && tela[y][x] != 3) {
                    tela[y][x] = valor;
                }
            }
        }
    }
}

void desenhar_jogador(jogador* player, int valor) {
    tela[player->y][player->x] = valor;
}

void desenhar_tiro(tiro* tiro, int valor) {
    if (tiro->ativo) {
        tela[tiro->y][tiro->x] = valor;
    }
}

int colisao(tetris* peca) {
    int i, j;
    int x_novo, y_novo;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetris_formas[peca->tipo][peca->rotacao][i][j]) {
                x_novo = peca->x + j;
                y_novo = peca->y + i;
                if (x_novo < 0 || x_novo >= BASE || y_novo >= ALTURA) {
                    return 1;
                }
                if (y_novo >= 0 && tela[y_novo][x_novo] == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int colisao_peca_jogador(tetris* peca) {
    int i, j;
    int x, y;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetris_formas[peca->tipo][peca->rotacao][i][j]) {
                x = peca->x + j;
                y = peca->y + i;
                if (x == player.x && y == player.y) {
                    return 1;  
                }
            }
        }
    }
    return 0;
}

void mover_deus_tetris() {
    deus.x = rand() % BASE;
    deus.y = rand() % ALTURA;
    desenhar_deus(&deus, 3);  
}

void remover_peca(tetris* peca) {
    desenhar_peca(peca, 0);  
    peca->x = rand() % (BASE - 4);
    peca->y = -4; 
    peca->rotacao = rand() % 4;
    peca->tipo = rand() % 7;
}

int colisao_jogador_deus() {
    if (player.x == deus.x && player.y == deus.y) {
        mover_deus_tetris();
        tempo_jogo += TEMPO_INCREMENTO;
        return 1;
    }
    return 0;
}

void* movimentar_tetris(void* arg) {
    int idx;
    tetris* peca;
    int sair = 0;  

    idx = *((int*)arg);
    peca = &pecas[idx];

    while (!sair) {
        pthread_mutex_lock(&mutex);
        if (game_over) {
            sair = 1;
        } else {
            desenhar_peca(peca, 0);  
            peca->y++;
            if (colisao(peca)) {
                peca->y--;
                desenhar_peca(peca, 0);  
                peca->x = rand() % (BASE - 4);
                peca->y = -4;  
                peca->rotacao = rand() % 4;
                peca->tipo = rand() % 7;
            } else {
                desenhar_peca(peca, 1);  
            }

            if (colisao_peca_jogador(peca)) {
                game_over = 1;
                desenhar_peca(peca, 1);
                sair = 1;
            }
        }
        pthread_mutex_unlock(&mutex);
        if (!sair) {
            usleep(300000);  
        }
    }
    pthread_exit(NULL);
    return NULL;
}

int processar_entrada(int ch) {
    if (ch == '\033') {  
        getchar();       
        switch (getchar()) {
            case 'A': return 'w';  
            case 'B': return 's';  
            case 'C': return 'd';  
            case 'D': return 'a';  
        }
    }
    return ch;
}

void atualizar_jogador(int ch) {
    desenhar_jogador(&player, 0);  

    if (ch == 'a') {
        player.x--;
        if (player.x < 0) player.x = 0;
    } else if (ch == 'd') {
        player.x++;
        if (player.x >= BASE) player.x = BASE - 1;
    } else if (ch == 'w') {
        player.y--;
        if (player.y < 0) player.y = 0;
    } else if (ch == 's') {
        player.y++;
        if (player.y >= ALTURA) player.y = ALTURA - 1;
    } else if (ch == ' ') {
        if (!tiro_jogador.ativo) {
            tiro_jogador.x = player.x;
            tiro_jogador.y = player.y - 1;
            tiro_jogador.ativo = 1;
        }
    }

    desenhar_jogador(&player, 2);  
}

void verificar_colisao_jogador() {
    int i;
    int colidiu = 0;  

    for (i = 0; i < NUM_PECAS && !colidiu; i++) {
        if (colisao_peca_jogador(&pecas[i])) {
            desenhar_jogador(&player, 2);
            game_over = 1;
            colidiu = 1;
        }
    }
}

void* usuario(void* arg) {
    (void)arg;  
    struct termios oldt, newt;
    int ch;
    int sair = 0;  

    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (!sair) {
        ch = getchar();
        ch = processar_entrada(ch);  

        pthread_mutex_lock(&mutex);
        if (game_over) {
            sair = 1;
        } else {
            atualizar_jogador(ch);  
            verificar_colisao_jogador();  
            if (colisao_jogador_deus()) {
                desenhar_jogador(&player, 2);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    pthread_exit(NULL);
    return NULL;
}



void mover_e_desenhar_tiro() {
    desenhar_tiro(&tiro_jogador, 0);  
    tiro_jogador.y--;
    if (tiro_jogador.y < 0) {
        tiro_jogador.ativo = 0;
    }
}


int colidir_com_deus() {
    return (tiro_jogador.x == deus.x && tiro_jogador.y == deus.y);
}

int colidir_com_peca() {
    int i, j, k;
    int colidiu = 0;
    int x, y;  
    tetris* peca;  

    for (i = 0; i < NUM_PECAS && !colidiu; i++) {
        peca = &pecas[i];
        for (j = 0; j < 4 && !colidiu; j++) {
            for (k = 0; k < 4 && !colidiu; k++) {
                if (tetris_formas[peca->tipo][peca->rotacao][j][k]) {
                    x = peca->x + k;
                    y = peca->y + j;
                    if (tiro_jogador.x == x && tiro_jogador.y == y) {
                        remover_peca(peca);
                        pontuacao += 50;
                        colidiu = 1;
                    }
                }
            }
        }
    }
    return colidiu;
}



void* mover_tiro(void* arg) {
    (void)arg;  
    int sair = 0;  

    while (!sair) {
        pthread_mutex_lock(&mutex);
        if (game_over) {
            sair = 1;
        } else if (tiro_jogador.ativo) {
            mover_e_desenhar_tiro();
            if (colidir_com_deus()) {
                desenhar_tiro(&tiro_jogador, 0);
            } else {
                desenhar_tiro(&tiro_jogador, 4);
            }
            if (colidir_com_peca()) {
                tiro_jogador.ativo = 0;
            }
        }
        desenhar_deus(&deus, 3);
        pthread_mutex_unlock(&mutex);
        if (!sair) {
            usleep(100000);
        }
    }
    pthread_exit(NULL);
    return NULL;
}



void* cronometro(void* arg) {
    (void)arg;  
    int sair = 0;  

    while (!sair) {
        sleep(1);
        pthread_mutex_lock(&mutex);
        tempo_jogo--;
        if (tempo_jogo <= 0) {
            game_over = 1;
            sair = 1;  
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
    return NULL;
}

void* renderizar_tela(void* arg) {
    (void)arg;  
    int sair = 0;  

    while (!sair) {
        pthread_mutex_lock(&mutex);
        if (game_over) {
            sair = 1;  
        } else {
            print_tela();
        }
        pthread_mutex_unlock(&mutex);
        if (!sair) {
            usleep(50000);  
        }
    }
    print_tela();  
    pthread_exit(NULL);
    return NULL;
}

void inicializar_jogo() {
    int i;

    mover_deus_tetris();   
    init_tela();
    desenhar_deus(&deus, 3);   

    for (i = 0; i < NUM_PECAS; i++) {
        pecas[i].tipo = rand() % 7;
        pecas[i].x = rand() % (BASE - 4);
        pecas[i].y = -4;  
        pecas[i].rotacao = rand() % 4;
    }

    player.x = BASE / 2;
    player.y = ALTURA - 1;

    tiro_jogador.ativo = 0;
}

int main() {
    pthread_t pecas_thread[NUM_PECAS], input_thread, cronometro_thread, render_thread, tiro_thread;
    int indices[NUM_PECAS];
    int i;

    srand(time(NULL));
    inicializar_jogo();  

    
    for (i = 0; i < NUM_PECAS; i++) {
        indices[i] = i;
        pthread_create(&pecas_thread[i], NULL, movimentar_tetris, &indices[i]);
    }
    pthread_create(&input_thread, NULL, usuario, NULL);
    pthread_create(&cronometro_thread, NULL, cronometro, NULL);
    pthread_create(&render_thread, NULL, renderizar_tela, NULL);
    pthread_create(&tiro_thread, NULL, mover_tiro, NULL);

    
    for (i = 0; i < NUM_PECAS; i++) {
        pthread_join(pecas_thread[i], NULL);
    }
    pthread_join(input_thread, NULL);
    pthread_join(cronometro_thread, NULL);
    pthread_join(render_thread, NULL);
    pthread_join(tiro_thread, NULL);

    return 0;
}
