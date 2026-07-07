#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura para representar uma face
typedef struct {
    int *vertices;       // Lista de índices dos vértices que compõem a face
    int num_vertices;    // Número de vértices da face (3 para triângulos, 4 para quadriláteros, etc)
} Face;

// Estrutura para representar um vértice
typedef struct {
    double x, y, z;      // Coordenadas espaciais do vértice
    int *faces;          // Lista de índices das faces a que o vértice pertence
    int num_faces;       // Número de faces que compartilham este vértice
    int capacity_faces;  // Capacidade atual do array de faces (para realocação dinâmica)
} Vertex;

// Estrutura da Malha (Face-Vértice)
typedef struct {
    Vertex *vertices;    // Tabela/Array de vértices
    int num_vertices;    // Quantidade total de vértices
    Face *faces;         // Tabela/Array de faces
    int num_faces;       // Quantidade total de faces
} Mesh;

// Função auxiliar para adicionar o índice de uma face na lista de faces de um vértice
void add_face_to_vertex(Vertex *v, int face_idx) {
    if (v->num_faces >= v->capacity_faces) {
        v->capacity_faces = v->capacity_faces == 0 ? 4 : v->capacity_faces * 2;
        v->faces = (int*)realloc(v->faces, v->capacity_faces * sizeof(int));
    }
    v->faces[v->num_faces++] = face_idx;
}

// Carrega o arquivo no formato OFF
Mesh* load_off(const char *filename) {
    FILE *file = fopen(filename, "r");
    char header[10];
    fscanf(file, "%s", header);

    int num_vertices, num_faces, num_edges;
    fscanf(file, "%d %d %d", &num_vertices, &num_faces, &num_edges);

    // Alocar a malha
    Mesh *mesh = (Mesh*)malloc(sizeof(Mesh));
    mesh->num_vertices = num_vertices;
    mesh->num_faces = num_faces;
    mesh->vertices = (Vertex*)malloc(num_vertices * sizeof(Vertex));
    mesh->faces = (Face*)malloc(num_faces * sizeof(Face));

    // Ler coordenadas dos vértices e inicializar lista de faces
    for (int i = 0; i < num_vertices; i++) {
        fscanf(file, "%lf %lf %lf", &mesh->vertices[i].x, &mesh->vertices[i].y, &mesh->vertices[i].z);
        mesh->vertices[i].faces = NULL;
        mesh->vertices[i].num_faces = 0;
        mesh->vertices[i].capacity_faces = 0;
    }

    // Ler as faces e popular tanto a lista de vértices da face, quanto a lista de faces do vértice
    for (int i = 0; i < num_faces; i++) {
        int n;
        fscanf(file, "%d", &n); // Lê a quantidade de vértices da face atual
        mesh->faces[i].num_vertices = n;
        mesh->faces[i].vertices = (int*)malloc(n * sizeof(int));
        
        for (int j = 0; j < n; j++) {
            int v_idx;
            fscanf(file, "%d", &v_idx);
            mesh->faces[i].vertices[j] = v_idx;
            
            // Atualiza o vértice para adicionar que ele compõe esta face
            add_face_to_vertex(&mesh->vertices[v_idx], i);
        }
    }

    fclose(file);
    return mesh;
}

// Desaloca a memória ocupada pela malha
void free_mesh(Mesh *mesh) {
    if (!mesh) return;
    
    // Libera a lista de faces de cada vértice
    for (int i = 0; i < mesh->num_vertices; i++) {
        free(mesh->vertices[i].faces);
    }
    free(mesh->vertices);
    
    // Libera a lista de vértices de cada face
    for (int i = 0; i < mesh->num_faces; i++) {
        free(mesh->faces[i].vertices);
    }
    free(mesh->faces);
    
    free(mesh);
}

// Exporta a malha para um arquivo de imagem vetorial (SVG)
void export_to_svg(Mesh *mesh, const char *filename) {
    if (!mesh || mesh->num_vertices == 0) return;

    // Encontrar os limites da malha (bounding box) para normalizar a imagem
    double min_x = mesh->vertices[0].x, max_x = mesh->vertices[0].x;
    double min_y = mesh->vertices[0].y, max_y = mesh->vertices[0].y;

    for (int i = 1; i < mesh->num_vertices; i++) {
        if (mesh->vertices[i].x < min_x) min_x = mesh->vertices[i].x;
        if (mesh->vertices[i].x > max_x) max_x = mesh->vertices[i].x;
        if (mesh->vertices[i].y < min_y) min_y = mesh->vertices[i].y;
        if (mesh->vertices[i].y > max_y) max_y = mesh->vertices[i].y;
    }

    double width = max_x - min_x;
    double height = max_y - min_y;
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    // Escalar para caber numa tela de 800x800 com 10% de margem
    double scale_x = 800.0 / width;
    double scale_y = 800.0 / height;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    scale *= 0.9; 

    double offset_x = 400.0 - (min_x + width / 2.0) * scale;
    double offset_y = 400.0 - (min_y + height / 2.0) * scale;

    // Definir nome do arquivo SVG
    char out_filename[256];
    strncpy(out_filename, filename, sizeof(out_filename));
    char *dot = strrchr(out_filename, '.');
    if (dot) *dot = '\0';
    strcat(out_filename, ".svg");

    FILE *f = fopen(out_filename, "w");
    if (!f) {
        printf("Erro ao criar o arquivo SVG %s\n", out_filename);
        return;
    }

    // Cabecalho SVG
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"800\" height=\"800\" style=\"background: white;\">\n");

    // Desenhar as faces como poligonos
    for (int i = 0; i < mesh->num_faces; i++) {
        fprintf(f, "  <polygon points=\"");
        for (int j = 0; j < mesh->faces[i].num_vertices; j++) {
            int v_idx = mesh->faces[i].vertices[j];
            double vx = mesh->vertices[v_idx].x * scale + offset_x;
            // Eixo Y no SVG cresce para baixo, então precisamos inverter
            double vy = 800.0 - (mesh->vertices[v_idx].y * scale + offset_y);
            fprintf(f, "%.2f,%.2f ", vx, vy);
        }
        fprintf(f, "\" fill=\"#add8e6\" stroke=\"#00008b\" stroke-width=\"1\" stroke-linejoin=\"round\" />\n");
    }

    fprintf(f, "</svg>\n");
    fclose(f);
    printf(">> Imagem 2D gerada com sucesso: %s\n", out_filename);
}

// Função para imprimir os dados para conferência
void print_mesh_info(Mesh *mesh) {
    if (!mesh) return;
    printf("Malha contem %d vértices e %d faces.\n", mesh->num_vertices, mesh->num_faces);
    printf("\nPrimeiros 3 vértices:\n");
    for (int i = 0; i < 3 && i < mesh->num_vertices; i++) {
        printf("  v[%d]: (%.2f, %.2f, %.2f) -> Pertence a %d faces: ", i, mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z, mesh->vertices[i].num_faces);
        for (int j = 0; j < mesh->vertices[i].num_faces; j++) {
            printf("%d ", mesh->vertices[i].faces[j]);
        }
        printf("\n");
    }
    
    printf("\nPrimeiras 3 faces:\n");
    for (int i = 0; i < 3 && i < mesh->num_faces; i++) {
        printf("  f[%d]: %d vértices: ", i, mesh->faces[i].num_vertices);
        for (int j = 0; j < mesh->faces[i].num_vertices; j++) {
            printf("%d ", mesh->faces[i].vertices[j]);
        }
        printf("\n");
    }
    printf("--------------------------------------\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.off>\n", argv[0]);
        return 1;
    }

    for(int i = 1; i < argc; i++){
        printf("Tentando carregar arquivo: %s\n", argv[i]);
        Mesh *mesh = load_off(argv[i]);
        if (mesh) {
            printf("Arquivo %s carregado com sucesso!\n", argv[i]);
            print_mesh_info(mesh);
            export_to_svg(mesh, argv[i]);
            free_mesh(mesh);
        }
    }

    return 0;
}
