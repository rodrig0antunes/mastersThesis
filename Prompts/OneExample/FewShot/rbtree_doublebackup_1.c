
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

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * rbtree_map_rotate -- (internal) performs a left/right rotation around a node
 */
static void
rbtree_map_rotate(TOID(struct rbtree_map) map,
	TOID(struct tree_map_node) node, enum rb_children c)
{
	TOID(struct tree_map_node) child = D_RO(node)->slots[!c];
		
		// BUG //

	TOID(struct tree_map_node) s = D_RO(map)->sentinel;

	TX_ADD(node);
	TX_ADD(node);
	TX_ADD(child);

	PM_EQU(D_RW(node)->slots[!c], D_RO(child)->slots[c]);
		
		// BUG //

	if (!TOID_EQUALS(D_RO(child)->slots[c], s))
		TX_SET(D_RW(child)->slots[c], parent, node);
	
	PM_EQU(NODE_P(child), NODE_P(node));

	TX_SET(NODE_P(node), slots[NODE_LOCATION(node)], child);

	PM_EQU(D_RW(child)->slots[c], node);
	PM_EQU(D_RW(node)->parent, child);
}
'''.
