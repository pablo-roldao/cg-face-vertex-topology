# Estrutura de dados topológica Face-Vértice

Este projeto é uma implementação em **C** de uma estrutura de dados topológica do tipo **Face-Vértice** para processamento de malhas poligonais (triangulares, quadriláteras ou mistas). A aplicação foi desenvolvida para a disciplina de Computação Gráfica ministrada pelo professor Icaro Cunha.

O programa lê os vértices e faces de arquivos no formato genérico `.off` e permite que sejam instanciados na memória. Como extra, ele gera dinamicamente:
1. Um relatório via terminal constando as informações e topologias conectadas da malha.
2. A renderização/desenho das formas 2D diretamente em arquivos vetoriais (`.svg`), sem depender de bibliotecas gráficas pesadas ou externas para o parser C.

## A estrutura Face-Vértice
No modelo **Face-Vértice**, trabalhamos o conceito de duas listas (tabelas) independentes que se relacionam através dos índices:
* **Tabela de vértices (`Vertex`)**: Cada vértice armazena sua posição no espaço (coordenadas `x, y, z`) juntamente com uma lista dinâmica que diz *quais faces o usam*.
* **Tabela de faces (`Face`)**: Cada face guarda uma lista para referenciar *quais vértices a constroem* (permitindo triângulos ou polígonos de `N` lados).

## Como compilar e executar em C

1. Faça a compilação do código com `gcc`:
```bash
gcc face_vertex.c -o face_vertex
```

2. Execute passando seus modelos de teste em formato `.off` via argumento de linha de comando:
```bash
./face_vertex triangles.off hand-hybrid.off
```

O programa criará arquivos vetoriais como `triangles.svg` e `hand-hybrid.svg` baseados nas dimensões da sua malha.
