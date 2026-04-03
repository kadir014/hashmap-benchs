#include <iostream>
#include <unordered_map>
#include "profiler.h"
#include "hash.h"

extern "C" {
    #include "astropath.h"
    #include "nova.h"
}


typedef struct {
    uint32_t id;
    int value;
} Item;

void generate_items(Item *items, size_t n) {
    unsigned int seed = 67;
    srand(seed);

    for (size_t i = 0; i < n; i++) {
        items[i].id = (uint32_t)rand();
        items[i].value = rand();
    }
}


#define BENCH_OUTER 30 // to reduce noise in the timings
#define BENCH_INNER 1 // to scale the amount of iterated items
#define ITEMS_N 5'000'000


double get_total(double *timings) {
    double total = 0.0;
    for (size_t i = 0; i < BENCH_OUTER; i++) {
        total += timings[i];
    }
    return total;
}

double get_avg(double *timings) {
    return get_total(timings) / (double)(BENCH_OUTER);
}

double get_min(double *timings) {
    double min = 9999999.9;
    for (size_t i = 0; i < BENCH_OUTER; i++) {
        if (timings[i] < min) {
            min = timings[i];
        }
    }

    return min;
}

double get_max(double *timings) {
    double max = -9999999.9;
    for (size_t i = 0; i < BENCH_OUTER; i++) {
        if (timings[i] > max) {
            max = timings[i];
        }
    }

    return max;
}

void print_timings(double *timings) {
    double sum = get_total(timings);
    double avg = get_avg(timings);
    double min = get_min(timings);
    double max = get_max(timings);

    printf("  total: %f s\n", sum);
    printf("  avg:   %f ms\n", avg * 1000.0);
    printf("  min:   %f ms\n", min * 1000.0);
    printf("  max:   %f ms\n", max * 1000.0);
}


static inline AP_Hash ap_hash(void *obj) {
    uint32_t x = *(uint32_t *)obj;
    return (AP_Hash)hash_u32to32(x);
}

static inline int ap_eq_check(void *obj1, void *obj2) {
    uint32_t x = *(uint32_t *)obj1;
    uint32_t y = *(uint32_t *)obj2;
    return x == y;
}

static inline nv_uint64 nv_hash(void *obj) {
    Item *item = (Item *)obj;
    return (nv_uint64)hash_u32to64(item->id);
}


int main() {
    Item *items = (Item *)malloc(sizeof(Item) * ITEMS_N);
    generate_items(items, ITEMS_N);

    double *timings = (double *)malloc(sizeof(double) * BENCH_OUTER);


/*******************************************************************************
 *                                                                             *
 *                          C++ std::unordered_map                             *
 *                                                                             *
 *******************************************************************************/

    

    printf(
        "std::unordered_map  -  PUT  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        std::unordered_map<uint32_t, Item *> map;

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    map[item.id] = &item;
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
    }

    printf(
        "\nstd::unordered_map  -  GET  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        std::unordered_map<uint32_t, Item *> map;
        Item noopt_item;
        
        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];
            map[item.id] = &item;
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    noopt_item = *map[item.id];
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
    }

    printf(
        "\nstd::unordered_map  -  REMOVE  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        std::unordered_map<uint32_t, Item *> map;
        Item noopt_item;

        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];
            map[item.id] = &item;
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    map.erase(item.id);
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
    }


/*******************************************************************************
 *                                                                             *
 *                           Astropath Dictionary                              *
 *                                                                             *
 *******************************************************************************/
    printf(
        "\nAP_Dict  -  PUT  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        AP_Dict *dict = (AP_Dict *)malloc(sizeof(AP_Dict));
        AP_DictInit(dict, (AP_HashFunc)&ap_hash, (AP_DictEqCheck)&ap_eq_check);

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    AP_DictSet(dict, &(item.id), &item);
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        AP_DictFree(dict);
    }

    printf(
        "\nAP_Dict  -  GET  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        AP_Dict *dict = (AP_Dict *)malloc(sizeof(AP_Dict));
        AP_DictInit(dict, (AP_HashFunc)&ap_hash, (AP_DictEqCheck)&ap_eq_check);
        Item *noopt_item;

        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];

            AP_DictSet(dict, &(item.id), &item);
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    noopt_item = (Item *)AP_DictGet(dict, &(item.id));
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        AP_DictFree(dict);
    }

    printf(
        "\nAP_Dict  -  REMOVE  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        AP_Dict *dict = (AP_Dict *)malloc(sizeof(AP_Dict));
        AP_DictInit(dict, (AP_HashFunc)&ap_hash, (AP_DictEqCheck)&ap_eq_check);
        Item *noopt_item;

        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];

            AP_DictSet(dict, &(item.id), &item);
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    AP_DictDel(dict, &(item.id));
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        AP_DictFree(dict);
    }


/*******************************************************************************
 *                                                                             *
 *                           Nova Physics Hashmap                              *
 *                                                                             *
 *******************************************************************************/
    printf(
        "\nnvHashmap  -  PUT  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        nvHashMap *map = nvHashMap_new(sizeof(Item), 0, nv_hash);

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    nvHashMap_set(map, &item);
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        nvHashMap_free(map);
    }

    printf(
        "\nnvHashmap  -  GET  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        nvHashMap *map = nvHashMap_new(sizeof(Item), 0, nv_hash);
        Item *noopt_item;

        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];

            nvHashMap_set(map, &item);
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    Item search_item;
                    search_item.id = item.id;
                    noopt_item = (Item *)nvHashMap_get(map, &search_item);
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        nvHashMap_free(map);
    }

    printf(
        "\nnvHashmap  -  REMOVE  -  %u items @ %ux%u runs\n",
        ITEMS_N, BENCH_INNER, BENCH_OUTER
    );
    {
        nvPrecisionTimer timer;

        nvHashMap *map = nvHashMap_new(sizeof(Item), 0, nv_hash);
        Item *noopt_item;

        for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
            Item item = items[item_i];

            nvHashMap_set(map, &item);
        }

        for (size_t bencho_i = 0; bencho_i < BENCH_OUTER; bencho_i++) {
            nvPrecisionTimer_start(&timer);

            for (size_t benchi_i = 0; benchi_i < BENCH_INNER; benchi_i++) {
                for (size_t item_i = 0; item_i < ITEMS_N; item_i++) {
                    Item item = items[item_i];

                    Item search_item;
                    search_item.id = item.id;
                    nvHashMap_remove(map, &search_item);
                }
            }

            nvPrecisionTimer_stop(&timer);
            timings[bencho_i] = timer.elapsed;
        }

        print_timings(timings);
        nvHashMap_free(map);
    }

    free(items);
}