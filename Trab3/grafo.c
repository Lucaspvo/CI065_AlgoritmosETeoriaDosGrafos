#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>
#include <string.h>
#include "lista.h"
#include "grafo.h"

typedef struct aresta *aresta;
typedef struct componente *componente;
typedef struct vertice *vertice;


void salva_aresta(Agedge_t *a, struct vertice *v,struct grafo *g); 	//salva aresta nas listas
struct aresta* cria_aresta(struct grafo *g,struct vertice *ori,struct vertice *dest, long int peso);
int destroi_aresta(void* a);	
int compara (const void* a, const void* b);			
unsigned int find (struct componente *componentes, unsigned int i);	
void Union (struct componente *componentes, unsigned int x, unsigned int y);
void salva_aresta_agm(struct vertice *ori, struct vertice *dest, long int peso,struct grafo *agm);	
void assignment(vertice u, vertice root);					
void depth_first_search(vertice v, lista l);		

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

struct vertice {
	char *nome;
	struct lista *vizinhos_ent;
	struct lista *vizinhos_sai;
	int comp;                   	//Componente ao qual pertence
	int visitado;					//Checar se o vertice foi visitado
	unsigned int id;
};

struct aresta {
	struct vertice *ori;
	struct vertice *dest;
	long int peso;
};

struct componente {
	unsigned int pai;
	int rank;
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
	return g->n_arc; 
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// devolve o nome do vertice v

char *nome_vertice(vertice v) {
	return v->nome;
}


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
	while (strcmp(agnameof(aux),g->p_vert[i].nome)){
		i++;
	}
	if (strcmp(g->p_vert[i].nome, v->nome)) {
		nox = insere_lista(cria_aresta(g,v,&g->p_vert[i],peso),v->vizinhos_sai);
		if (!nox)
			exit(-1);
		if (direcionado(g)) {
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

    g->nome = (char*) malloc(sizeof(char) * strlen(agnameof(g_bruto)));					//maloca espaço para o nome no grafo
    strcpy (g->nome, agnameof(g_bruto));

  	g->tipo =  agisdirected(g_bruto);											
  	g->n_ver = (unsigned int) agnnodes(g_bruto);
  	g->n_arc = (unsigned int) agnedges(g_bruto);

  	g->p_vert = (struct vertice*) malloc(sizeof(struct vertice)*(g->n_ver)); 	//aloca vetor de vertices
  	i = 0;


  	for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {		
			g->p_vert[i].nome = malloc(sizeof(char) * (strlen(agnameof(v))+1));
			strcpy (g->p_vert[i].nome,agnameof(v));
			g->p_vert[i].vizinhos_ent = constroi_lista();
			g->p_vert[i].vizinhos_sai = constroi_lista();
			
			i++;

	}

	i = 0;
  	if (direcionado(g)) {
		
  		for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {	
	  		for (Agedge_t *a=agfstout(g_bruto,v); a; a=agnxtout(g_bruto,a)){ 
	  			salva_aresta(a,&g->p_vert[i],g);
	  		}
	  		i++;
	   	}
	}
	else {
		for (Agnode_t *v=agfstnode(g_bruto); v; v=agnxtnode(g_bruto,v)) {//pega todos os vertices e guarda na lista de vertices
	  		for (Agedge_t *a=agfstedge(g_bruto,v); a; a=agnxtedge(g_bruto,a,v)){ //verifica todos os arcos do v
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
	for (unsigned int i = 0; i < g2->n_ver; i++){
		if(g2->p_vert[i].vizinhos_sai)
			destroi_lista(g2->p_vert[i].vizinhos_sai, destroi_aresta);
		if((g2->tipo == 1) && (g2->p_vert[i].vizinhos_ent))
			destroi_lista(g2->p_vert[i].vizinhos_ent, destroi_aresta);
		free(g2->p_vert[i].nome);
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

	for ( i = 0; i < g->n_ver;i++) {
		 fprintf(output, "    \"%s\"\n", g->p_vert[i].nome);
	}
	fprintf(output, "\n");

	char rep_aresta = direcionado(g) ? '>' : '-';


	for (i = 0; i < g->n_ver;i++) {
		no aux = primeiro_no(g->p_vert[i].vizinhos_sai);
		while (aux) {
			k = conteudo(aux);
			fprintf(output, "    \"%s\" -%c \"%s\" ",	g->p_vert[i].nome, rep_aresta, k->dest->nome);
			if (g->pond) {
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

unsigned int find (struct componente *componentes, unsigned int i) {
	if (componentes[i].pai != i)
		componentes[i].pai = find(componentes,componentes[i].pai);
	return componentes[i].pai;
}

void Union (struct componente *componentes, unsigned int x, unsigned int y) {
	unsigned int xraiz = find(componentes,x);
	unsigned int yraiz = find(componentes,y);

	if (componentes[xraiz].rank < componentes[yraiz].rank)
        componentes[xraiz].pai = yraiz;
    else if (componentes[xraiz].rank > componentes[yraiz].rank)
        componentes[yraiz].pai = xraiz;
 
    
    else {
        componentes[yraiz].pai = xraiz;
        componentes[xraiz].rank++;
    }
} 

int compara (const void* a, const void* b) {
	struct aresta* const *a2 = a;
	struct aresta* a1 = *a2;
	struct aresta* const *b2 = b;
	struct aresta* b1 = *b2;
	if (a1->peso < b1->peso){
		return -1;
	}
	else if (a1->peso > b1->peso){
		return 1;  
	}
	else 
		return 0;
}

void salva_aresta_agm(struct vertice *ori, struct vertice *dest, long int peso,struct grafo *agm) {
	int i = 0;
	no nox;

	while (strcmp(ori->nome,agm->p_vert[i].nome)){
		i++;
	}
	nox = insere_lista(cria_aresta(agm,ori,dest,peso),agm->p_vert[i].vizinhos_sai);
	if (!nox)
		exit(-1);
	if (direcionado(agm)) {
		nox = insere_lista(cria_aresta(agm,ori,dest,peso),agm->p_vert[i].vizinhos_ent);
		if (!nox)
			exit(-1);
	}

}

grafo arvore_geradora_minima(grafo g) {



	int l = 0,w = 0;
	unsigned int i,k,d,o;
	struct aresta *q;
	struct componente *componentes = (struct componente*) malloc( g->n_arc * sizeof(struct componente));
	struct aresta vaux[g->n_ver];
	struct aresta *v_ar = (struct aresta*) malloc( g->n_arc * sizeof(struct aresta));
	struct grafo* agm = malloc(sizeof(struct grafo));
	agm->nome = (char*) malloc(sizeof(char) * (strlen(g->nome)+1));					//maloca espaço para o nome no grafo
	strcpy(agm->nome,"agm-");
    strcat (agm->nome, g->nome);
    agm->n_ver = g->n_ver;
    agm->tipo = 0;
    agm->pond = 1;
    agm->p_vert = (struct vertice*) malloc(sizeof(struct vertice)*(g->n_ver));

	   
    for (i = 0; i < g->n_ver;i++) {
                no aux = primeiro_no(g->p_vert[i].vizinhos_sai);
                while (aux) {
                        q = conteudo(aux);
                       /* v_ar[l].ori = q->ori;
                        v_ar[l].dest = q->dest;
                        v_ar[l].peso = q->peso;*/
                        v_ar[l] = *q;
                        l++;
                        aux = proximo_no(aux);
                }
      }

    for (i = 0; i < g->n_ver;i++) {
    	agm->p_vert[i].nome = malloc(sizeof(char) * (strlen(g->p_vert[i].nome)+1));
		strcpy (agm->p_vert[i].nome,g->p_vert[i].nome);
		agm->p_vert[i].vizinhos_ent = constroi_lista();
		agm->p_vert[i].vizinhos_sai = constroi_lista();
    }

	qsort(v_ar, l, sizeof(struct aresta), compara);
	for (i = 0; i < g->n_arc; ++i) {
		componentes[i].pai = i;
		componentes[i].rank = 0;
	}
	while ((k < g->n_arc) && (w < l)) {
		struct aresta *prox_aresta = &v_ar[w++];
		for (i = 0; i < agm->n_ver; i++) {
			if  (!(strcmp(prox_aresta->ori->nome,agm->p_vert[i].nome)))
				o = i;
			if  (!(strcmp(prox_aresta->dest->nome,agm->p_vert[i].nome))) 
				d = i;
		}
        unsigned int x = find(componentes, o);
        unsigned int y = find(componentes, d);
	o = 0;
	d = 0;
        if (x != y) {
            vaux[k++] = *prox_aresta;
            Union(componentes, x, y);
        }
	}

	for (i = 0; i < k; i++) {
		salva_aresta_agm(vaux[i].ori,vaux[i].dest,vaux[i].peso,agm);
	}

	agm->n_arc = k;
	return agm;

}

//********************************************************************************************************************//

void depth_first_search(vertice v, lista l){

	if(v->visitado)
		return;

	v->visitado = 1;

	for(no n = primeiro_no(v->vizinhos_sai); n; n = proximo_no(n)){
		struct aresta* a = conteudo(n);
		depth_first_search(a->dest, l);
	}
	insere_lista(v, l);
}

void assignment(vertice u, vertice root){
	if(u->comp == -1){
		if(u == root)
			u->comp = (int) u->id;
		else
			u->comp = root->comp;
		for(no n = primeiro_no(u->vizinhos_ent); n; n = proximo_no(n)){
			struct aresta* a = conteudo(n);
			assignment(a->ori, root);
		}
	}
}

lista componentes_fortemente_conexos(grafo g){
	vertice vert_ptr = g->p_vert;

	lista lista_vertices = constroi_lista();

	for(unsigned int i = 0; i < g->n_ver; i++){
		vert_ptr[i].visitado = 0;
		vert_ptr[i].comp = -1;
		vert_ptr[i].id = i;
	}

	for(unsigned int i = 0; i < g->n_ver; i++){
		depth_first_search(&vert_ptr[i], lista_vertices);
	}

	for(no n = primeiro_no(lista_vertices); n; n = proximo_no(n)){
		vertice v = conteudo(n);
		assignment(v, v);
	}

	destroi_lista(lista_vertices, NULL);

	lista list = constroi_lista();
	lista* vetor_componentes = malloc(g->n_ver * sizeof(lista));

	for(unsigned int i = 0; i < g->n_ver; i++){
		vetor_componentes[i] = constroi_lista();
	}

	for(unsigned int i = 0; i < g->n_ver; i++){
		insere_lista(&vert_ptr[i], vetor_componentes[vert_ptr[i].comp]);
	}

	for(unsigned int i = 0; i < g->n_ver; i++){
		if(tamanho_lista(vetor_componentes[i]))
			insere_lista(vetor_componentes[i], list);
		else
			destroi_lista(vetor_componentes[i], NULL);
	}

	free(vetor_componentes);

	return list;
}