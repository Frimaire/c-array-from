/*
  Copyright (c) 2025 Frimaire

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdint.h>
#include <string.h>
#include "array-from.h"
 
// 1M is large than the page in the most of systems (e.g. 4K for i386)
#define AF_PGSIZE 4096
#define AF_PAGE_POW_LIMIT 5

struct af_buf_info {
    uint8_t *buf;
    size_t len;
};

#define AF_POOL_CAP (AF_PGSIZE / sizeof(struct af_buf_info) - 1)
#define AF_POOL0_LEN (sizeof(pool0) / sizeof(struct af_buf_info))

struct af_pool {
    struct af_pool *next;
    struct af_buf_info pool[AF_POOL_CAP];
};

#define MAX2(a, b) ((a) > (b) ? (a) : (b))

static void array_from_free(struct af_buf_info *pool0, size_t pool0_size, struct af_pool *pool1) {
    // free pool0
    // pool0[0] is on stack
    while(pool0_size > 1) {
        pool0_size--;
        if(pool0[pool0_size].buf) {
            free(pool0[pool0_size].buf);
        }
    }
    while(pool1) {
        struct af_pool *pooln;
        size_t k = 0;
        while(k < AF_POOL_CAP) {
            if(pool1->pool[k].buf) {
                free(pool1->pool[k].buf);
            }
            k++;
        }
        pooln = pool1->next;
        free(pool1);
        pool1 = pooln;
    }
}

ssize_t array_from(void **array, bool (*generator)(void *item, size_t index, void *param), size_t item_size, void *param) {
    if(!item_size) {
        return -1;
    }

    // place in the stack at first, then open a pool
    struct af_buf_info pool0[8];
    memset(pool0, 0, sizeof(pool0));
    struct af_pool *pool1 = NULL, *pooln = NULL;
    struct af_buf_info *bi = pool0;

    size_t len = 0, p = 0;
    // first buf on the stack
    uint8_t buf0[256];
    uint8_t *buf = buf0;
    pool0[0].buf = buf0;
    size_t cap = sizeof(buf0) / item_size;
    
    // check if larger buffer is required
    if(!cap) {
        p = 1;
        size_t sz = MAX2(item_size, AF_PGSIZE);
        buf = calloc(sz, 1);
        if(!buf) {
            return -1;
        }
        cap = sz / item_size;
        pool0[1].buf = buf;
        bi = &pool0[1];
    }
    
    bool done = false;
    while(true) {
        size_t l = 0;
        // iterate
        while(l < cap) {
            done = generator(buf + item_size * l, len, param);
            if(done) {
                break;
            }
            len++;
            l++;
        }
        bi->len = l;
        if(done) {
            break;
        }
        
        // allocate more spaces
        // use next info
        p++;
        if(!pool1 && p < AF_POOL0_LEN) {
            // using pool0
            // buf size for pool0: 256, 4K, 16K, 64K, 256K, 1M, 1M, 1M
            // always 1M for bufs in af_pool
            if(p <= AF_PAGE_POW_LIMIT) {
                // increase the capacity by *4 until 1M
                // cap keeps afterwards
                cap = MAX2(item_size, AF_PGSIZE << (2 * (p - 1))) / item_size;
            }
            bi = &pool0[p];
        } else if((!pool1 && p == AF_POOL0_LEN) || p == AF_POOL_CAP) {
            // allocate more pools
            struct af_pool *next = calloc(1, sizeof(struct af_pool));
            if(!next) {
                array_from_free(pool0, AF_POOL0_LEN, pool1);
                return -1;
            }
            if(!pool1) {
                pool1 = pooln = next;
            } else {
                pooln->next = next;
                pooln = next;
            }
            p = 0;
            bi = &pooln->pool[0];
        } else {
            bi = &pooln->pool[p];
        }
        
        // allocate buffers
        bi->len = 0;
        bi->buf = buf = calloc(cap * item_size, 1);
        if(!buf) {
            array_from_free(pool0, AF_POOL0_LEN, pool1);
            return -1;
        }
    }

    // allocate the result
    // always allocate 1 byte+
    uint8_t *res = calloc(MAX2(len * item_size, 1), 1);
    if(!res) {
        array_from_free(pool0, AF_POOL0_LEN, pool1);
        return -1;
    }

    // collect
    pooln = NULL;
    p = pool0[0].len ? 0 : 1;
    bi = pool0[0].len ? &pool0[0] : &pool0[1];
    size_t pos = 0;
    while(pos < len) {
        memmove(res + pos * item_size, bi->buf, bi->len * item_size);
        pos += bi->len;
        if(pos == len) {
            break;
        }

        // switch buffer
        p++;
        if(!pooln && p < AF_POOL0_LEN) {
            bi = &pool0[p];
        } else {
            if(!pooln) {
                pooln = pool1;
                p = 0;
            } else if(p == AF_POOL_CAP) {
                pooln = pooln->next;
                p = 0;
            }
            bi = &pooln->pool[p];
        }
    }

    array_from_free(pool0, AF_POOL0_LEN, pool1);
    *array = res;
    return len;
}

