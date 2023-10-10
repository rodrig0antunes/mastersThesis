
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

## Example 2
''' C

void write_hello_string (char *buf, char *path)
{
	PMEMobjpool *pop;
	
    pop = pmemobj_create(path, LAYOUT, PMEMOBJ_MIN_POOL, 0666);
	
    if (pop == NULL) 
	{
		perror(path);
		exit(1);
	}

	PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);
    
    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

===== assistant =====

## Correction 2
''' C

void write_hello_string (char *buf, char *path)
{
	PMEMobjpool *pop;
	
    pop = pmemobj_create(path, LAYOUT, PMEMOBJ_MIN_POOL, 0666);
	
    if (pop == NULL) 
	{
		perror(path);
		exit(1);
	}

	PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);
    
    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

## Explanation
In the correction, one of the 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' calls has been removed because it is redundant.
By removing the redundant 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' call, the unnecessary overhead is eliminated, while still maintaining the correct behaviour of persisting the 'len' field, ensuring that this specific data is correctly persisted in the persistent memory pool.


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
	TOID(struct tree_map_node) s = D_RO(map)->sentinel;

	TX_ADD(node);
	TX_ADD(child);

	PM_EQU(D_RW(node)->slots[!c], D_RO(child)->slots[c]);

	if (!TOID_EQUALS(D_RO(child)->slots[c], s))
		TX_SET(D_RW(child)->slots[c], parent, node);
		
		// BUG#1 //

	PM_EQU(NODE_P(child), NODE_P(node));

	TX_ADD(NODE_P(node));
	PM_EQU(NODE_PARENT_AT(node, NODE_LOCATION(node)), child);


	PM_EQU(D_RW(child)->slots[c], node);
	PM_EQU(D_RW(node)->parent, child);
		
		// BUG#1 //

}

/*
 * rbtree_map_remove -- removes key-value pair from the map
 */
PMEMoid
rbtree_map_remove(PMEMobjpool *pop, TOID(struct rbtree_map) map, uint64_t key)
{
	PMEMoid ret = OID_NULL;

	TOID(struct tree_map_node) n = rbtree_map_find_node(map, key);
	if (TOID_IS_NULL(n))
		return ret;

	ret = D_RO(n)->value;

	TOID(struct tree_map_node) s = D_RO(map)->sentinel;
	TOID(struct tree_map_node) r = D_RO(map)->root;

	TOID(struct tree_map_node) y = (NODE_IS_NULL(D_RO(n)->slots[RB_LEFT]) ||
					NODE_IS_NULL(D_RO(n)->slots[RB_RIGHT]))
					? n : rbtree_map_successor(map, n);

	TOID(struct tree_map_node) x = NODE_IS_NULL(D_RO(y)->slots[RB_LEFT]) ?
			D_RO(y)->slots[RB_RIGHT] : D_RO(y)->slots[RB_LEFT];

	TX_BEGIN(pop) {
		TX_SET(x, parent, NODE_P(y));
			
		// BUG#2 //

		if (TOID_EQUALS(NODE_P(x), r)) {
			TX_SET(r, slots[RB_LEFT], x);
		} else {
			TX_ADD(y);
			PM_EQU(NODE_PARENT_AT(y, NODE_LOCATION(y)), x);
				
		// BUG#2 //
		
		}

		if (D_RO(y)->color == COLOR_BLACK)
			rbtree_map_repair(map, x);

		if (!TOID_EQUALS(y, n)) {
			TX_ADD(y);
			PM_EQU(D_RW(y)->slots[RB_LEFT], D_RO(n)->slots[RB_LEFT]);
			PM_EQU(D_RW(y)->slots[RB_RIGHT], D_RO(n)->slots[RB_RIGHT]);
			PM_EQU(D_RW(y)->parent, D_RO(n)->parent);
			PM_EQU(D_RW(y)->color, D_RO(n)->color);
			TX_SET(D_RW(n)->slots[RB_LEFT], parent, y);
			TX_SET(D_RW(n)->slots[RB_RIGHT], parent, y);
			
			// BUG#3 //

			TX_ADD(NODE_P(n));
			PM_EQU(NODE_PARENT_AT(n, NODE_LOCATION(n)), y);
				
			// BUG#3 //
		
		}
		TX_FREE(n);
	} TX_END

	return ret;
}
'''.
