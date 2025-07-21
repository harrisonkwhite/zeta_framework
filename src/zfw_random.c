#include <time.h>
#include <stdlib.h>
#include "zfw_random.h"

void ZFWInitRNG() {
    srand(time(NULL));
}

float ZFWRandPerc() {
    return (float)rand() / (RAND_MAX + 1.0f);
}

float ZFWRandPercIncl() {
    return (float)rand() / RAND_MAX;
}

int ZFWRandRangeI(const int beg, const int end) {
    assert(beg <= end);
    return beg + (rand() % (end - beg));
}
