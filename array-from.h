#include <stdlib.h>
#include <stdbool.h>

#ifndef _ARRAY_FROM_H
#define _ARRAY_FROM_H

/*
  This function calls generator repeatedly until it returns false.
*/
ssize_t array_from(void **array, bool (*generator)(void *item, size_t index, void *param), size_t item_size, void *param);

#endif
