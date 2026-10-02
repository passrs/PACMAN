#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define LINHA 21
#define COLUNA 30
#define QTD_FANTASMAS 5
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"

char mapa[LINHA][COLUNA];

typedef struct {
	char token;
	int x;
	int y;
	int delay_primario;
	int se_moveu;
} Fantasma;

typedef struct {
	char token;
	int x;
	int y;
	int ultima_jogada;
} Jogador;

int conta_pontuacao_maxima() {
	int total_comidas = 0;
	for (int i = 1; i < LINHA - 1; i++) {
		for (int j = 1; j < COLUNA - 1; j++) {
			if (mapa[i][j] == '.') {
				total_comidas++;
			}
		}
	}
	return total_comidas * 100;
}

int escolhe_mapa(int *mapa_escolhido) {
	srand(time(NULL));
	*mapa_escolhido = (rand() % 3) + 1;
	return *mapa_escolhido;
}

void gera_mapa(int *mapa_escolhido, int *pmax) {
	FILE *arquivo = fopen("mapas.txt", "r");
	if (arquivo == NULL) {
		printf("Erro ao abrir o arquivo mapas.txt\n");
		exit(1);
	}

	int linhaI_mapa = (*mapa_escolhido == 1) ? 5 : (*mapa_escolhido == 2) ? 30 : 55;
	int linhaF_mapa = (*mapa_escolhido == 1) ? 25 : (*mapa_escolhido == 2) ? 50 : 75;  
	
	char aux[COLUNA]; 
	int cont = 1, cont_Lmapa = 0;
	
	while (cont_Lmapa < LINHA && fgets(aux, COLUNA, arquivo) != NULL) {
		if (cont >= linhaI_mapa && cont <= linhaF_mapa) {
			strcpy(mapa[cont_Lmapa], aux);
			cont_Lmapa++;
		}
		cont++;
	}
	*pmax = conta_pontuacao_maxima();
	fclose(arquivo);	
} 

void printa_mapa(Jogador *jogador, Fantasma fantasmas[]) {
	int tem_fantasma;
	for (int i = 0; i < LINHA; i++) {
		puts("");
		for (int j = 0; j < COLUNA - 3; j++) {
			tem_fantasma = 0;
			for (int k = 0; k < QTD_FANTASMAS; k++) {
				if (fantasmas[k].x == i && fantasmas[k].y == j) {
					//colca aq os casos das cores SWITCH (K)
					printf("%s%c%s ", RED, fantasmas[k].token, RESET);
					tem_fantasma = 1;
					break;
				}
			}
			if (tem_fantasma == 1) continue;

			if (jogador->x == i && jogador->y == j) {
				printf("%c ", jogador->token);
			} else {
				if (j == 0) {
					printf("              %c ", mapa[i][j]);
				} else {
					printf("%c ", mapa[i][j]);
				} 
			}
		}
	}
	puts("");
}

int verifica_movimento_valido(int *pos_x, int *pos_y, int direcao) {
	switch (direcao) {
		case 1: if (mapa[*pos_x - 1][*pos_y] == '#') return 0; return 1;
		case 2: if (mapa[*pos_x + 1][*pos_y] == '#') return 0; return 1;
		case 3: if (mapa[*pos_x][*pos_y - 1] == '#') return 0; return 1;
		case 4: if (mapa[*pos_x][*pos_y + 1] == '#') return 0; return 1;
	}
	return 0;
}

int calcula_distancia_fantasma(int *pos_x_f, int *pos_y_f, int pos_x_alvo, int pos_y_alvo, int direcao) {
	// cima = 1 ; baixo = 2 ; esqeurda = 3 ; direita = 4
	int prox_x = *pos_x_f;
	int prox_y = *pos_y_f;

	if (direcao == 1) prox_x--;
	else if (direcao == 2) prox_x++;   
	else if (direcao == 3) prox_y--;   
	else if (direcao == 4) prox_y++;

	if (verifica_movimento_valido(pos_x_f, pos_y_f, direcao) == 1) {
		int m = (prox_x > pos_x_alvo) ? prox_x - pos_x_alvo : pos_x_alvo - prox_x;
		int n = (prox_y > pos_y_alvo) ? prox_y - pos_y_alvo : pos_y_alvo - prox_y;
		return m + n;
	}
	return 9999;
}

int define_direcao_fantasma(int *pos_x_f, int *pos_y_f, int pos_x_alvo, int pos_y_alvo) {
	int dists[4];
	dists[0] = calcula_distancia_fantasma(pos_x_f, pos_y_f, pos_x_alvo, pos_y_alvo, 1);
	dists[1] = calcula_distancia_fantasma(pos_x_f, pos_y_f, pos_x_alvo, pos_y_alvo, 2);
	dists[2] = calcula_distancia_fantasma(pos_x_f, pos_y_f, pos_x_alvo, pos_y_alvo, 3);
	dists[3] = calcula_distancia_fantasma(pos_x_f, pos_y_f, pos_x_alvo, pos_y_alvo, 4);

	int melhor_dist = 9999;
	int melhor_direcao = 0;

	for (int d = 0; d < 4; d++) {
		if (dists[d] < melhor_dist) {
			melhor_dist = dists[d];
			melhor_direcao = d + 1;
		}
	}

	return melhor_direcao;
}

void faz_movimento(int *pos_x, int *pos_y, int direcao) {
	if (verifica_movimento_valido(pos_x, pos_y, direcao)) {		
		switch (direcao) {
			case 1: *pos_x -= 1; break;
			case 2: *pos_x += 1; break;
			case 3: *pos_y -= 1; break;
			case 4: *pos_y += 1; break;
		}
	}
}

void faz_movimento_fantasma(Fantasma fantasmas[], int *pos_x_j, int *pos_y_j, int *jogadas, int dir_jogador) {
	int mov;
	for (int i = 0; i < QTD_FANTASMAS; i++) {
		if (fantasmas[i].delay_primario > *jogadas) continue;

		int alvo_x = *pos_x_j;
		int alvo_y = *pos_y_j;

		switch (i) {
			case 0: // Caçador Direto
				break;
			case 1: // Emboscador: Mira 3 blocos à frente do jogador
				if (dir_jogador == 1) alvo_x -= 3;
				else if (dir_jogador == 2) alvo_x += 3;
				else if (dir_jogador == 3) alvo_y -= 3;
				else if (dir_jogador == 4) alvo_y += 3;
				break;
			case 2: // Flanqueador Esquerdo
				alvo_y -= 3;
				break;
			case 3: // Flanqueador Direito
				alvo_y += 3;
				break;
			default: // caçador auxiliar
				alvo_x += 2;
				break;
		}

		mov = define_direcao_fantasma(&fantasmas[i].x, &fantasmas[i].y, alvo_x, alvo_y);
		faz_movimento(&fantasmas[i].x, &fantasmas[i].y, mov);
	}
}

int direcao_jogador(int *ultima_jogada, char *input) {
	switch (*input) {
		case 'W': case 'w': return 1;
		case 'S': case 's': return 2;
		case 'A': case 'a': return 3;
		case 'D': case 'd': return 4;
		case '\n': return *ultima_jogada;
		default: break;
	}
	return *ultima_jogada;
}

int verifica_estado_jogo(Jogador *jogador, Fantasma fantasmas[], int *pontos, int *pmax) {
	for (int i = 0; i < QTD_FANTASMAS; i++) {
		if (fantasmas[i].x == jogador->x && fantasmas[i].y == jogador->y) return 1;
	}
	if (*pontos == *pmax && *pmax > 0) return 2;
	return 0;
}

void aplica_pontuacao(int *pos_x, int *pos_y, int *pontos) {
	if (mapa[*pos_x][*pos_y] == '.') {
		mapa[*pos_x][*pos_y] = ' ';
		*pontos += 100;
	}
}

void limpa_tela() {
	printf("\033[H\033[J");
}

int main() {
	limpa_tela();
	char input;
	int pontos = 0, mapa_escolhido, pmax, jogadas = 0;
	Jogador jogador = {'P', 15, 13, 3};

	Fantasma fantasmas[QTD_FANTASMAS] = {
		{'G', 9, 11, 3, 0}, 
		{'G', 9, 12, 5, 0}, 
		{'G', 9, 13, 7, 0}, 
		{'G', 9, 14, 9, 0}, 
		{'G', 9, 15, 11, 0}
	};
	
	escolhe_mapa(&mapa_escolhido);
	gera_mapa(&mapa_escolhido, &pmax);
	printa_mapa(&jogador, fantasmas);

	puts("----------------------------------------------------------------------------------");
	puts("'W', 'A', 'S' ou 'D' para se mover. 'ENTER' se move de acordo com a ultima jogada.");
	puts("----------------------------------------------------------------------------------");
	
	while (verifica_estado_jogo(&jogador, fantasmas, &pontos, &pmax) == 0) {
		input = getchar();
		if (input != '\n') getchar();

		jogador.ultima_jogada = direcao_jogador(&jogador.ultima_jogada, &input);
		faz_movimento(&jogador.x, &jogador.y, jogador.ultima_jogada);
		
		jogadas += 1;
		faz_movimento_fantasma(fantasmas, &jogador.x, &jogador.y, &jogadas, jogador.ultima_jogada);
		
		aplica_pontuacao(&jogador.x, &jogador.y, &pontos);
		
		limpa_tela();
		printa_mapa(&jogador, fantasmas);
		
		puts("----------------------------------------------------------------------------------");
		puts("'W', 'A', 'S' ou 'D' para se mover. 'ENTER' se move de acordo com a ultima jogada.");
		puts("----------------------------------------------------------------------------------");	
		printf("                               PONTOS: %d  / %d \n", pontos, pmax);
		puts("----------------------------------------------------------------------------------");
	}

	if (verifica_estado_jogo(&jogador, fantasmas, &pontos, &pmax) == 1) {
		puts("----------------------------------------------------------------------------------");
		puts("                                    VOCE PERDEU!!!");
		puts("----------------------------------------------------------------------------------");	
	} else if (verifica_estado_jogo(&jogador, fantasmas, &pontos, &pmax) == 2) {
		puts("----------------------------------------------------------------------------------");
		puts("                                    VOCE VENCEU!!!");
		puts("----------------------------------------------------------------------------------");	
	}

	return 0;
}
