#include <time.h>
#include <stdlib.h>
#include "zfw_random.h"

void ZFW_InitRNG() {
    srand(time(NULL));
}

float ZFW_RandPerc() {
    return (float)rand() / (RAND_MAX + 1.0f);
}

float ZFW_RandPercIncl() {
    return (float)rand() / RAND_MAX;
}

int ZFW_RandRangeI(const int beg, const int end) {
    assert(beg <= end);
    return beg + (rand() % (end - beg));
}
