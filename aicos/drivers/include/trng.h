#ifndef __TRNG_H__
#define __TRNG_H__

int trng_get_data(int *data, int size);
int drbg_get_data(int *data, int size, int *seed, int seed_size);

#endif
