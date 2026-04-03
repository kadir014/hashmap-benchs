#ifndef HASH_H
#define HASH_H

#include <stdint.h>


static inline uint32_t hash_u32to32(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static inline uint64_t hash_u32to64(uint32_t x) {
    uint64_t h = x;
    h = (h ^ (h >> 16)) * 0x85ebca6b;
    h = (h ^ (h >> 13)) * 0xc2b2ae35;
    h = h ^ (h >> 16);
    return h;
}


#endif // HASH_H