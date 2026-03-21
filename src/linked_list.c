#include "../linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct stNode {
    void *data;
    struct stNode *next;
    struct stNode *prev;
} node;

typedef struct stLista {
    int tam;
    node *head;
    node *tail;
} lista;

// ========== FUNÇÕES BASE ==========

lista *init_lista() {
    lista *l = malloc (sizeof(lista));
    if (l == NULL) {
        printf("LISTA: Erro ao alocar memoria para a lista!\n");
        return NULL;
    }

    l -> tam = 0;
    l -> head = NULL;
    l -> tail = NULL;

    return l;
}

int get_tam_lista(lista *l) {
    return l -> tam;
}

bool is_empty_lista(lista *l) {
    if (l -> tam == 0 || l -> head == NULL) {
        return true;
    }

    return false;
}

void insert_head(lista *l, void *new_data) {
    node *new = malloc (sizeof(node));
    if (new == NULL) {
        printf("LISTA: Erro ao alocar memoria para o novo node da lista!\n");
        return;
    }

    new -> data = new_data;
    new -> next = l -> head;
    new -> prev = NULL;

    if (l -> head != NULL) {
        l -> head -> prev = new;
    } else {
        l -> tail = new;
    }

    l -> head = new;
    l -> tam++;

}

void insert_tail(lista *l, void *new_data) {
    node *new = malloc (sizeof(node));
    if (new == NULL) {
        printf("LISTA: Erro ao alocar memoria para o novo node\n");
        return;
    }

    new -> data = new_data;
    new -> next = NULL;

   if (is_empty_lista(l)) {
        new -> prev = NULL;
        l -> head = new;
        l -> tail = new;
    }

    else {
        new -> prev = l -> tail;
        l -> tail -> next = new;
        l -> tail = new;
    }

    l -> tam ++;

}

void index_insert(lista *l, void *new_data, int index) {
    if (index > l -> tam || index < 0) {
        return;
    }

    if (index == 0) {
        insert_head(l, new_data);
        return;
    }

    if (index == l -> tam) {
        insert_tail(l, new_data);
        return;
    }

    node *new = malloc (sizeof(node));
    if (new == NULL) {
        printf("LISTA: Erro ao alocar memoria o novo no da lista!\n");
        return;
    }
    new -> data = new_data;

    node *current = l -> head;
    for (int i = 0; i < index; i++) {
        current = current -> next;
    }


    new -> next = current;
    new -> prev = current -> prev;

    current -> prev -> next = new;
    current -> prev = new;

    l -> tam++;

}

void *remove_head(lista *l) {
    if (l == NULL) {
        return NULL;
    }

    if (l -> head == NULL) {
        printf("LISTA: Nada a remover, a lista ja esta vazia!\n");
        return NULL;
    }

    node *removed = l -> head;

    void *data = l -> head -> data;

    if (l -> head == l -> tail) {
        l -> head = NULL;
        l -> tail = NULL;
    }

    else {
        l->head = l -> head -> next;
        l->head->prev = NULL;
    }

    l -> tam--;

    free(removed);

    return data;
}

void *remove_tail(lista *l) {
    if (l == NULL) {
        return NULL;
    }

    if (is_empty_lista(l)) {
        printf("LISTA: Nada a remover, a lista ja esta vazia!\n");
        return NULL;
    }

    node *removed = l -> tail;
    void *data = l -> tail -> data;

    if (l -> head == l -> tail) {
        l -> head = NULL;
        l -> tail = NULL;
    }

    else {
        l -> tail = l -> tail -> prev;
        l -> tail -> next = NULL;
    }

    l -> tam--;

    free(removed);

    return data;

}

void *remove_index(lista *l, int index) {
    if (l == NULL || index < 0 || index >= l -> tam ) {
        return NULL;
    }

    if (index == 0) {
        return remove_head(l);

    }

    if (index == l -> tam - 1) {
        return remove_tail(l);
    }

    node *current = l -> head;
    for (int i = 0; i < index; i++) {
        current = current -> next;
    }

    void *data = current -> data;

    current -> prev -> next = current -> next;
    current -> next -> prev = current -> prev;

    l -> tam--;
    free(current);
    return data;

}

void free_lista(lista *l, void (*free_data)(void *data)) {
    if (l == NULL) {
        return;
    }

    node *current = l -> head;
    while (current != NULL) {
        node *next = current -> next;

        if (free_data != NULL && current -> data != NULL) {
            free_data(current -> data);
        }

        free(current);
        current = next;
    }

    free(l);
}

// ========== FUNÇÕES GET ==========

void *get_head_data(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> head -> data;
}

void *get_node_data(node *n) {
    if (n == NULL || n -> data == NULL) {
        return NULL;
    }

    return n -> data;
}

node *get_head_node(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> head;
}

void *get_tail_data(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> tail -> data;
}

node *get_tail_node(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> tail;
}

node *get_node_index(lista *l, int index) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    if (index < 0 || index > l -> tam - 1) {
        printf("LISTA: Index inválido!\n");
        return NULL;
    }

    if (index == 0) {
        return get_head_node(l);
    }

    if (index == l -> tam - 1) {
        return get_tail_node(l);
    }

    node *current = l -> head;
    for (int i = 0; i < index; i++) {
        current = current -> next;
    }

    return current;
}

void *get_data_index(lista *l, int index) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    if (index < 0 || index > l -> tam - 1) {
        printf("Index inválido!\n");
        return NULL;
    }

    if (index == 0) {
        return get_head_data(l);
    }

    if (index == l -> tam - 1) {
        return get_tail_data(l);
    }

    node *current = l -> head;
    for (int i = 0; i < index; i++) {
        current = current -> next;
    }

    return current -> data;
}

node *go_next_node(node *n) {
    if (n == NULL || n -> next == NULL) {
        return NULL;
    }

    return n -> next;
}

node *go_prev_node(node *n) {
    if (n == NULL || n -> prev == NULL) {
        return NULL;
    }

    return n -> prev;
}

node *get_head_next(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> head -> next;
}

node *get_tail_prev(lista *l) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    return l -> tail -> prev;
}

// =========== FUNÇÕES AUXILIARES ===========

void *remove_first_data(lista *l, void *key, int (*compare)(void *a, void *b)) {
    if (l == NULL) {
        return NULL;
    }

    if (l -> tam == 0) {
        printf("LISTA: A lista está vazia! Nada a remover!\n");
        return NULL;
    }

    node *current = l -> head;
    int index = 0;
    while (current != NULL) {
        if (compare(key, current -> data) == 0) {
            return remove_index(l, index);
        }

        current = current -> next;
        index++;
    }

    printf("LISTA: Elemento não encontrado na lista\n");
    return NULL;
}

int get_index(lista *l, node *n) {
    if (l == NULL || n == NULL) {
        return -1;
    }

    int i = 0;

    node *aux = l -> head;
    while (aux != NULL && aux != n) {
        aux = go_next_node(aux);
        i++;
    }

    if (aux == NULL) {
        return -1;
    }

    return i;
}

void *search_lista(lista *l, void *key, int (*compare)(void *a, void *b)) {
    node *current = l -> head;
    while (current != NULL) {
        if (compare(key, current -> data) == 0) {
            return current -> data;
        }

        current = current -> next;
    }

    return NULL;
}

void print_lista(lista *l, void (*print_data)(void *data)) {
    if (l == NULL) {
        printf("LISTA: Ponteiro nulo passado para a funcao 'print_lista'\n");
        return;
    }

    node *current = l -> head;
    while (current != NULL) {
        if (current -> data != NULL && print_data != NULL) {
            print_data(current->data);
        }

        current = current -> next;
    }
}

void reverse_lista(lista *l) {
    if (l == NULL || l -> head == NULL) {
        return;
    }

    node *current = l -> head;
    node *temp = NULL;

    while (current != NULL) {
        temp = current -> prev;

        current -> prev = current -> next;
        current -> next = temp;

        current = current->prev;
    }

    temp = l->head;
    l->head = l->tail;
    l->tail = temp;
}

void clear_lista(lista *l, void (*free_data)(void *data)) {
    if (l == NULL) {
        return;
    }

    node *current = l->head;
    while (current != NULL) {
        node *next = current->next;

        if (free_data != NULL && current->data != NULL) {
            free_data(current->data);
        }

        free(current);
        current = next;
    }

    l->head = NULL;
    l->tail = NULL;
    l->tam = 0;
}

void foreach_lista(lista *l, void (*apply)(void *data)) {
    if (l == NULL) return;
    if (is_empty_lista(l)) {
            return;
    }

    node *current = l -> head;
    while (current != NULL) {
        if (apply) {
            apply(current -> data);
        }
        current = current -> next;
    }
}

void *find_max(lista *l, int (*compare)(void *a, void *b)) {
    if (is_empty_lista(l)) {
        return NULL;
    }

    if (l -> tam == 1) {
        return l -> head -> data;
    }

    void *max = l -> head -> data;
    node *current = l -> head -> next;
    while (current != NULL) {
        if (compare(current -> data, max) > 0) {
            max = current -> data;
        }
        current = current  -> next;
    }

    return max;
}

void *find_min(lista *l, int (*compare)(void *a, void *b)) {
     if (is_empty_lista(l)) {
        return NULL;
    }

    if (l -> tam == 1) {
        return l -> head -> data;
    }

    void *min = l -> head -> data;
    node *current = l -> head -> next;
    while (current != NULL) {
        if (compare(current -> data, min) < 0) {
            min = current -> data;
        }
        current = current  -> next;
    }

    return min;
}

int contains(lista *l, void *key, int (*compare)(void *a, void *b)) {
    if (search_lista(l, key, compare) != NULL) {
        return true;
    }

    return false;
}

void swap_node_data(node *n1, node *n2) {
    if (n1 == NULL || n2 == NULL) return;

    void *temp = n1 -> data;
    n1 -> data = n2 -> data;
    n2 -> data = temp;
}

lista *filter_lista(lista *l, int (*predicate)(void *data)) {
    if (l == NULL) return NULL;
    if (is_empty_lista(l)) return init_lista();

    lista *filtered_lista = init_lista();
    if (filtered_lista == NULL) {
        return NULL;
    }

    node *current = l -> head;
    while (current != NULL) {
        if (predicate && predicate(current -> data) == 1) {
            insert_tail(filtered_lista, current -> data);
        }
        current = current -> next;
    }

    return filtered_lista;
}

void map_lista(lista *l, void (*transform)(void *data)) {
    if (l == NULL || is_empty_lista(l) || transform == NULL) return;

    node *current = l -> head;

    while (current != NULL) {
        transform(current -> data);
        current = current -> next;
    }
}

int remove_all_if(lista *l, int (*predicate)(void *data), void (*free_data)(void *data)) {
    if (l == NULL || predicate == NULL) return 0;

    int removidos = 0;
    node *current = l->head;
    node *next_node = NULL;

    while (current != NULL) {
        next_node = current->next;

        if (predicate(current->data)) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                l->head = current->next;
            }

            if (current->next != NULL) {
                current->next->prev = current->prev;
            } else {
                l->tail = current->prev;
            }

            if (free_data != NULL) {
                free_data(current->data);
            }
            free(current);
            removidos++;
            l->tam--;
        }
        current = next_node;
    }

    return removidos;
}

// =========== FUNÇÕES DE ORDENAÇÃO PARA LISTA ==========

void insertion_sort_lista(lista *l, int (*cmp_data)(void *a, void *b)) {
    if (is_empty_lista(l) || get_tam_lista(l) <= 1) {
        return;
    }

    node *node_i = l -> head -> next;


    while (node_i != NULL) {
        node *node_j = node_i;
        node *j_prev = node_j -> prev;

        node *next_i = node_i -> next;


        while (j_prev != NULL && cmp_data(j_prev -> data, node_j -> data) > 0) {
            swap_node_data(j_prev, node_j);
            node_j = j_prev;
            j_prev = node_j -> prev;
        }

        node_i = next_i;
    }
}

static node *get_middle_lista(node *head) {
    node *slow = head;
    node *fast = head -> next;

    while (fast && fast -> next) {
        slow = slow -> next;
        fast = fast -> next -> next;
    }

    return slow;
}

static node *merge_nodes(node *right, node *left, int (*cmp)(void *a, void *b)) {
    if (left == NULL) {
        return right;
    }

    if (right == NULL) {
        return left;
    }

    node *final_head = NULL;

    if (cmp(left -> data, right -> data) <= 0) {
        final_head = left;
        left = left -> next;
    } else {
        final_head = right;
        right = right -> next;
    }

    final_head -> prev = NULL;

    node *tail = final_head;


    while (left != NULL && right != NULL) {
        if (cmp(left -> data, right -> data) <= 0) {
            tail -> next = left;
            left -> prev = tail;
            tail = left;
            left = left -> next;
        } else {
            tail -> next = right;
            right -> prev = tail;
            tail = right;
            right = right -> next;
        }
    }

    if (left != NULL) {
        tail -> next = left;
        left -> prev = tail;
    } else if (right != NULL) {
        tail -> next = right;
        right -> prev = tail;
    }

    return final_head;
}

static node *recursive_sort(node *head, int (*cmp)(void *a, void *b)) {
    if (head == NULL || head -> next == NULL) return head;

    node *mid = get_middle_lista(head);
    node *right_head = mid -> next;

    mid -> next = NULL;

    if (right_head != NULL) {
        right_head -> prev = NULL;
    }

    node *sorted_left = recursive_sort(head, cmp);
    node *sorted_right = recursive_sort(right_head, cmp);

    return merge_nodes(sorted_left, sorted_right, cmp);
}

void merge_sort_lista(lista *l, int (*cmp)(void *a, void *b)) {
    if (l == NULL || l -> tam <= 1) return;
    l -> head = recursive_sort(l -> head, cmp);

    node *tail = l -> head;

    if (tail != NULL) {
        while (tail -> next != NULL) {
            tail = tail -> next;
        }
    }

    l -> tail = tail;

}
