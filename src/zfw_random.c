#include <time.h>
#include <stdlib.h>
#include "gce_random.h"

void InitRNG() {
    srand(time(NULL));
}

float RandPerc() {
    return (float)rand() / RAND_MAX;
}
