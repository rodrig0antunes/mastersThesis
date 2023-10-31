
===== system =====

You are a helpful programming assistant and an expert in the development of Persistent Memory programs. You are helping a user repair the bugs inside a Persistent Memory program. 
The user has written a program in C programming language while using the PMDK library libpmemobj. However, the program has some bugs and is not working as expected. 
The user has analysed the program with a bug detection tool that has located the bug or bugs. You will use this information to generate a corrected version of the program.
The bug or bugs to repair will be located in an area of the code delimited by an expression. The beggining and end of the area of the code where a bug is and where the fix is 
supposed to go will be delimited by the expression '// BUG //'.
When presenting the correction, present the whole code and not just the corrected segment of the code.
Put the whole corrected program within code delimiters, as follows:
                ''' C
                # YOUR CODE HERE
                '''.

===== user =====

### EXAMPLES

## Example 1

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

## Correction 1

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

===== user =====

## Example 2

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

## Correction 2

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

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM

''' C
/*
 * btree_map_create_split_node -- (internal) splits a node into two
 */
static TOID(struct tree_map_node)
btree_map_create_split_node(TOID(struct tree_map_node) node,
	struct tree_map_node_item *m)
{

		// BUG //

	TOID(struct tree_map_node) right = TX_ZNEW(struct tree_map_node);
	
	
	int c = (BTREE_ORDER / 2);
	*m = D_RO(node)->items[c - 1]; /* select median item */
		
		// BUG //

	PM_EQU(D_RW(node)->items[c - 1], EMPTY_ITEM);

	/* move everything right side of median to the new node */
	for (int i = c; i < BTREE_ORDER; ++i) {
		if (i != BTREE_ORDER - 1) {
			PM_EQU(D_RW(right)->items[D_RW(right)->n],
				D_RO(node)->items[i]);
			PM_EQU(D_RW(right)->n, D_RO(right)->n + 1);
			PM_EQU(D_RW(node)->items[i], EMPTY_ITEM);
		}
		PM_EQU(D_RW(right)->slots[i - c], D_RO(node)->slots[i]);
		PM_EQU(D_RW(node)->slots[i], TOID_NULL(struct tree_map_node));
	}
	PM_EQU(D_RW(node)->n, c - 1);

	return right;
}
'''.
