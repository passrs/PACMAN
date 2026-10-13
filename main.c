//TRABALHO DE CONSTRUÇÃO DE ALGORITMOS - P2
//PROFESSOR: EUGÊNIO DA SILVA
//ALUNOS: DANIEL VAZ LOUREIRO ALVEZ DA CONCEIÇÃO
//        PAULO ANDRÉ SOTERO DA SILVA RIBEIRO
//TURMA: CC

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define LINHA 21
#define COLUNA 30
#define QTD_FANTASMAS 5
#define VERMELHO "\x1b[31m"
#define ROSA     "\033[38;5;205m"
#define LARANJA  "\033[38;5;208m"
#define AMARELO  "\033[33m"
#define AZUL     "\x1b[34m"
#define VERDE    "\033[32m"
#define RESET    "\x1b[0m"

char mapa[LINHA][COLUNA]; //VARIAVEL GLOBAL QUE ARMAZENA O MAPA


typedef struct {
	char token;
	int x;
	int y;
	int delay;
	int perfil;
	int ultima_direcao;
} Fantasma;

typedef struct {
	char token;
	int x;
	int y;
	int ultima_jogada;
} Jogador;

// calcula a pontuação máxima que o jogador pode alcançar na partida
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

// funçao que decide aleatoriamente qual mapa será escolhido
int escolhe_mapa(int *mapa_escolhido) {
	srand(time(NULL));
	*mapa_escolhido = (rand() % 3) + 1;
	return *mapa_escolhido;
}

// gera o mapa escolhido com base no valor da funcao "escolha_mapa"
void gera_mapa(int *mapa_escolhido, int *pmax) {
	FILE *arquivo = fopen("mapas.txt", "r");
	if (arquivo == NULL) {
		printf("Erro ao abrir o arquivo mapas.txt\n");
		exit(1);
	}

	// com base no número obtido da funcao "escolha_mapa", irá selecionar a faixa das linhas do arquivo "mapas.txt", sendo:
	// mapa_1: da linha 5 até a linha 25
	// mapa_2: da linha 30 até a linha 50
	// mapa_3: da linha 55 até a linha 75

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

// funcao responsavel por mostrar o mapa na tela.
// o 'RESET' e as constantes com nome de cores servem apenas para mudar a cor dos fantasmas e do jogador
void printa_mapa(Jogador *jogador, Fantasma fantasmas[]) {
	int tem_fantasma;
	for (int i = 0; i < LINHA; i++) {
		puts("");
		for (int j = 0; j < COLUNA - 3; j++) {
			tem_fantasma = 0;
			for (int k = 0; k < QTD_FANTASMAS; k++) {
				if (fantasmas[k].x == i && fantasmas[k].y == j) {
					switch (k) {
						case 0:
							printf("%s%c%s ", VERMELHO, fantasmas[k].token, RESET);
							break;
						case 1: 
							printf("%s%c%s ", ROSA, fantasmas[k].token, RESET);
							break;
						case 2: 
							printf("%s%c%s ", LARANJA, fantasmas[k].token, RESET);
							break;
						case 3: 
							printf("%s%c%s ", AZUL, fantasmas[k].token, RESET);
							break;
						case 4: 
							printf("%s%c%s ", VERDE, fantasmas[k].token, RESET);
							break;	
						default:
							break;
					}
					tem_fantasma = 1;
					break;
				}
			}

			if (tem_fantasma == 1) {
				continue;
			}

			if (jogador->x == i && jogador->y == j) {
				printf(AMARELO "%c" RESET " ", jogador->token);
				if (jogador->y == 0){
					printf("               ");
					}
			} else {
				if (j == 0) {
					printf("               %c ", mapa[i][j]);
				} else {
					printf("%c ", mapa[i][j]);
				} 
			}
		}
	}
	puts("");
}

// essa funcao é responsavel por ler a posiçao que o jogador quer ir e verificar
// se nao tem nenhuma parede naquela posiçao 

int verifica_movimento_valido(int *pos_x, int *pos_y, int direcao) {
	switch (direcao) {
		case 1: if (mapa[*pos_x - 1][*pos_y] == '#') return 0; return 1;
		case 2: if (mapa[*pos_x + 1][*pos_y] == '#') return 0; return 1;
		case 3: if (mapa[*pos_x][*pos_y - 1] == '#') return 0; return 1;
		case 4: if (mapa[*pos_x][*pos_y + 1] == '#') return 0; return 1;
	}
	return 0;
}

// essa funcao, após verificar se o movimento é valido, atualizando e retornando a nova posiçao do jogador.

void faz_movimento(int *pos_x, int *pos_y, int direcao) {
	if (verifica_movimento_valido(pos_x, pos_y, direcao)) {		
		switch (direcao) {
			case 1: 
				*pos_x = *pos_x < 1 ? 20 : *pos_x - 1; 
				break;
			case 2: 
				*pos_x = *pos_x > LINHA-2 ? 0 : *pos_x + 1; 
				break;
			case 3: 
				*pos_y -= 1; 
				break;
			case 4: 
				*pos_y += 1;
				break;
		}
	}
}

// essa funcao recebe os 2 valores das coordenadas do jogador e de cada fantasma para poder calcular e retornar
// a menor distancia entre o jogador e cada um dos fantasmas

int calcula_distancia_fantasma(int *pos_x_f, int *pos_y_f, int *pos_x_j, int *pos_y_j, int direcao) {

	// como estamos manipulando coordenadas em uma matriz as relaçoes para alterar as coordenadas se comportam de uma maneira diferente.
	// quando queremos que algo se mecha na horizontal é preciso que adicionar ou subtrair o número de sua coluna
	// e para alterar a posiçao na vertical é preciso adicionar ou subtrair o número de sua linha

	int prox_x = *pos_x_f;
	int prox_y = *pos_y_f;

	if (direcao == 1) { // 1 = cima (-1 na linha) 
		prox_x--;}
	else if (direcao == 2) { // 2 = baixo (+1 na linha)
		prox_x++;}
	else if (direcao == 3) { // 3 = esquerda (-1 na coluna)
		prox_y--;}  
	else if (direcao == 4) { // 4 = direita (+1 na coluna)
		prox_y++;}

	if (verifica_movimento_valido(pos_x_f, pos_y_f, direcao) == 1) {
		int m = (prox_x > *pos_x_j) ? prox_x - *pos_x_j : *pos_x_j - prox_x;
		int n = (prox_y > *pos_y_j) ? prox_y - *pos_y_j : *pos_y_j - prox_y;
		return m + n;
	}
	return 999;
}

// essa funcao é responsavel por alterar o perfil de cada fantasma.(O perfil é o responsavel por determinar o comportamento de cada fantasma)
void muda_perfil(Fantasma fantasmas[], int *jogadas) {
	for (int i = 0; i < QTD_FANTASMAS; i++) {
		if (fantasmas[i].delay != 0 && *jogadas % fantasmas[i].delay == 0) {
			fantasmas[i].perfil *= -1;
		}
	}
}

// essa funcao é reponsavel por determinar a direçao que os fantasmas vao seguir para chegar no jogador
int define_direcao_fantasma(Fantasma *fantasma, int *pos_x_j, int *pos_y_j) {
	int dists[4];
	int aux = 0;

	dists[0] = calcula_distancia_fantasma(&fantasma->x, &fantasma->y, pos_x_j, pos_y_j, 1);
	dists[1] = calcula_distancia_fantasma(&fantasma->x, &fantasma->y, pos_x_j, pos_y_j, 2);
	dists[2] = calcula_distancia_fantasma(&fantasma->x, &fantasma->y, pos_x_j, pos_y_j, 3);
	dists[3] = calcula_distancia_fantasma(&fantasma->x, &fantasma->y, pos_x_j, pos_y_j, 4);
	
	//determina a direção oposta dada a ultima direção
	switch (fantasma->ultima_direcao) {
		case 1: aux = 2; break; //cima <-> baixo
		case 2: aux = 1; break;
		case 3: aux = 4; break; //esquerda <-> direita
		case 4: aux = 3; break;
		default: aux = 0; break;
	}

	int melhor_dist = 999;
	int melhor_direcao = 0;

	//busca a melhor direção ignorando a meia volta
	for (int i = 0; i < 4; i++) {
		int dir_atual = i + 1;

		if (dists[i] != 999 && dir_atual != aux) {
			if (dists[i] < melhor_dist) {
				melhor_dist = dists[i];
				melhor_direcao = dir_atual;
			}
		}
	}

	//trata beco sem saída (meia volta é a unica opção possível)
	if (melhor_direcao == 0 && aux != 0 && dists[aux - 1] != 999) {
		melhor_direcao = aux;
	}
	
	fantasma->ultima_direcao = melhor_direcao;
	return melhor_direcao;
}

//essa funcao realiza os movimentos dos fantasmas com base na posiçao do jogador e no perfil de cada fantasma
void faz_movimento_fantasma(Fantasma fantasmas[], int *pos_x_j, int *pos_y_j, int *jogadas) {
	muda_perfil(fantasmas, jogadas);
	for (int i = 0; i < QTD_FANTASMAS; i++) {
		if (fantasmas[i].perfil == 1) {
			int px = *pos_x_j;
			int py = *pos_y_j;

			switch (i) {
				case 1: px += 3; break; //flaqueador superior
				case 0: px -= 3; break; //flaqueador inferior
				case 3: py += 5; break; //flaqueador direita
				case 4: py -= 5; break; //flaqueador esquerda
				default: break;
			}

			faz_movimento(&fantasmas[i].x, &fantasmas[i].y, define_direcao_fantasma(&fantasmas[i], &px, &py));
		} else if (*jogadas > fantasmas[i].delay){ //só permite o fantasma se mover se o delay dele já foi atingido
			faz_movimento(&fantasmas[i].x, &fantasmas[i].y, define_direcao_fantasma(&fantasmas[i], pos_x_j, pos_y_j));
		}
	}
}

// lê a direçao do jogador
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

// verifica o estado do jogo, verificando para cada fantasma se a posicao deles é igual a do jogador
// retorna 1 se o jogador perdeu, 2  se ele ganhou e 0 como default
int verifica_estado_jogo(Jogador *jogador, Fantasma fantasmas[], int *pontos, int *pmax) {
	for (int i = 0; i < QTD_FANTASMAS; i++) {
		if (fantasmas[i].x == jogador->x && fantasmas[i].y == jogador->y) return 1;
	}
	if (*pontos == *pmax && *pmax > 0) return 2;
	return 0;
}

//aplica a pontuacao do jogo armazenando-a no endereço da variavel *pontos
void aplica_pontuacao(int *pos_x, int *pos_y, int *pontos) {
	if (mapa[*pos_x][*pos_y] == '.') {
		mapa[*pos_x][*pos_y] = ' ';
		*pontos += 100;
	}
}

//limpa a tela do terminal
void limpa_tela() {
	printf("\033[H\033[J");
}

int main() {
	limpa_tela();
	char input;
	int pontos = 0, mapa_escolhido, pmax, jogadas = 0;
	Jogador jogador = {'P', 15, 13, 3};

	Fantasma fantasmas[QTD_FANTASMAS] = {
		{'G', 9, 11, 35, -1, 1}, 
		{'G', 9, 12, 100, -1, 1}, 
		{'G', 9, 13, 0, 1, 1}, 
		{'G', 9, 14, 100, -1, 1}, 
		{'G', 9, 15, 35, -1, 1}
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
		
		if (verifica_estado_jogo(&jogador, fantasmas, &pontos, &pmax) == 1){
			limpa_tela();
			printa_mapa(&jogador, fantasmas);
			puts("----------------------------------------------------------------------------------");	
			printf("                               PONTOS: %d  / %d \n", pontos, pmax);
			puts("----------------------------------------------------------------------------------");	
			puts("----------------------------------------------------------------------------------");
			puts("                                    VOCE PERDEU!!!");
			puts("----------------------------------------------------------------------------------");
			break;
		}
		faz_movimento_fantasma(fantasmas, &jogador.x, &jogador.y, &jogadas);
		
		aplica_pontuacao(&jogador.x, &jogador.y, &pontos);
		
		limpa_tela();
		printa_mapa(&jogador, fantasmas);
		
		puts("----------------------------------------------------------------------------------");
		puts("'W', 'A', 'S' ou 'D' para se mover. 'ENTER' se move de acordo com a ultima jogada.");
		puts("----------------------------------------------------------------------------------");	
		printf("                               PONTOS: %d  / %d \n", pontos, pmax);
		puts("----------------------------------------------------------------------------------");
	}
	if (verifica_estado_jogo(&jogador, fantasmas, &pontos, &pmax) == 2) {
		puts("----------------------------------------------------------------------------------");
		puts("                                    VOCE VENCEU!!!");
		puts("----------------------------------------------------------------------------------");	
	}

	return 0;
}
