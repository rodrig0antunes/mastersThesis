
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

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * ctree_map_remove -- removes key-value pair from the map
 */
PMEMoid
ctree_map_remove(PMEMobjpool *pop, TOID(struct ctree_map) map, uint64_t key)
{
	struct tree_map_entry *parent = NULL;
	struct tree_map_entry *leaf = ctree_map_get_leaf(map, key, &parent);
	if (leaf == NULL)
		return OID_NULL;

	PMEMoid ret = leaf->slot;

	if (parent == NULL) { /* root */
		TX_BEGIN(pop) {
			pmemobj_tx_add_range_direct(leaf, sizeof(*leaf));
			PM_EQU(leaf->key, 0);
			PM_EQU(leaf->slot, OID_NULL);
		} TX_END
	} else {
		/*
		 * In this situation:
		 *	 parent
		 *	/     \
		 *   LEFT   RIGHT
		 * there's no point in leaving the parent internal node
		 * so it's swapped with the remaining node and then also freed.
		 */
		TX_BEGIN(pop) {
				
		// BUG //

			struct tree_map_entry *dest = parent;
			TOID(struct tree_map_node) node;
			TOID_ASSIGN(node, parent->slot);
			// pmemobj_tx_add_range_direct(dest, sizeof(*dest));
			PM_EQU(*dest, D_RW(node)->entries[
				D_RO(node)->entries[0].key == leaf->key]);
		
		// BUG //

			TX_FREE(node);
		} TX_END
	}

	return ret;
}
'''.
