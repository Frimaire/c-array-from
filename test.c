#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include "array-from.h"

bool gen(void *item, size_t index, void *param) {
    uint32_t limit = *(uint32_t *)param;
    if(index == limit) {
        return true;
    }
    *(uint32_t *)item = rand();
    return false;
}

int main() {
    uint32_t limit;
    assert(scanf("%" PRIu32, &limit) == 1);
    
    // generate
    uint32_t *arr;
    srand(10000);
    ssize_t l = array_from((void **)&arr, gen, sizeof(float), &limit);
    assert(l == limit);

    // verify
    srand(10000);
    for(size_t k = 0; k < limit; k++) {
        assert(arr[k] == rand());
    }

    free(arr);

    return 0;
}
