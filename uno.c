#include "uno.h"

void embaralhaNovamente (baralho *B, baralho *monte, int nCartasNoMonte) {
	carta aux;
	int r;

	time_t t;
	srand((unsigned)time(&t));

	for (int i = 0; i < nCartasNoMonte; ++i){
		r = (rand() % nCartasNoMonte);		
		aux = monte->monte[r];
		monte->monte[r] = monte->monte[i];
		monte->monte[i] = aux; 
	}

	for (int i = 0; i < nCartasNoMonte; ++i){
		B->monte[i] = monte->monte[i];
	}
	monte->n_cartas = 0;

	return;
}

int re_embaralha(baralho *B, baralho *D){
	//printf("EMABARALHANDOOOOO\n");
	int r,aux;

	aux = D->n_cartas - 1;
	for (int i = 0; i < D->n_cartas - 1; ++i)
	{
		r = (rand() % (aux));
		B->n_cartas++;
		B->monte[B->n_cartas] = D->monte[r];
		D->monte[r] = D->monte[aux];
		aux--;
	}
	D->monte[0] = D->monte[D->n_cartas];
	D->n_cartas = 0;
}

baralho* inicUNO (baralho *B){

//---------------------------------------------------------INICIA BARALHO---------------------
	B = malloc(sizeof(baralho));
	B->n_cartas = 56;

	for (int i = 0; i < 4 ; ++i) {

		for (int j = 0; j < VAR_DECK; ++j) {
			
			if (j < 10) {
				B->monte[i*VAR_DECK + j].tipo = j;
			} 
			else {

				if ((j == 10) || (j == 11)) {
					B->monte[i*VAR_DECK + j].tipo = 10;
				}
				if ((j == 12) || (j == 13)) {
					B->monte[i*VAR_DECK + j].tipo = 11;
				}

			}

			B->monte[i*VAR_DECK + j].cor = i;
		
		}
	}
//---------------------------------------------------------FINALIZA BARALHO-------------------


//---------------------------------------------------------INICIA EMBARALHAMENTO--------------
	carta aux;
	int r;

	time_t t;
	srand((unsigned)time(&t));

	for (int i = 0; i < TAM_DECK; ++i){
		r = (rand() % TAM_DECK);		
		aux = B->monte[r];
		B->monte[r] = B->monte[i];
		B->monte[i] = aux; 
	}
//---------------------------------------------------------FINALIZA EMBARALHAMENTO------------

	return B;
}