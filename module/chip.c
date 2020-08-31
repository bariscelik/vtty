#include "chip.h"

struct shash_desc *init_sdesc(struct crypto_shash *alg)
{
    struct shash_desc *sdesc;

    sdesc = kmalloc(sizeof(*sdesc) + crypto_shash_descsize(alg), GFP_KERNEL);
    if (!sdesc)
        return ERR_PTR(-ENOMEM);
    sdesc->tfm = alg;

    return sdesc;
}

int calc_hash(struct crypto_shash *alg,
                     const unsigned char *data, unsigned int datalen,
                     unsigned char *digest)
{
    struct shash_desc *sdesc;
    int ret;

    sdesc = init_sdesc(alg);
    if (IS_ERR(sdesc)) {
        pr_err("can't alloc sdesc\n");
        return PTR_ERR(sdesc);
    }

    ret = crypto_shash_digest(sdesc, data, datalen, digest);
    kfree(sdesc);
    return ret;
}

int generate_hash(const unsigned char *data, unsigned int datalen,
                     unsigned char *digest)
{
    struct crypto_shash *alg;
    char *hash_alg_name = "sha256";
    int ret;

    alg = crypto_alloc_shash(hash_alg_name, 0, 0);
    if (IS_ERR(alg)) {
        pr_err("can't alloc alg %s\n", hash_alg_name);
        return PTR_ERR(alg);
    }
    ret = calc_hash(alg, data, datalen, digest);

    crypto_free_shash(alg);
    return ret;
}