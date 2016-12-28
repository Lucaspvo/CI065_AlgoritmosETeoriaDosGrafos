#include <stdio.h>
#include "grafo.h"

//------------------------------------------------------------------------------

int main(void) {

  grafo g = le_grafo(stdin),
    agm = arvore_geradora_minima(g);

  escreve_grafo(stdout, agm);  

  return ! (destroi_grafo(agm) && destroi_grafo(g));
}
