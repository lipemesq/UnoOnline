#include <arpa/inet.h>
#include <sys/socket.h>
 
#include "uno.h"

#define BUFLEN 512  


/* ESTRUTURA DO CONTROLE DA MENSAGEM */
typedef struct controle_t {
	int destino;
	int remetente;
	int bastao;
	int jogando;
	int nCartasNaMao;
	int uno;
	int comprar;
	int passarVez;
	int fimDoJogo; 
} controle_t;

/* ESTRUTURA DA MENSAGEM */
typedef struct msg_t {
	controle_t controle;
	carta umaCarta;
} msg_t;

/* ESTRUTURA PARA GUARDAR O IP DAS MÁQUINAS*/
struct end_ip {
  char end[14];
};

/* ESTRUTURA COM OS DADOS DA MÁQUINA DO JOGADOR */
typedef struct jogador {
  char porta[6];
  struct end_ip ips [4]; 
} jogadores;

// FUNÇÃO PARA DEBUG, MENSAGEM DE SAÍDA
void DeuRuimPorque (char* motivo) {
	perror (motivo);
	exit (-1);
}

/* ENDEREÇO DO SOCKET DESTA MAQUINA (EU) E O DA PRÓXIMA (PROX) */
struct sockaddr_in eu = {0}, prox = {0};
int s, slen = sizeof(prox); // SOCKET, TAMANHO DO ENDEREÇO E ESTRUTURA PRA RECEBER DADOS

/* FUNÇÃO QUE ENVIA UMA MENSAGEM QUALQUER PARA A PRÓXIMA MÁQUINA */
void EnviarMensagem (msg_t* novaMsg) {
	if (sendto(s, novaMsg, sizeof(msg_t), 0, (struct sockaddr*) &prox, slen) == -1) {
		DeuRuimPorque ("Não enviou uma mensagem");
	}
	return;
}

/* FUNÇÃO QUE RECEBE UMA MENSAGEM QUALQUER */
void RecebeMensagem (msg_t* novaMsg) {
	if (recv(s, novaMsg, sizeof(msg_t), 0) == -1) {
	    DeuRuimPorque ("recvfrom()");
	}
	return;
}

/* FORMATA A MENSAGEM PARA DAR UMA CARTA */
void DarCarta (baralho* B, msg_t* novaMsg, int jComprando, int n) {
	novaMsg->controle.destino 	= jComprando;
	novaMsg->controle.remetente	= n;
	novaMsg->controle.bastao 	= 0;
	novaMsg->controle.jogando 	= 0;
	novaMsg->controle.nCartasNaMao = 0;
	novaMsg->controle.uno 		= 0;
	novaMsg->controle.comprar 	= -1;
	novaMsg->controle.passarVez	= 0;
	novaMsg->controle.fimDoJogo	= 0;

	novaMsg->umaCarta = B->monte[B->n_cartas-1];
	B->n_cartas--;
}

/* FORMATA A MENSAGEM PARA JOGAR UMA CARTA */
void Jogar (carta c, msg_t* novaMsg, int cartasNaMao, int n) {
	novaMsg->controle.destino 	= n;
	novaMsg->controle.remetente = n;
	novaMsg->controle.bastao 	= 0;
	novaMsg->controle.jogando 	= 1;
	novaMsg->controle.nCartasNaMao = cartasNaMao;
	novaMsg->controle.uno 		= 0;
	novaMsg->controle.comprar 	= 0;
	novaMsg->controle.passarVez = 0;
	novaMsg->controle.fimDoJogo = 0;

	novaMsg->umaCarta = c;
}

/* FORMATA A MENSAGEM PARA PASSAR O BASTÃO */
void PassaBastao (msg_t* novaMsg, int prox, int n) {
	novaMsg->controle.destino 	= prox;
	novaMsg->controle.remetente	= n;
	novaMsg->controle.bastao 	= 1;
	novaMsg->controle.jogando 	= 0;
	novaMsg->controle.nCartasNaMao = 0;
	novaMsg->controle.uno 		= 0;
	novaMsg->controle.comprar 	= 0;
	novaMsg->controle.passarVez	= 0;
	novaMsg->controle.fimDoJogo	= 0;
}

/* FORMATA A MENSAGEM PARA PEDIR UMA CARTA AO DEALER */
void PedeCarta (msg_t* novaMsg, int d, int n){
	novaMsg->controle.destino 	= d;
	novaMsg->controle.remetente	= n;
	novaMsg->controle.bastao 	= 0;
	novaMsg->controle.jogando 	= 0;
	novaMsg->controle.nCartasNaMao = 0;
	novaMsg->controle.uno 		= 0;
	novaMsg->controle.comprar 	= 2;
	novaMsg->controle.passarVez	= 0;
	novaMsg->controle.fimDoJogo	= 0;	
}

/* FORMATA A MENSAGEM PARA AVISAR QUE ESTÁ PEDINDO UNO AOS OUTROS JOGADORES */
void GritaUno (msg_t* novaMsg, int n){
	novaMsg->controle.destino 	= n;
	novaMsg->controle.remetente	= n;
	novaMsg->controle.bastao 	= 0;
	novaMsg->controle.jogando 	= 0;
	novaMsg->controle.nCartasNaMao = 0;
	novaMsg->controle.uno 		= 1;
	novaMsg->controle.comprar 	= 0;
	novaMsg->controle.passarVez	= 0;
	novaMsg->controle.fimDoJogo	= 0;	
}

/* VITÓRIA, OU MORTE! */
void Ganhar (msg_t* novaMsg, int n){
	novaMsg->controle.destino 	= n;
	novaMsg->controle.remetente	= n;
	novaMsg->controle.bastao 	= 0;
	novaMsg->controle.jogando 	= 0;
	novaMsg->controle.nCartasNaMao = 0;
	novaMsg->controle.uno 		= 0;
	novaMsg->controle.comprar 	= 0;
	novaMsg->controle.passarVez	= 0;
	novaMsg->controle.fimDoJogo	= 1;	
}


/* FUNÇÃO QUE IMPRIME A MÃO DO JOGADOR */
void imprimeCartas (carta *deck, int nCarta){

	for (int i = 0; i < nCarta ; i++)
	{
		if (deck[i].cor == 0){
			if (deck[i].tipo < 10){
				printf("%c -> " ANSI_COLOR_RED "%d" ANSI_COLOR_RESET "\n", (97 + i),deck[i].tipo);
			} else if (deck[i].tipo == 10){
				printf("%c -> " ANSI_COLOR_RED "PULA" ANSI_COLOR_RESET "\n", ('a' + i));
			} else if (deck[i].tipo == 11){
				printf("%c -> " ANSI_COLOR_RED "+2" ANSI_COLOR_RESET "\n", ('a' + i));
			}
			
		}
		if (deck[i].cor == 1){
			if (deck[i].tipo < 10){
				printf("%c -> " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "\n", (97 + i),deck[i].tipo);
			} else if (deck[i].tipo == 10){
				printf("%c -> " ANSI_COLOR_GREEN "PULA" ANSI_COLOR_RESET "\n", ('a' + i));
			} else if (deck[i].tipo == 11){
				printf("%c -> " ANSI_COLOR_GREEN "+2" ANSI_COLOR_RESET "\n", ('a' + i));
			}
			
		}
		if (deck[i].cor == 2){
			if (deck[i].tipo < 10){
				printf("%c -> " ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\n", (97 + i),deck[i].tipo);
			} else if (deck[i].tipo == 10){
				printf("%c -> " ANSI_COLOR_YELLOW "PULA" ANSI_COLOR_RESET "\n", ('a' + i));
			} else if (deck[i].tipo == 11){
				printf("%c -> " ANSI_COLOR_YELLOW "+2" ANSI_COLOR_RESET "\n", ('a' + i));
			}
			
		}
		if (deck[i].cor == 3){
			if (deck[i].tipo < 10){
				printf("%c -> " ANSI_COLOR_BLUE "%d" ANSI_COLOR_RESET "\n", (97 + i),deck[i].tipo);
			} else if (deck[i].tipo == 10){
				printf("%c -> " ANSI_COLOR_BLUE "PULA" ANSI_COLOR_RESET "\n", ('a' + i));
			} else if (deck[i].tipo == 11){
				printf("%c -> " ANSI_COLOR_BLUE "+2" ANSI_COLOR_RESET "\n", ('a' + i));
			}
			
		}
	}
}

/* FUNÇÃO QUE IMPRIME A CARTA DO TOPO */
void imprimeTopo (carta topo){


	if (topo.cor == 0){
		if (topo.tipo < 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_RED "%d" ANSI_COLOR_RESET "\n",topo.tipo);
		} else if (topo.tipo == 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_RED "PULA" ANSI_COLOR_RESET "\n");
		} else if (topo.tipo == 11){
			printf("CARTA POR CIMA -> " ANSI_COLOR_RED "+2" ANSI_COLOR_RESET "\n");
		}
		
	}
	if (topo.cor == 1){
		if (topo.tipo < 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET "\n",topo.tipo);
		} else if (topo.tipo == 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_GREEN "PULA" ANSI_COLOR_RESET "\n");
		} else if (topo.tipo == 11){
			printf("CARTA POR CIMA -> " ANSI_COLOR_GREEN "+2" ANSI_COLOR_RESET "\n");
		}
		
	}
	if (topo.cor == 2){
		if (topo.tipo < 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\n",topo.tipo);
		} else if (topo.tipo == 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_YELLOW "PULA" ANSI_COLOR_RESET "\n");
		} else if (topo.tipo == 11){
			printf("CARTA POR CIMA -> " ANSI_COLOR_YELLOW "+2" ANSI_COLOR_RESET "\n");
		}
		
	}
	if (topo.cor == 3){
		if (topo.tipo < 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_BLUE "%d" ANSI_COLOR_RESET "\n",topo.tipo);
		} else if (topo.tipo == 10){
			printf("CARTA POR CIMA -> " ANSI_COLOR_BLUE "PULA" ANSI_COLOR_RESET "\n");
		} else if (topo.tipo == 11){
			printf("CARTA POR CIMA -> " ANSI_COLOR_BLUE "+2" ANSI_COLOR_RESET "\n");
		}
		
	}
}

/* ORGANIZA A MAÕ DO JOGADOR: COR (TIPO) */
void ajeitaDeck(carta *deck, int nCarta){
	carta aux;
	for (int i = 0; i < nCarta; ++i) {
		for (int j = i+1; j < nCarta; ++j) {
			if(deck[j].cor < deck[i].cor){
				aux = deck[i];
				deck[i] = deck[j];
				deck[j] = aux;
			}
		}
	}
	
	for (int i = 0; i < nCarta; ++i) {
		for (int j = i+1; j < nCarta; ++j) {
			if((deck[j].cor == deck[i].cor) && (deck[j].tipo < deck[i].tipo)){
				aux = deck[i];
				deck[i] = deck[j];
				deck[j] = aux;
			}
		}	
	}
}
/* FUNÇÃO QUE IMPRIME A MÃO E A ULTIMA CARTA JOGADA */
void imprimeJogo (carta topo, carta *deck, int nCarta){
	ajeitaDeck(deck,nCarta); 

	printf("\n\n");
	imprimeTopo(topo);
	if (nCarta > 1)	printf(ANSI_COLOR_GREY "VOCÊ TEM %d CARTAS: " ANSI_COLOR_RESET "\n", nCarta);
	else if (nCarta == 1) printf(ANSI_COLOR_GREY "VOCÊ TEM %d CARTAS: " ANSI_COLOR_RESET "\n", nCarta);
	imprimeCartas(deck,nCarta);
}

/* FUNÇÃO PARA QUANDO O DEALER TIVER QUE COMPRAR UMA CARTA */
carta DealerPegaCarta (baralho* b, baralho *monte) {
	if (b->n_cartas > 0) {
		b->n_cartas--;
		return b->monte[b->n_cartas+1];
	}
	else {
		re_embaralha (b, monte);
		b->n_cartas--;
		return b->monte[b->n_cartas+1];	
	}
}


/* MAIN */
int main (int argc, char** argv) {

	/*  
	 *	n = NÚMERO DESTE JOGADOR
	 *	dealer = NÚMERO DO DEALER
	 */
	int n, dealer = 0;

	/*
	:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	************************************************************************
					TEM QUE AVISAR QUEM É O DEALER
				E PERGUNTAR SE JÁ PODEMOS COMEÇAR O JOGO
	*************************************************************************
	:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	*/

	/* LÊ QUEM É QUEM E INICIA O JOGO */
	char c;
	while ((c=getopt (argc, argv, "j:d::"))!= -1){
		switch(c){
			case'j':
				n = atoi(optarg);
			break;
			case'd':
				dealer = 1;
			break;
		}
	}

	jogadores *joga;
	joga = malloc (sizeof(jogadores));

	FILE * arq;
	arq = fopen("config","r");
	
	/* ABRE O ARQUIVO COM O IP DOS JOGADORES E COLOCA NA ESTRUTURA */
	if (!arq){
		DeuRuimPorque ("Erro ao abrir o arquivo de configuração.");
	} else {
		fread (joga,sizeof(jogadores),1,arq);
	}

	joga->porta[5] = '\0';
	joga->ips[0].end[13] = '\0';
	joga->ips[1].end[13] = '\0';
	joga->ips[2].end[13] = '\0';
	joga->ips[3].end[13] = '\0';


	/* AVISA QUEM É VOCÊ E QUEM É O DEALER */
	printf(ANSI_COLOR_MAGENTA "Você é o jogador n %d." ANSI_COLOR_RESET "\n", n);
	if (dealer) printf("Você é o dealer.\n");

	// Mostra as configurações para que todos possam conferir
	printf("Os jogadores tem os seguintes IPs:\n\t0 = %s\n\t1 = %s\n\t2 = %s\n\t3 = %s\nA porta é %s.\n\n\n", 
	joga->ips[0].end, joga->ips[1].end, joga->ips[2].end, joga->ips[3].end, joga->porta);

	fclose(arq);
   	msg_t *mensagem = malloc (sizeof(msg_t));
   
    /* ABRE O SOCKET */
   	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
   		DeuRuimPorque ("Erro ao abrir o socket");
   	}
    
    /* DEFINE O ENDEREÇO DESTA MÁQUINA */
	eu.sin_family 	= AF_INET;
	eu.sin_port 	= htons(atoi(joga->porta)); 
  	eu.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* DEFINE O ENDEREÇO DA PRÓXIMA MÁQUINA */
   	prox.sin_family = AF_INET;
 	prox.sin_port = htons(atoi(joga->porta));
   	if (inet_aton(joga->ips[(n+1)%4].end, &prox.sin_addr) == 0) {	// Rotina de manipulação de endereços de internet
       	DeuRuimPorque("O inet_aton não converteu.");				// IPV4 => binário
   	}
    
    /* NOMEIA O SOCKET COM O ENDEREÇO DESTA MÁQUINA */
   	if( bind(s, (struct sockaddr*)&eu, sizeof(eu) ) == -1) {
       	perror ("bind");
       	exit (-1);
   	}

   	/* O DEALER COMEÇA, POR ISSO ELE QUE TEM O BASTÃO*/
	int temBastao = dealer;   	
	int ndealer = 0;
  
  	/* CRIA O BARALHO */ 	
   	baralho *b, *m;

   	/* SÓ ALOCA O BARALHO SE FOR O DEALER */
   	if (dealer) {
   		char prontos;
   		do {
   			printf("Todos os jogadores já se conectaram? [S/N]\n");
   			prontos = getchar();
   			int c;
			while ((c = getchar()) != '\n' && c != EOF) { }
   		} while (prontos != 'S');

   		b = inicUNO (b);
   		m = malloc(sizeof(baralho));
   		m->n_cartas = 0;

   		/* AVISA AOS OUTROS QUE ESTA MÁQUINA É O DEALER */
   		GritaUno(mensagem, n);
   		EnviarMensagem(mensagem);
   		RecebeMensagem(mensagem);
   	}
   	else {
   		/* QUEM NÃO É O DEALER FICA ESPERANDO RECEBER UMA MENSAGEM AVISANDO QUEM O É */
   		RecebeMensagem(mensagem);
   		ndealer = mensagem->controle.remetente;
   		EnviarMensagem(mensagem);	
   	}

   	/* CRIA A MÃO DO JOGADOR E A ULTIMA CARTA JOGADA */
   	carta minhasCartas[56];
   	carta topoMonte;
   	int nMinhasCartas = 0;

   	/* O DEALER PRECISA DAR AS CARTAS */
   	if (dealer) {
   		for (int i = 0; i < 4; i++) { // N JOGADORES
   			for (int j = 0; j < 7; j++) { // N DE CARTAS NA MÃO DE CADA UM
   				if (i != n) { // SE O DEALER FOR DAR CARTA PRA OUTRO JOGADOR
	   				DarCarta (b, mensagem, i, n);
	   				if (b->n_cartas <= 2) re_embaralha(b, m);
	   				EnviarMensagem (mensagem);
	   				RecebeMensagem (mensagem);
	   			} else { // SE FOR COMPRAR CARTAS PRA ELE MESMO
	   				minhasCartas[nMinhasCartas] = b->monte[b->n_cartas-1];
					b->n_cartas--;
					if (b->n_cartas <= 2) re_embaralha(b, m);
					nMinhasCartas++;
	   			}
   			}
   		}

   		/* COLOCA A CARTA DO TOPO PARA SER JOGADA */ 
   		m->monte[m->n_cartas] = b->monte[b->n_cartas-1];
   		m->n_cartas++;
		b->n_cartas--;
		topoMonte = m->monte[m->n_cartas-1];

		mensagem->controle.destino 	= n;
		mensagem->controle.remetente= n;
		mensagem->controle.bastao 	= 0;
		mensagem->controle.jogando 	= 1;
		mensagem->controle.uno 		= 0;
		mensagem->controle.comprar 	= 0;
		mensagem->controle.passarVez= 0;
		mensagem->controle.fimDoJogo= 0;

		mensagem->umaCarta = topoMonte;
		EnviarMensagem (mensagem); // MANDA PRA GALERA AVISANDO QUAL A PRIMEIRA CARTA

   	} else {

   		/* SE NÃO FOR O DEALER, SÓ ESPERE PRA RECEBER AS CARTAS */
   		for (int i = 0; i < 21; i++) {
   			RecebeMensagem (mensagem);
   			if (mensagem->controle.destino == n) { // SE A MENSAGEM FOR PRA ESTE JOGADOR
   				minhasCartas[nMinhasCartas].cor = mensagem->umaCarta.cor;
   				minhasCartas[nMinhasCartas].tipo = mensagem->umaCarta.tipo;
   				nMinhasCartas++;
   			}
   			// REPASSA
   			EnviarMensagem (mensagem);
   		}
   		RecebeMensagem (mensagem);

   		/* PEGA A CARTA DO TOPO */
   		if (mensagem->controle.jogando) {
   			topoMonte = mensagem->umaCarta;
   		}
   		// REPASSA
   		EnviarMensagem (mensagem);
   	}

   	/*
	:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	************************************************************************
			E SE O PRIMEIRO JOGADOR NÃO PUDER JOGAR NENHUMA CARTA?
	*************************************************************************
	:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	*/


   	// MOSTRA A MÃO INICIAL E A CARTA QUE VIROU
	imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);

	if (dealer) {
		// VERIFICA SE O PRIMEIRO JOGADOR TEM O QUE JOGAR
	   	int temOQjogar = 0;
		for (int x = 0; (x < nMinhasCartas) && (temOQjogar == 0); x++) {
			if((minhasCartas[x].tipo == topoMonte.tipo) || 
					(minhasCartas[x].cor == topoMonte.cor)){
				temOQjogar = 1;
			}
		}

		// CASO NÃO POSSA JOGAR NADA, COMPRA E PASSA A VEZ
		if (!temOQjogar) {
			printf(ANSI_COLOR_MAGENTA "ESSA NÃO! VOCÊ NÃO PODE JOGAR NENHUMA CARTA, E POR ISSO TERÁ QUE COMPRAR." ANSI_COLOR_RESET "\n");
			minhasCartas[nMinhasCartas] = DealerPegaCarta (b, m);
			nMinhasCartas++;
			imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);	
			PassaBastao(mensagem, (n+1)%4,n);
			temBastao = 0;
			EnviarMensagem(mensagem);
		}
	}

   	// CARTA QUE O JOGADOR ESCOLHE PRA JOGAR
	char escolha;

	// AUXILIARES DE CONTROLE
	int cartaCerta = 0,tenhoQComprar = 0,fuiPulado = 0, cartaPraJogar;

   	/***************** JOGO *******************/
	while (1) {
		if (temBastao) {

			imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
			// AVISA O JOGADOR QUE ELE VAI JOGAR
			printf("\nSua vez! Entre com o caractere correspondente a carta que deseja jogar: \n");

			// PEGA A LETRA (ESCOLHA) DELE E O '\n' 
			fscanf (stdin, "%c", &escolha); 
			int c;
			while ((c = getchar()) != '\n' && c != EOF) { }//getchar();

			/* SE FOR JOGAR PRA GRITAR UNO (SE ELE PODE OU NÃO JOGAR É VERIFICADO ANTERIORMENTE)
			   SE ESQUECER DO UNO OU GRITAR FORA DE HORA  */
			if (((nMinhasCartas == 2) && (escolha != 'u')) || ((nMinhasCartas != 2) && (escolha == 'u'))) {
				printf("OH NÃO, VOCÊ SE ESQUECEU DE COMO O \"UNO!\" FUNCIONA!\n");
				printf("A PUNIÇÃO É COMPRAR DUAS CARTAS.\n");

				// TEM QUE COMPRAR CARTA, E CASO ELE SEJA O DEALER
				if (dealer) {
					minhasCartas[nMinhasCartas] = DealerPegaCarta (b, m);
					nMinhasCartas++;
					minhasCartas[nMinhasCartas] = DealerPegaCarta (b, m);
					nMinhasCartas++;
				}
				else {
					// SE NÃO FOR O DEALER, PEDE CARTA
					for (int i = 0; i < 2; ++i) {
						PedeCarta(mensagem,ndealer,n);
						EnviarMensagem(mensagem);
						RecebeMensagem(mensagem);
						minhasCartas[nMinhasCartas] = mensagem->umaCarta;
						nMinhasCartas++;
					}
				}

				// IMPRIME A MÃO E PASSA O BASTÃO
				//imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
				/*temBastao = 0;
				PassaBastao(mensagem, (n+1)%4,n);
				EnviarMensagem(mensagem);*/
			}
				/* CASO ESTEJA JOGANDO TRANQUILAMENTE E TALVEZ TENHA DITO UNO (CORRETAMENTE) */
				
				if (nMinhasCartas == 2 && escolha == 'u') {
					GritaUno (mensagem, n);
					EnviarMensagem (mensagem);
					RecebeMensagem (mensagem);
					
					printf(ANSI_COLOR_MAGENTA "UNO!" ANSI_COLOR_RESET "\n");

					printf("Agora jogue sua carta!: \n");
					fscanf (stdin, "%c", &escolha); //getchar();
					int c;
					while ((c = getchar()) != '\n' && c != EOF) { }
				}

				/* ENQUANTO NÃO ESCOLHER UMA OPÇÃO VÁLIDA PARA JOGAR */
		 		while (!cartaCerta){
		 			
		 			// CARACTER INVÁLIDO
		 			while (escolha < 'a' || escolha > 'a' + nMinhasCartas) {
		 				imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
		 				printf("Você não tem essa carta. Escolha a letra minuscula dentre as cartas existentes na sua mão.\n");
		 				fscanf (stdin, "%c", &escolha);//getchar();
		 				int c;
						while ((c = getchar()) != '\n' && c != EOF) { }
		 			} 
		 			cartaPraJogar = escolha - 'a';

		 			// CARTA NÃO COMPATÍVEL
		 			if((minhasCartas[cartaPraJogar].tipo != topoMonte.tipo) && 
		 				(minhasCartas[cartaPraJogar].cor != topoMonte.cor)){
		 				imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
		 				printf("Carta selecionada não pode ser jogada, selecione uma carta certa.\n");
		 				fscanf (stdin, "%c", &escolha); //getchar();
		 				int c;
						while ((c = getchar()) != '\n' && c != EOF) { }
		 			}
		 			else{
		 				cartaCerta = 1;
		 			}
		 			
		 		}
		 		cartaCerta = 0;
		 		printf("\n\n");

		 		// JOGAA A CARTA ESCOLHIDA
		 		Jogar(minhasCartas[cartaPraJogar], mensagem, nMinhasCartas-1, n);
		 		EnviarMensagem (mensagem);

		 		RecebeMensagem (mensagem);
		 		topoMonte = mensagem->umaCarta;
				
				if (nMinhasCartas == 1) {
					Ganhar (mensagem, n);
					EnviarMensagem (mensagem);
					RecebeMensagem (mensagem);
					printf(ANSI_COLOR_MAGENTA "O JOGADOR <%d> GANHOU O JOGO!" ANSI_COLOR_RESET "\n", mensagem->controle.remetente);
 					sleep(5);
 					return(0);  			
				}
		 		///-------------------------------------------------------------------------------------------

		 		minhasCartas[cartaPraJogar] = minhasCartas[nMinhasCartas-1];
		 		nMinhasCartas--;

		 		
		 		
		 		if (dealer) {
		 			m->monte[m->n_cartas] = topoMonte;
	   				m->n_cartas++;
		 		}

		 		PassaBastao (mensagem, (n+1)%4, n);
		 		EnviarMensagem(mensagem);

		 		imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
		    	temBastao = 0;

		    	printf("//////////////////////////////////////////////////////////////\n");
	    	
		} 
		else {
			RecebeMensagem (mensagem);

			if (mensagem->controle.destino == n) { // Mensagem para este jogador

				if ((mensagem->controle.jogando == 0) && (mensagem->controle.comprar > 0)) {
					printf(ANSI_COLOR_GREY "O JOGADOR <%d> COMPROU UMA CARTA." ANSI_COLOR_RESET "\n", mensagem->controle.remetente);
   				}

	   			//*********************** TAREFAS EXCLUSIVAS AO DEALER ***********************
	   			if (dealer) {
	   				if (mensagem->controle.comprar > 0) {
	   					// VERIFICA SE AINDA TEM CARTA NO DECK
	   					// SE NÃO TIVER, EMBARALHA
	   					DarCarta (b, mensagem, mensagem->controle.remetente, n); // AQUI MUDOU
	   					if (b->n_cartas <= 2) re_embaralha(b, m);
	   					EnviarMensagem (mensagem);
	   				}
	   				// ...
	   			}

	   			//*********************** TAREFAS DE UM JOGADOR  ***********************
	   			if (mensagem->controle.bastao > 0) {
	   				printf("//////////////////////////////////////////////////////////////\n");

	   				if(fuiPulado){
	   					printf(ANSI_COLOR_MAGENTA "VOCÊ RECEBEU UM BLOQUEIO E NÃO JOGARÁ DESTA VEZ." ANSI_COLOR_RESET "\n");
	   					PassaBastao(mensagem, (n+1)%4, n);
	   					fuiPulado = 0;
	   					EnviarMensagem(mensagem);
	   					printf("//////////////////////////////////////////////////////////////\n");
	   				}
	   				else if(tenhoQComprar > 0){ // TRATAR O CASO DE ELE SER O DEALER
	   					printf(ANSI_COLOR_MAGENTA "VOCÊ RECEBEU UM +2 E IRÁ COMPRAR AS CARTAS." ANSI_COLOR_RESET "\n");
	   					if (dealer) {
							for (int i = 0; i < tenhoQComprar; ++i) {
								minhasCartas[nMinhasCartas] = DealerPegaCarta (b, m);
								nMinhasCartas++;
							}
							imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
						}
						else {
		   					for (int i = 0; i < tenhoQComprar; ++i) {
		   						PedeCarta(mensagem,ndealer,n);
		   						EnviarMensagem(mensagem);
		   						RecebeMensagem(mensagem);
		   						minhasCartas[nMinhasCartas] = mensagem->umaCarta;
		   						nMinhasCartas++;
		   					}
							imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
		   				}

	   					tenhoQComprar = 0;

	   					PassaBastao(mensagem, (n+1)%4,n);
	   					EnviarMensagem(mensagem);
	   					printf("//////////////////////////////////////////////////////////////\n");
	   				}
	   				else {
	   					int temEscolha = 0;
						for (int x = 0; (x < nMinhasCartas) && (temEscolha == 0); x++) {
							if((minhasCartas[x].tipo == topoMonte.tipo) || 
		 						(minhasCartas[x].cor == topoMonte.cor)){
		 						temEscolha = 1;
		 					}
						}
						if (!temEscolha) {
							printf(ANSI_COLOR_MAGENTA "ESSA NÃO! VOCÊ NÃO PODE JOGAR NENHUMA CARTA, E POR ISSO TERÁ QUE COMPRAR." ANSI_COLOR_RESET "\n");
							if (dealer) {
								minhasCartas[nMinhasCartas] = DealerPegaCarta (b, m);
								nMinhasCartas++;
							}
							else {
		   						PedeCarta(mensagem,ndealer,n);
		   						EnviarMensagem(mensagem);
		   						RecebeMensagem(mensagem);
		   						minhasCartas[nMinhasCartas] = mensagem->umaCarta;
		   						nMinhasCartas++;
		 					}
		 					imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);	
		   					PassaBastao(mensagem, (n+1)%4,n);
		   					EnviarMensagem(mensagem);
		   					printf("//////////////////////////////////////////////////////////////\n");
						}
						else {
	   						temBastao = 1;
	   					}
	   				}
	   			}

	   		}
	   		else if (mensagem->controle.destino == mensagem->controle.remetente){

   				if (mensagem->controle.jogando > 0){ //JOGADOR.destino JOGOU UMA CARTA
   					topoMonte = mensagem->umaCarta;

   					printf(ANSI_COLOR_GREY "O JOGADOR <%d> AINDA TEM %d CARTAS NA MÃO." ANSI_COLOR_RESET "\n", mensagem->controle.remetente,
   																											mensagem->controle.nCartasNaMao);

   					//imprimeJogo(topoMonte,minhasCartas,nMinhasCartas);
   					imprimeTopo(topoMonte);
   					if (dealer) {
   						m->monte[m->n_cartas] = topoMonte;
   						m->n_cartas++;
   					}
   					if ((mensagem->controle.remetente + 1) % 4 == n){
   						if(topoMonte.tipo == CBLOCK){
   							fuiPulado = 1;
   						}
   						if(topoMonte.tipo == CC2){
   							tenhoQComprar = 2;
   						}
   					}
   					EnviarMensagem(mensagem);
   				}
   	
   			} 
   			else {
   				if ((mensagem->controle.jogando == 0) && (mensagem->controle.comprar > 0)) {
					printf(ANSI_COLOR_GREY "O JOGADOR <%d> COMPROU UMA CARTA." ANSI_COLOR_RESET "\n", mensagem->controle.remetente);
   				}
   				if ((mensagem->controle.jogando == 0) && (mensagem->controle.comprar < 0)) {
					printf(ANSI_COLOR_GREY "O JOGADOR <%d> COMPROU UMA CARTA." ANSI_COLOR_RESET "\n", mensagem->controle.destino);
   				}
   				else if ((mensagem->controle.passarVez > 0)) {
					printf(ANSI_COLOR_GREY "O JOGADOR <%d> FOI PULADO!" ANSI_COLOR_RESET "\n", mensagem->controle.destino);
   				}
   				EnviarMensagem(mensagem);
   			}



   			if (mensagem->controle.fimDoJogo > 0) {
   				EnviarMensagem (mensagem);
   				printf(ANSI_COLOR_MAGENTA "O JOGADOR <%d> GANHOU O JOGO!" ANSI_COLOR_RESET "\n", mensagem->controle.remetente);
 				sleep(5);
 				return(0);  		
   				//ExibirTelaDerrota (mensagem->controle.remetente);
   				// REPASSA A MENSAGEM
   				// FINALIZA O PROGRAMA (fechar o socket, liberar os mallocs, etc)
   			}

   			if (mensagem->controle.uno > 0) {
   				//AVISA UNO BONITINHO
   				
   				printf(ANSI_COLOR_MAGENTA "O JOGADOR <%d> GRITOU UNO!" ANSI_COLOR_RESET "\n", mensagem->controle.remetente);
   				EnviarMensagem (mensagem);
   			}





		}
	}
}
