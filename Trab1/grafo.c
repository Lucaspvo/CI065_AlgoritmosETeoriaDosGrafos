#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include <string.h>
#include "lista.h"
#include "grafo.h"

typedef struct aresta *aresta;
void salva_aresta(Agedge_t *a, struct vertice *v,struct grafo *g); 	//salva aresta nas listas
struct aresta* cria_aresta(struct grafo *g,struct vertice *ori,struct vertice *dest, long int peso);
int destroi_aresta(void* a);													

//------------------------------------------------------------------------------
// (apontador para) estrutura de dados para representar um grafo
// 
// o grafo pode ser
// - direcionado ou não
// - com pesos nas arestas ou não
// 
// além dos vértices e arestas, o grafo tem um nome, que é uma "string"
// 
// num grafo com pesos nas arestas todas as arestas tem peso, que é um long int
// 
// o peso default de uma aresta é 0

struct grafo {	
	unsigned int n_arc;			 	//numero de arcos			
	unsigned int n_ver;				//numero de vertices do grafo 
	int tipo;						//tipo do grafo (0 se eh direcionado 1 c.c.)
	int pond;						//1 se ponderado 0 c.c
	char *nome;						//ponteiro p/ nome do grafo
	struct vertice *p_vert;			//ptr p/ primeiro vertice
};


// (apontador para) estrutura de dados que representa um vértice do grafo
// 
// cada vértice tem um nome que é uma "string"

typedef struct vertice {
	char *nome;
	struct lista *vizinhos_ent;
	struct lista *vizinhos_sai;
};

typedef struct aresta {
	struct vertice *ori;
	struct vertice *dest;
	long int peso;
};

//------------------------------------------------------------------------------
// devolve o nome do grafo g

char *nome_grafo(grafo g) {
	return g->nome;
}

//------------------------------------------------------------------------------
// devolve 1, se g é direcionado, ou
//         0, caso contrário

int direcionado(grafo g) {
	return g->tipo;
}

//------------------------------------------------------------------------------
// devolve 1, se g tem pesos nas arestas/arcos,
//      ou 0, caso contrário

int ponderado(grafo g) {
	return g->pond;
}

//------------------------------------------------------------------------------
// devolve o número de vértices do grafo g

unsigned int n_vertices(grafo g){
	return g->n_ver;
}

//------------------------------------------------------------------------------
// devolve o número de arestas/arcos do grafo g

unsigned int n_arestas(grafo g){
	return g->n_arc; //global
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// devolve o nome do vertice v

char *nome_vertice(vertice v) {
	return v->nome;
}

//apenas malloca memoria para uma aresta e salva a origem e destino dos vertice da aresta
struct aresta* cria_aresta(struct grafo *g,struct vertice *ori,struct vertice *dest, long int peso) {
	struct aresta *ares = malloc(sizeof(struct aresta));
	ares->ori = ori;
	ares->dest = dest;

	if (peso > 0) {
		ares->peso = peso;
		g->pond = 1;
	} 
	return ares;
}

void salva_aresta(Agedge_t *a, struct vertice *v, struct grafo *g) {
	int i = 0;
	char *peso_s;
	long int peso = 0;
	no nox;
	Agnode_t *aux = aghead(a);
	peso_s = agget(a, (char *)"peso");
	if (peso_s)
		peso = atoll(peso_s);
	while (strcmp(agnameof(aux),g->p_vert[i].nome)){												//procura a cabeça do arco ou aresta na nossa estrutura
		i++;
	}
	if (strcmp(g->p_vert[i].nome, v->nome)) {
		nox = insere_lista(cria_aresta(g,v,&g->p_vert[i],peso),v->vizinhos_sai);					//salva arestas ou arcos de saida
		if (!nox)
			exit(-1);
		if (direcionado(g)) {																		//caso seja direcionado salva os arcos de entrada
			nox = insere_lista(cria_aresta(g,v,&g->p_vert[i],peso),g->p_vert[i].vizinhos_ent);
			if (!nox)
				exit(-1);
		}
	}

}

//------------------------------------------------------------------------------
// lê um grafo no formato dot de input, usando as rotinas de libcgraph
// 
// desconsidera todos os atributos do grafo lido exceto o atributo
// "peso" quando ocorrer; neste caso o valor do atributo é o peso da
// aresta/arco que é um long int
// 
// num grafo com pesos todas as arestas/arcos tem peso
// 
// o peso default de uma aresta num grafo com pesos é 0
// 
// todas as estruturas de dados alocadas pela libcgraph são
// desalocadas ao final da execução
// 
// devolve o grafo lido ou
//         NULL em caso de erro 

grafo le_grafo(FILE *input) {
	int i;

	struct grafo* g = malloc(sizeof(struct grafo));

	if(!g)
		return NULL;


	Agraph_t *g_bruto = agread(input, NULL); 								// le o grafo

	if ( !g_bruto ) 														// testa se o grafo foi lido corretamente
    	return NULL;

    g->nome = (char*) malloc(sizeof(char) * strlen(agnameof(g_bruto)));				//maloca espaço para o nome no grafo
    strcpy (g->nome, agnameof(g_bruto));

  	g->tipo =  agisdirected(g_bruto);											
  	g->n_ver = (unsigned int) agnnodes(g_bruto);
  	g->n_arc = (unsigned int) agnedges(g_bruto);

  	g->p_vert = (struct vertice*) malloc(sizeof(struct vertice)*(g->n_ver)); 		//aloca vetor de vertices
  	i = 0;


  	for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {				//constroi um vetor com todas os vertices do arquivo input
			g->p_vert[i].nome = malloc(sizeof(char) * strlen(agnameof(v)));
			g->p_vert[i].vizinhos_ent = constroi_lista();
			g->p_vert[i].vizinhos_sai = constroi_lista();
			strcpy (g->p_vert[i].nome,agnameof(v));
			i++;

	}

	i = 0;
  	if (direcionado(g)) {															//se for direcionado adiciona arcos
		
  		for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {	
	  		for (Agedge_t *a=agfstout(g_bruto,v); a; a=agnxtout(g_bruto,a)){ 
	  			salva_aresta(a,&g->p_vert[i],g);
	  		}
	  		i++;
	   	}
	}
	else {																			//se nao for direcionado adiciona arestas
		for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {			//pega todos os vertices e guarda na lista de vertices
	  		for (Agedge_t *a=agfstedge(g_bruto,v); a; a=agnxtedge(g_bruto,a,v)){ 	//verifica todos os arcos do v
	  			salva_aresta(a,&g->p_vert[i],g); 
	  		}
	  		i++;
	   	}
	}
	agclose(g_bruto);
	return g;	 
}


int destroi_aresta(void* a) {
	free(a);
	return 1;
} 

//------------------------------------------------------------------------------
// desaloca toda a memória usada em *g
// 
// devolve 1 em caso de sucesso ou
//         0 caso contrário
// 
// g é um (void *) para que destroi_grafo() possa ser usada como argumento de
// destroi_lista()

int destroi_grafo(void *g){
	struct grafo *g2 = g;
	
	free(g2->nome);
	for (unsigned int i = 0; i < g2->n_ver; i++){										//pega cada um dos vertices e da um free na lista de arestas antes de dar free no vertice
		if(g2->p_vert[i].vizinhos_sai)
			destroi_lista(g2->p_vert[i].vizinhos_sai, destroi_aresta);
/*		if((g2->tipo == 1) && (g2->p_vert[i].vizinhos_ent))
			destroi_lista(g2->p_vert[i].vizinhos_ent, destroi_aresta);*/
//		free(g2->p_vert[i].nome);
	}
	free(g2->p_vert);
	free(g2);

	return 1;
}

//------------------------------------------------------------------------------
// escreve o grafo g em output usando o formato dot, de forma que
// 
// 1. todos os vértices são escri tos antes de todas as arestas/arcos 
// 
// 2. se uma aresta tem peso, este deve ser escrito como um atributo 
//    de nome "peso"
//
// devolve o grafo escrito ou
//         NULL em caso de erro 

grafo escreve_grafo(FILE *output, grafo g) {
	struct aresta *k;
	unsigned int i;

 	fprintf(output, "strict %sgraph \"%s\" {\n\n", direcionado(g) ? "di" : "",nome_grafo(g));

	for ( i = 0; i < g->n_ver;i++) {																			//imprimi todos os vertices primeiro
		 fprintf(output, "    \"%s\"\n", g->p_vert[i].nome);
	}
	fprintf(output, "\n");

	char rep_aresta = direcionado(g) ? '>' : '-';


	for (i = 0; i < g->n_ver;i++) {																				//pega aresta por aresta e imprimi seus arcos/arestas
		no aux = primeiro_no(g->p_vert[i].vizinhos_sai);
		while (aux) {
			k = conteudo(aux);
			fprintf(output, "    \"%s\" -%c \"%s\" ",	g->p_vert[i].nome, rep_aresta, k->dest->nome);
			if (k->peso > 0) {
				fprintf(output, " [peso=%ld]",k->peso);

			}
			fprintf(output, "\n");
			aux = proximo_no(aux);
		}
	}
	fprintf(output, "}\n");
	return g;
}

//------------------------------------------------------------------------------
// devolve a vizinhança do vértice v

lista vizinhanca(vertice v) {
	return v->vizinhos_sai;
}
//------------------------------------------------------------------------------
// devolve a vizinhança de entrada do vértice v

lista vizinhanca_entrada(vertice v) {
	return v->vizinhos_ent;
}

//------------------------------------------------------------------------------
// devolve a vizinhança de saída do vértice v

lista vizinhanca_saida(vertice v) {
	return v->vizinhos_sai;
}

//------------------------------------------------------------------------------
// devolve o grau do vértice v

unsigned int grau(vertice v) {
	return grau_entrada(v) + grau_saida(v);
}

//------------------------------------------------------------------------------
// devolve o grau de entrada do vértice v

unsigned int grau_entrada(vertice v) {
	return tamanho_lista(v->vizinhos_ent);

}

//------------------------------------------------------------------------------
// devolve o grau se saída do vértice v

unsigned int grau_saida(vertice v) {
	return tamanho_lista(v->vizinhos_sai);
}
