#include <crypto/hash.h>

#ifndef MODULE_CHIP_H
#define MODULE_CHIP_H
struct shash_desc *init_sdesc(struct crypto_shash *alg);
int calc_hash(struct crypto_shash *alg,
                     const unsigned char *data, unsigned int datalen,
                     unsigned char *digest);
int generate_hash(const unsigned char *data, unsigned int datalen,
                             unsigned char *digest);
#endif //MODULE_CHIP_H
