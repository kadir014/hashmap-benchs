/* 
    Reference: https://github.com/Matiiss/astropath/tree/6bc0e4cc3d0008873a164c80a92eba52b2eac6a7
*/

#ifndef ASTROPATH_H
#define ASTROPATH_H


#include <stdlib.h>
#include <stdio.h>


/*******************************************************************************
 *                                                                             *
 *                                  AP_list.h                                  *
 *                                                                             *
 *******************************************************************************/

#define AP_LIST_CHUNK 32

typedef struct AP_List AP_List;

struct AP_List {
    void **array;
    size_t length;  // item count in the array
    size_t _size;  // allocated size

    int (*append)(AP_List *self, void *item);
    void *(*get_at)(AP_List *self, size_t index);
    int (*set_at)(AP_List *self, size_t index, void *item);
    void *(*pop)(AP_List *self, size_t index);
    void *(*pop_end)(AP_List *self);
    void (*free)(AP_List *self);
};

int AP_ListInit(AP_List *self);
int AP_ListAppend(AP_List *self, void *item);
void *AP_ListGetAt(AP_List *self, size_t index);
int AP_ListSetAt(AP_List *self, size_t index, void *item);
void *AP_ListPop(AP_List *self, size_t index);
void *AP_ListPopEnd(AP_List *self);
void AP_ListFree(AP_List *self);


/*******************************************************************************
 *                                                                             *
 *                                  AP_dict.h                                  *
 *                                                                             *
 *******************************************************************************/

typedef struct AP_Dict AP_Dict;
typedef struct AP_DictNode AP_DictNode;
typedef long long AP_Hash;
typedef long long (*AP_HashFunc)(void *key);
typedef int (*AP_DictEqCheck)(void *key1, void *key2);

struct AP_DictNode {
    int state;
    AP_Hash hash;
    void *key;
    void *item;
};

struct AP_Dict {
    AP_List *nodes;
    size_t using_field;
    size_t fullness;
    AP_HashFunc hash;
    AP_DictEqCheck eq_check;

    void *(*get)(AP_Dict *self, void *key);
    int (*set)(AP_Dict *self, void *key, void *item);
    int (*del)(AP_Dict *self, void *key);
    void (*free)(AP_Dict *self);
};

int AP_DictInit(AP_Dict *self, AP_HashFunc hash, AP_DictEqCheck eq_check);
void *AP_DictGet(AP_Dict *self, void *key);
int AP_DictSet(AP_Dict *self, void *key, void *item);
int AP_DictDel(AP_Dict *self, void *key);
void AP_DictFree(AP_Dict *self);


#endif // ASTROPATH_H