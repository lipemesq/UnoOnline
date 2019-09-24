#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define ANSI_COLOR_GREY    "\x1b[90m"

#define TAM_DECK 56 // 0-9 de cada cor + 8 pular + 8 "+2" 
#define VAR_DECK 14

typedef enum tipocarta {
	C0 = 0,C1,C2,C3,C4,C5,C6,C7,C8,C9,CBLOCK,CC2
} tipocarta;

typedef enum cores {
	vermelho = 0,verde,amarelo,azul,preta
} cores;

typedef struct carta{

	tipocarta tipo;
	cores cor;
} carta ;

typedef struct baralho{
	carta monte[56];
	int n_cartas;
} baralho;

void embaralhaNovamente (baralho *B, baralho *monte, int nCartasNoMonte);

int re_embaralha(baralho *B, baralho *D);

baralho* inicUNO (baralho *B);
