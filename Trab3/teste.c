#include <stdio.h>
#include "grafo.h"

//------------------------------------------------------------------------------
static void escreve_lista_vertices(lista l) {

  for(no n=primeiro_no(l); n; n=proximo_no(n))
  
    printf("%s ", nome_vertice(conteudo(n)));

  printf("\n");
}
//------------------------------------------------------------------------------

int main(void) {

  grafo g = le_grafo(stdin);
  lista componentes = componentes_fortemente_conexos(g);

  for(no n=primeiro_no(componentes); n; n=proximo_no(n)) {

    lista l = conteudo(n);
    escreve_lista_vertices(l);
    destroi_lista(l,NULL);
    *((lista *)conteudo(n)) = NULL;
  }

  return ! (destroi_lista(componentes,NULL) && destroi_grafo(g));
}
