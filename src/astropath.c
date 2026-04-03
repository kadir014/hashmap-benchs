/* 
    Reference: https://github.com/Matiiss/astropath/tree/6bc0e4cc3d0008873a164c80a92eba52b2eac6a7
*/

#include <stdlib.h>
#include <stdio.h>

#include "astropath.h"


/*******************************************************************************
 *                                                                             *
 *                                  AP_list.c                                  *
 *                                                                             *
 *******************************************************************************/

int AP_ListInit(AP_List *self) {
    self->array = (void **)malloc(AP_LIST_CHUNK * sizeof(void *));

    if (!self->array) {
        return 1;
    }

    self->length = 0;
    self->_size = AP_LIST_CHUNK;

    self->append = &AP_ListAppend;
    self->get_at = &AP_ListGetAt;
    self->set_at = &AP_ListSetAt;
    self->pop = &AP_ListPop;
    self->pop_end = &AP_ListPopEnd;
    self->free = &AP_ListFree;

    return 0;
}

int AP_ListAppend(AP_List *self, void *item) {
    self->length++;
    if (self->length > self->_size) {
        self->_size += AP_LIST_CHUNK;
        self->array = realloc(self->array, self->_size * sizeof(void *));
        if (!self->array) {
            return 1;
        }
    }

    self->array[self->length - 1] = item;
    return 0;
}

void *AP_ListGetAt(AP_List *self, size_t index) {
    if (index >= self->length) {
        return NULL;
    }

    return self->array[index];
}

int AP_ListSetAt(AP_List *self, size_t index, void *item) {
    if (index >= self->length) {
        return 1;
    }

    self->array[index] = item;
    return 0;
}

void *AP_ListPop(AP_List *self, size_t index) {
    void *item = self->get_at(self, index);
    if (!item) {
        return NULL;
    }

    self->length--;
    for (size_t i = index; i < self->length; ++i) {
        self->array[i] = self->array[i + 1];
    }

    return item;
}

void *AP_ListPopEnd(AP_List *self) {
    return self->pop(self, self->length - 1);
}

void AP_ListFree(AP_List *self) {
    free(self->array);
}


/*******************************************************************************
 *                                                                             *
 *                                  AP_dict.c                                  *
 *                                                                             *
 *******************************************************************************/

#define TAKEN 0
#define FREE 1
#define DUMMY 2

#define UNSET 0
#define SET 1

AP_List *create_list(size_t new_size);

typedef struct {
    AP_DictNode *node;
    size_t slot;
} SlotTuple;

int AP_DictInit(AP_Dict *self, AP_HashFunc hash, AP_DictEqCheck eq_check) {
    self->nodes = create_list(512);
    if (!self->nodes) {
        return 1;
    }

    self->using_field = 0;
    self->fullness = 0;
    self->hash = hash;
    self->eq_check = eq_check;

    self->get = &AP_DictGet;
    self->set = &AP_DictSet;
    self->del = &AP_DictDel;
    self->free = &AP_DictFree;

    return 0;
}

int match_keys(AP_Dict *self, AP_DictNode *current, AP_DictNode *target) {
    if (current->key == target->key) {
        return 1;
    }
    if (current->hash != target->hash) {
        return 0;
    }
    return self->eq_check(current->key, target->key);
}

SlotTuple lookup(AP_Dict *self, AP_DictNode *target) {
    SlotTuple tpl;
    int slot_state = UNSET;

    size_t i = target->hash % self->nodes->length;
    AP_Hash perturb = target->hash;

    while (1) {
        AP_DictNode *node = self->nodes->get_at(self->nodes, i);
        if (node->state == FREE) {
            tpl.node = node;
            if (slot_state == UNSET) {
                tpl.slot = i;
            }
            return tpl;
        } else if (node->state == DUMMY) {
            if (slot_state == UNSET) {
                slot_state = SET;
                tpl.slot = i;
            }
        } else if (match_keys(self, node, target)) {
            tpl.node = node;
            tpl.slot = i;
            return tpl;
        }

        i = (5 * i + perturb + 1) % self->nodes->length;
        perturb >>= 5;
    }
}

AP_List *create_list(size_t new_size) {
    AP_List *new_list = malloc(sizeof(AP_List));
    if (!new_list) {
        return NULL;
    }

    if (AP_ListInit(new_list)) {
        return NULL;
    }

    for (size_t i = 0; i < new_size; ++i) {
        AP_DictNode *new_node = malloc(sizeof(AP_DictNode));
        if (!new_node) {
            return NULL;
        }

        new_node->state = FREE;
        new_node->key = NULL;
        if (new_list->append(new_list, (void *)new_node)) {
            return NULL;
        }
    }

    return new_list;
}

int resize(AP_Dict *self, size_t new_size) {
    AP_List *old_list = self->nodes;
    AP_List *new_list = create_list(new_size);
    if (!new_list) {
        return 1;
    }

    self->nodes = new_list;

    for (size_t i = 0; i < old_list->length; ++i) {
        AP_DictNode *node = old_list->get_at(old_list, i);
        if (node->state == TAKEN) {
            SlotTuple tpl = lookup(self, node);
            if (tpl.node->state != FREE) {
                return 1;
            }
            free(new_list->get_at(new_list, tpl.slot));
            if (new_list->set_at(new_list, tpl.slot, (void *)node)) {
                return 1;
            }
        } else {
            free(node);
        }
    }

    old_list->free(old_list);
    free(old_list);

    return 0;
}

void *AP_DictGet(AP_Dict *self, void *key) {
    AP_DictNode node;
    node.hash = self->hash(key);
    node.key = key;

    SlotTuple tpl = lookup(self, &node);
    if (tpl.node->state == FREE) {
        return NULL;
    }

    return tpl.node->item;
}

int AP_DictSet(AP_Dict *self, void *key, void *item) {
    AP_DictNode node;
    node.hash = self->hash(key);
    node.key = key;

    SlotTuple tpl = lookup(self, &node);
    if (tpl.node->state != TAKEN) {
        int state = tpl.node->state;
        tpl.node->state = TAKEN;
        tpl.node->hash = node.hash;
        tpl.node->key = key;
        tpl.node->item = item;
        self->using_field++;

        if (state == FREE) {
            self->fullness++;
            if (self->fullness * 3 > self->nodes->length * 2) {
                if (resize(self, self->nodes->length * 4)) {
                    return 1;
                }
            }
        }
    } else {
        tpl.node->item = item;
    }
    return 0;
}

int AP_DictDel(AP_Dict *self, void *key) {
    AP_DictNode node;
    node.hash = self->hash(key);
    node.key = key;

    SlotTuple tpl = lookup(self, &node);
    if (tpl.node->state != TAKEN) {
        return 1;
    }

    tpl.node->state = DUMMY;
    self->using_field--;

    return 0;
}

void AP_DictFree(AP_Dict *self) {
    for (size_t i = 0; i < self->nodes->length; ++i) {
        free(self->nodes->get_at(self->nodes, i));
    }
    self->nodes->free(self->nodes);
    free(self->nodes);
}