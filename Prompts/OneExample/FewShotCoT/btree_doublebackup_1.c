
===== system =====

You are a helpful programming assistant and an expert in the development of Persistent Memory programs. You are helping a user repair the errors inside a Persistent Memory program. 
The user has written a program in the programming language C and the PMDK library libpmemobj. However, the program has some errors and is not working as expected. 
The user has analysed the program with a bug detection tool and will provide you with a textual explanation of where the error is.  
You will use this information to generate a corrected version of the program.
In order to help locate the bug to repair an expression that signals the interval where the bug is will be provided. The beggining and end of the area of the code where 
the fix is supposed to go will be delimited by the exprexion '// BUG //'.
Put your corrected program within code delimiters, as follows:
                ''' C
                # YOUR CODE HERE
                '''.

===== user =====

### EXAMPLES

## Example
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

		D_RW(hashmap)->buckets = TX_ZALLOC(struct buckets, sz);
		D_RW(D_RW(hashmap)->buckets)->nbuckets = len;
        TX_ADD(hashmap);
	} TX_ONABORT {
		fprintf(stderr, "%s: transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
	} TX_END
}

'''.

===== assistant =====

## Correction
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

		D_RW(hashmap)->buckets = TX_ZALLOC(struct buckets, sz);
		D_RW(D_RW(hashmap)->buckets)->nbuckets = len;
	} TX_ONABORT {
		fprintf(stderr, "%s: transaction aborted: %s\n", __func__,
			pmemobj_errormsg());
		abort();
	} TX_END
}

'''.

## Explanation
In the correction, one of the 'TX_ADD(hashmap);' calls has been removed. The 'TX_ADD(hashmap);' call is only made once at the beginning of the transaction block,
which is the correct way to indicate that 'hashmap' is part of the transaction.
By removing the redundant 'TX_ADD(hashmap);', it is removed the unnecessary duplication of statements. This change simplifies the code while preserving the correct transactional behaviour.

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * btree_map_insert_node -- (internal) inserts and makes space for new node
 */
static void
btree_map_insert_node(TOID(struct tree_map_node) node, int p,
	struct tree_map_node_item item,
	TOID(struct tree_map_node) left, TOID(struct tree_map_node) right)
{
	TX_ADD(node);
	if (D_RO(node)->items[p].key != 0) { /* move all existing data */
		PM_MEMMOVE(&D_RW(node)->items[p + 1], &D_RW(node)->items[p],
		sizeof(struct tree_map_node_item) * ((BTREE_ORDER - 2 - p)));
			// BUG //
	
		PM_MEMMOVE(&D_RW(node)->slots[p + 1], &D_RW(node)->slots[p],
		sizeof(TOID(struct tree_map_node)) * ((BTREE_ORDER - 1 - p)));
	}
	TX_ADD(node);
	PM_EQU(D_RW(node)->slots[p], left);
	PM_EQU(D_RW(node)->slots[p + 1], right);
	btree_map_insert_item_at(node, p, item);

			// BUG //

}
'''.
