#include "../include/svg_handler.h"

#include "exhash.h"
#include <stdio.h>
#include <stdlib.h>
#include "quadra.h"

FILE *svg_init(const char* caminho_arquivo) {
	FILE *svg = fopen(caminho_arquivo, "w");
	if (svg == NULL) {
		perror("ERRO ao abrir o arquivo SVG");
		return NULL;
	}

	fprintf(svg, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	fprintf(svg, "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" width=\"1000\" height=\"1000\">\n");

	fprintf(svg, "<g>\n");

	return svg;
}


void svg_quadra_insert(FILE *svg, const quadra_t *q) {
	fprintf(svg, "\t<rect id=\"%s\" x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" stroke=\"%s\" fill=\"%s\" stroke-width=\"%lf\" opacity=\"%lf\" />\n",
		quadra_get_cep(q),
		quadra_get_x(q),
		quadra_get_y(q),
		quadra_get_w(q),
		quadra_get_h(q),
		quadra_get_corb(q),
		quadra_get_corp(q),
		quadra_get_sw(q), 0.5);
}

void fecha_svg(FILE *svg) {
	if (svg == NULL) return;

	fprintf(svg, "</g>\n");

	fprintf(svg, "</svg>\n");

	fclose(svg);
}

void svg_x_vermelho(FILE *svg, double x, double y) {
	double tamanho = 5.0;

	fprintf(svg, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"red\" stroke-width=\"2\" />\n",
			x - tamanho, y - tamanho, x + tamanho, y + tamanho);

	fprintf(svg, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"red\" stroke-width=\"2\" />\n",
			x - tamanho, y + tamanho, x + tamanho, y - tamanho);
}

void svg_escrever_populacao_pq(FILE *svg, quadra_t *quadra) {

	double x = quadra_get_x(quadra);
	double y = quadra_get_y(quadra);
	double w = quadra_get_w(quadra);
	double h = quadra_get_h(quadra);

	int face_n = quadra_get_N(quadra);
	int face_l = quadra_get_L(quadra);
	int face_s = quadra_get_S(quadra);
    int face_o = quadra_get_O(quadra);

	int total_habitantes = face_l + face_n + face_s + face_o;

	double centro_x = x + (w / 2.0);
	double centro_y = y + (h / 2.0);

	fprintf(svg, "<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" text-anchor=\"middle\" dominant-baseline=\"middle\" font-weight=\"bold\">%d</text>\n",
			centro_x, centro_y, total_habitantes);

	fprintf(svg, "<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" text-anchor=\"middle\">%d</text>\n",
			centro_x, y + 15.0, face_n);

	fprintf(svg, "<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" text-anchor=\"middle\">%d</text>\n",
			centro_x, (y + h) - 5.0, face_s);


	fprintf(svg, "<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" dominant-baseline=\"middle\">%d</text>\n",
			(x + w) - 15.0, centro_y, face_l);

	fprintf(svg, "<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" dominant-baseline=\"middle\">%d</text>\n",
			x + 5.0, centro_y, face_o);
}

void svg_cruz_insert(FILE *svg, double cx, double cy) {
	fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"6\" stroke-linecap=\"round\"/>\n",
		cx - 20.0, cy,       cx + 20.0, cy);
	fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"6\" stroke-linecap=\"round\"/>\n",
		cx, cy - 20.0, cx, cy + 20.0);
}

void svg_marcador_mudanca(FILE *svg, double cx, double cy, const char *cpf) {
	fprintf(svg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"30\" height=\"15\""
		"stroke=\"red\" fill=\"red\" stroke-width=\"1\"/>\n", cx - 15.0, cy - 7.5);
	fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\"font-size=\"6\" font-style=\"italic\" fill=\"white\"text-anchor=\"middle\" "
		 "dominant-baseline=\"middle\">%s</text>\n", cx, cy, cpf);
}

void svg_circulo_despejo(FILE *svg, double cx, double cy) {
	fprintf(svg, "\t<circle cx=\"%lf\" cy=\"%lf\" r=\"5\" fill=\"black\"/>\n",cx, cy);
}

void svg_desenha_mapa_base(FILE *svg, exhash_t *mapa_quadras) {

	uint64_t total_quadras;
	void **lista_quadras = exhash_get_all(mapa_quadras, &total_quadras);

	if (lista_quadras == NULL) return;

	for (uint64_t i = 0; i < total_quadras; i++) {
		quadra_t *q = (quadra_t *) lista_quadras[i];

		svg_quadra_insert(svg, q);

		quadra_destroy(q);
	}
	free(lista_quadras);
}