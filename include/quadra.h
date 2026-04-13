#ifndef PROJETO_01_EDII_QUADRA_H
#define PROJETO_01_EDII_QUADRA_H

typedef struct stQuadra quadra_t;

quadra_t quadra_init(const char *cep, double x, double y, double w, double h);

void quadra_destroy(quadra_t *q);

void quadra_set_corp(quadra_t *q, const char *nova_corp);

void quadra_set_corb(quadra_t *q, const char *nova_corb);

void quadra_set_sw(quadra_t *q, double novo_sw);

char  *quadra_get_corp(quadra_t *q);

char *quadra_get_corb(quadra_t *q);

double quadra_get_sw(const quadra_t *q);

void quadra_set_cq(quadra_t *q, double sw, const char *corb, const char *corp);

#endif //PROJETO_01_EDII_QUADRA_H
