
===== system =====

You are a helpful programming assistant and an expert in the development of Persistent Memory programs. You are helping a user repair the bugs inside a Persistent Memory program. 
The user has written a program in C programming language while using the PMDK library libpmemobj. However, the program has some bugs and is not working as expected. 
The user has analysed the program with a bug detection tool that has located the bug or bugs. You will use this information to generate a corrected version of the program.
The bug or bugs to repair will be located in an area of the code delimited by an expression. The beggining and end of the area of the code where a bug is and where the fix is 
supposed to go will be delimited by the exprexion '// BUG //'.
When presenting the correction, present the whole code and not just the corrected segment of the code.
Put the whole corrected program within code delimiters, as follows:
                ''' C
                # YOUR CODE HERE
                '''.

===== user =====

### EXAMPLES

## Example 1

''' C

static void
create_hashmap(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint32_t seed)
{
	size_t len = INIT_BUCKETS_NUM;
	size_t sz = sizeof(struct buckets) +
			len * sizeof(TOID(struct entry));

	TX_BEGIN(pop) {
		TX_ADD(hashmap);

		D_RW(hashmap)->seed = seed;
		do {
			D_RW(hashmap)->hash_fun_a = (uint32_t)rand();
		} while (D_RW(hashmap)->hash_fun_a == 0);
		D_RW(hashmap)->hash_fun_b = (uint32_t)rand();
		D_RW(hashmap)->hash_fun_p = HASH_FUNC_COEFF_P;
		
		// BUG //

		D_RW(hashmap)->buckets = TX_ZALLOC(struct buckets, sz);
		D_RW(D_RW(hashmap)->buckets)->nbuckets = len;
        TX_ADD(hashmap);
		
		// BUG //

	} TX_ONABORT {
		fprintf(stderr, "%s: transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
	} TX_END
}

'''.

===== assistant =====

## Correction 1

''' C

    static void
create_hashmap(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint32_t seed)
{
	size_t len = INIT_BUCKETS_NUM;
	size_t sz = sizeof(struct buckets) +
			len * sizeof(TOID(struct entry));

	TX_BEGIN(pop) {
		TX_ADD(hashmap);

		D_RW(hashmap)->seed = seed;
		do {
			D_RW(hashmap)->hash_fun_a = (uint32_t)rand();
		} while (D_RW(hashmap)->hash_fun_a == 0);
		D_RW(hashmap)->hash_fun_b = (uint32_t)rand();
		D_RW(hashmap)->hash_fun_p = HASH_FUNC_COEFF_P;
		
		// BUG //
		
		D_RW(hashmap)->buckets = TX_ZALLOC(struct buckets, sz);
		D_RW(D_RW(hashmap)->buckets)->nbuckets = len;
		
		// BUG //

	} TX_ONABORT {
		fprintf(stderr, "%s: transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
	} TX_END
}

'''.

## Explanation
In the example, the bug to repair is located in the area of the code delimited by the two '// BUG //' expressions. 
In the correction of the example, the bug fix is put in the area of the code delimited by the two '// BUG //' expressions.
In the correction, one of the 'TX_ADD(hashmap);' calls has been removed. The 'TX_ADD(hashmap);' call is only made once at the beginning of the transaction block,
which is the correct way to indicate that 'hashmap' is part of the transaction.
By removing the redundant 'TX_ADD(hashmap);', it is removed the unnecessary duplication of statements. This change simplifies the code while preserving the correct transactional behaviour.

===== user =====

## Example 2

''' C

int main(int argc, char *argv[]) {

    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, NULL, PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid value = pmemobj_tx_alloc(sizeof(int), 0);
    if (PMEMOID_IS_NULL(value)) {
        perror("pmemobj_tx_alloc");
        pmemobj_close(pop);
        return 1;
    }

    int *value_ptr = pmemobj_direct(value);

    // BUG //

    if (*value_ptr == 42) {
        TX_ADD(value);
        TX_ADD(value);
    }

    // BUG //

    pmemobj_close(pop);

    return 0;
}

'''.

===== assistant =====

## Correction 2

''' C
 
int main(int argc, char *argv[]) {
    
    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, NULL, PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid value = pmemobj_tx_alloc(sizeof(int), 0);
    if (PMEMOID_IS_NULL(value)) {
        perror("pmemobj_tx_alloc");
        pmemobj_close(pop);
        return 1;
    }

    int *value_ptr = pmemobj_direct(value);

    // BUG //

    if (*value_ptr == 42) {
        TX_ADD(value);
    }

    // BUG //

    pmemobj_close(pop);

    return 0;
}

'''.

## Explanation
In the example, the bug to repair is located in the area of the code delimited by the two '// BUG //' expressions. 
In the correction of the example, the bug fix is put in the area of the code delimited by the two '// BUG //' expressions.
In the correction, one of the 'TX_ADD(value);' calls has been removed because it is redundant.
The correction eliminates unnecessary overhead and maintains the correct behaviour intended in the code, including the value in the transaction if the condition is met, preserving the correct transactional behaviour.


===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM

''' C
/*
 * hm_tx_insert -- inserts specified value into the hashmap,
 * returns:
 * - 0 if successful,
 * - 1 if value already existed,
 * - -1 if something bad happened
 */
int
hm_tx_insert(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
	uint64_t key, PMEMoid value)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);
	int num = 0;

	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			var = D_RO(var)->next) {
		if (D_RO(var)->key == key)
			return 1;
		num++;
	}

	int ret = 0;
	TX_BEGIN(pop) {

		// BUG //
			
		TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		//TX_ADD_FIELD(buckets, bucket[h]);
		TX_ADD_FIELD(hashmap, count);
		TX_ADD_FIELD(hashmap, count);
		
		TOID(struct entry) e = TX_NEW(struct entry);
			
		// BUG //
		
		PMTest_assign(&(D_RW(e)->key), sizeof(struct entry));
		(D_RW(e)->key) = (key);
		(D_RW(e)->value) = (value);
		(D_RW(e)->next) = (D_RO(buckets)->bucket[h]);
		/*
		PM_EQU((D_RW(e)->key), (key));
		PM_EQU((D_RW(e)->value), (value));
		PM_EQU((D_RW(e)->next), (D_RO(buckets)->bucket[h]));
		*/
		PM_EQU((D_RW(buckets)->bucket[h]), (e));
		PM_EQU((D_RW(hashmap)->count), (D_RW(hashmap)->count+1));
		num++;
	} TX_ONABORT {
		fprintf(stderr, "transaction aborted: %s\n",
			pmemobj_errormsg());
		ret = -1;
	} TX_END

	if (ret)
		return ret;

	if (num > MAX_HASHSET_THRESHOLD ||
			(num > MIN_HASHSET_THRESHOLD &&
			D_RO(hashmap)->count > 2 * D_RO(buckets)->nbuckets))
		hm_tx_rebuild(pop, hashmap, D_RO(buckets)->nbuckets * 2);

	return 0;
}
'''.
