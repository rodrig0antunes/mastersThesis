
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
 * ctree_map_insert -- inserts a new key-value pair into the map
 */
int
ctree_map_insert(PMEMobjpool *pop, TOID(struct ctree_map) map,
	uint64_t key, PMEMoid value)
{
	struct tree_map_entry *p = &D_RW(map)->root;
	int ret = 0;

	/* descend the path until a best matching key is found */
	TOID(struct tree_map_node) node;
	while (!OID_IS_NULL(p->slot) &&
		OID_INSTANCEOF(p->slot, struct tree_map_node)) {
		TOID_ASSIGN(node, p->slot);
		p = &D_RW(node)->entries[BIT_IS_SET(key, D_RW(node)->diff)];
	}

	struct tree_map_entry e = {key, value};
	TX_BEGIN(pop) {
		
		// BUG //

		if (p->key == 0 || p->key == key) {
			PM_EQU(*p, e);
			pmemobj_tx_add_range_direct(p, sizeof(*p));
		} else {
			ctree_map_insert_leaf(&D_RW(map)->root, e,
					find_crit_bit(p->key, key));
		
		// BUG //

		}
	} TX_ONABORT {
		ret = 1;
	} TX_END

	// not needed for now
	// printf("ret = %d\n", ret);
	return ret;
}
'''.
