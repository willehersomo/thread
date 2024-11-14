#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>

#define BASE 10  
#define ALTURA 20 

typedef enum {
    I, O, T, S, Z, J, L
} peca_tipo;


typedef struct {
    int x, y; 
    int rotacao; 
    peca_tipo tipo; 
} tetris;


int tela[ALTURA][BASE]; 
tetris peca_atual; 
tetris proxima_peca; 
int pontuacao = 0; 
int game_over = 0; 
int tempo_jogo = 0; 
float velocidade = 1.0; 
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

void print_tela() {
    int i, j; 
    printf("\033[H\033[J"); 

    for (i = 0; i < ALTURA; i++) {
        for (j = 0; j < BASE; j++) {
            printf(tela[i][j] == 0 ? "." : "#");
        }
        printf("\n");
    }
    printf("Pontuação: %d\n", pontuacao);  
    printf("Tempo: %d s\n", tempo_jogo);  
    printf("Velocidade: %.2f\n", velocidade); 
    printf("Próxima peça:\n");

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf(tetris_formas[proxima_peca.tipo][0][i][j] ? "#" : ".");
        }
        printf("\n");
    }
}

void desenhar_peca(tetris* peca, int valor) {
    int i, j, x, y; 
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetris_formas[peca->tipo][peca->rotacao][i][j]) {
                x = peca->x + j;
                y = peca->y + i;
                if (y >= 0 && x >= 0 && y < ALTURA && x < BASE) {
                    tela[y][x] = valor;
                }
            }
        }
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
                if (x_novo < 0 || x_novo >= BASE || y_novo >= ALTURA || (y_novo >= 0 && tela[y_novo][x_novo])) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void mesclar_tetris(tetris* peca) {
    desenhar_peca(peca, 1);
}

void verificar_linhas_completas() {
    int i, j, k, l; 
    int linhas_completas = 0;
    int completo;
    for (i = 0; i < ALTURA; i++) {
        completo=1;
        for (j = 0; j < BASE; j++) {
            if (tela[i][j] == 0) {
                completo = 0;
            }
        }

        if (completo) {
            linhas_completas++;
            for (k = i; k > 0; k--) {
                for (l = 0; l < BASE; l++) {
                    tela[k][l] = tela[k - 1][l];
                }
            }
            for (l = 0; l < BASE; l++) {
                tela[0][l] = 0;
            }
        }
    }

    if (linhas_completas > 0) {
        pontuacao += 1000 * (2 * linhas_completas);
    }
}

void configurar_nova_peca() {
    peca_atual = proxima_peca;
    proxima_peca.tipo = rand() % 7;
    proxima_peca.x = BASE / 2 - 2;
    proxima_peca.y = 0;
    proxima_peca.rotacao = 0;

    if (colisao(&peca_atual)) {
        printf("Game Over!\n");
        game_over = 1; 
    }
}

void atualizar_posicao_peca() {
    desenhar_peca(&peca_atual, 0);  
    peca_atual.y++;

    if (colisao(&peca_atual)) {
        peca_atual.y--;
        desenhar_peca(&peca_atual, 1); 
        mesclar_tetris(&peca_atual);
        verificar_linhas_completas();
        configurar_nova_peca();
    }

    desenhar_peca(&peca_atual, 1);
}



void* movimentar_tetris(void* arg) {
    (void)arg;  

    int sair = 0; 
    while (!sair && !game_over) {
        pthread_mutex_lock(&mutex);
        atualizar_posicao_peca();
        if (game_over) {
            sair = 1; 
        }
        pthread_mutex_unlock(&mutex);
        usleep(500000 / velocidade); 
    }
    pthread_exit(NULL);
    return NULL;
}

void configurar_terminal(struct termios* oldt) {
    struct termios newt;
    tcgetattr(STDIN_FILENO, oldt);
    newt = *oldt;
    newt.c_lflag &= ~(ICANON | ECHO);                       
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restaurar_terminal(const struct termios* oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldt);
}

void mover_peca_esquerda() {
    peca_atual.x--;
    if (colisao(&peca_atual)) {
        peca_atual.x++;
    }
}

void mover_peca_direita() {
    peca_atual.x++;
    if (colisao(&peca_atual)) {
        peca_atual.x--;
    }
}

void rotacionar_peca() {
    peca_atual.rotacao = (peca_atual.rotacao + 1) % 4;
    if (colisao(&peca_atual)) {
        peca_atual.rotacao = (peca_atual.rotacao + 3) % 4; 
    }
}

void mover_peca_baixo() {
    peca_atual.y++;
    if (colisao(&peca_atual)) {
        peca_atual.y--;
    }
}

void* usuario(void* arg) {
    (void)arg;  
    struct termios oldt;
    configurar_terminal(&oldt);

    while (1) {
        int ch = getchar();
        pthread_mutex_lock(&mutex);
        desenhar_peca(&peca_atual, 0); 

        if (ch == 'a') {
            mover_peca_esquerda();
        } else if (ch == 'd') {
            mover_peca_direita();
        } else if (ch == 'w') {
            rotacionar_peca();
        } else if (ch == 's') {
            mover_peca_baixo();
        }

        desenhar_peca(&peca_atual, 1);  
        pthread_mutex_unlock(&mutex);
    }

    restaurar_terminal(&oldt);
    pthread_exit(NULL);
    return NULL;
}

void incrementar_tempo_jogo() {
    tempo_jogo++;
}

void* cronometro(void* arg) {
    (void)arg;  

    while (1) {
        sleep(1);
        pthread_mutex_lock(&mutex);
        incrementar_tempo_jogo();
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
    return NULL;
}

void ajustar_velocidade_jogo() {
    velocidade += 0.25;
}

void* gerenciar_velocidade(void* arg) {
    (void)arg;  

    while (1) {
        sleep(60); 
        pthread_mutex_lock(&mutex);
        ajustar_velocidade_jogo();
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
    return NULL;
}

void inicializar_jogo() {
    srand(time(NULL));
    init_tela();
    peca_atual.tipo = rand() % 7;
    peca_atual.x = BASE / 2 - 2;
    peca_atual.y = 0;
    peca_atual.rotacao = 0;
    proxima_peca.tipo = rand() % 7;
    proxima_peca.x = BASE / 2 - 2;
    proxima_peca.y = 0;
    proxima_peca.rotacao = 0;
}

void criar_threads(pthread_t* mover_thread, pthread_t* input_thread, pthread_t* cronometro_thread, pthread_t* velocidade_thread) {
    pthread_create(mover_thread, NULL, movimentar_tetris, NULL);
    pthread_create(input_thread, NULL, usuario, NULL);
    pthread_create(cronometro_thread, NULL, cronometro, NULL);
    pthread_create(velocidade_thread, NULL, gerenciar_velocidade, NULL);
}

void executar_loop_principal() {
    while (1) {
        pthread_mutex_lock(&mutex);
        print_tela();
        pthread_mutex_unlock(&mutex);
        usleep(50000);
    }
}

int main() {
    inicializar_jogo();

    pthread_t mover_thread, input_thread, cronometro_thread, velocidade_thread;
    criar_threads(&mover_thread, &input_thread, &cronometro_thread, &velocidade_thread);

    executar_loop_principal();

    pthread_join(mover_thread, NULL);
    pthread_join(input_thread, NULL);
    pthread_join(cronometro_thread, NULL);
    pthread_join(velocidade_thread, NULL);

    return 0;
}




