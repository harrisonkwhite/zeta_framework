#include <time.h>
#include <stdlib.h>
#include "zfw_random.h"

void InitRNG() {
    srand(time(NULL));
}

float RandPerc() {
    return (float)rand() / (RAND_MAX + 1.0f);
}

int RandRangeI(const int beg, const int end) {
    assert(beg <= end);
    return beg + (rand() % (end - beg));
}
