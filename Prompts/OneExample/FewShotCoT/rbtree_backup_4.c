
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

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {
		*map = TX_ZNEW(struct hashmap_tx);

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

===== assistant =====

## Correction
''' C

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {
		TX_ADD_DIRECT(map);
		*map = TX_ZNEW(struct hashmap_tx);

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

## Explanation
In the correction, a new line 'TX_ADD_DIRECT(map);' is added before allocating memory for the new struct 'hashmap_tx'. 
This line tells the transaction system that it is intended to modify the 'map' pointer during the transaction. It essentially registers 'map' as a part of the transaction, making sure that if the transaction aborts, the changes to 'map' are rolled back as well.
By adding 'TX_ADD_DIRECT(map);', the transaction system ensures that the assignment of the new struct 'hashmap_tx' to 'map' is properly recorded. 
If the transaction is successful, this change will be committed, and if the transaction aborts, it will be rolled back, maintaining the consistency of your persistent memory data structures.

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * rbtree_map_insert_bst -- (internal) inserts a node in regular BST fashion
 */
static void
rbtree_map_insert_bst(TOID(struct rbtree_map) map, TOID(struct tree_map_node) n)
{
	TOID(struct tree_map_node) parent = D_RO(map)->root;
	TOID(struct tree_map_node) *dst = &RB_FIRST(map);
	TOID(struct tree_map_node) s = D_RO(map)->sentinel;

	PM_EQU(D_RW(n)->slots[RB_LEFT], s);
	PM_EQU(D_RW(n)->slots[RB_RIGHT], s);

	while (!NODE_IS_NULL(*dst)) {
		parent = *dst;
		dst = &D_RW(*dst)->slots[D_RO(n)->key > D_RO(*dst)->key];
	}
	
		// BUG //

	TX_SET(n, parent, parent);

	// pmemobj_tx_add_range_direct(dst, sizeof(*dst));
	PM_EQU(*dst, n);

		// BUG //

}
'''.
