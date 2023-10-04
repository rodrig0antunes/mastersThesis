
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
 * btree_map_rotate_right -- (internal) takes one element from right sibling
 */
static void
btree_map_rotate_right(TOID(struct tree_map_node) rsb,
	TOID(struct tree_map_node) node,
	TOID(struct tree_map_node) parent, int p)
{
	/* move the separator from parent to the deficient node */
	struct tree_map_node_item sep = D_RO(parent)->items[p];
	btree_map_insert_item(node, D_RO(node)->n, sep);
		
		// BUG //
		
	/* the first element of the right sibling is the new separator */
	// TX_ADD_FIELD(parent, items[p]);
	PM_EQU(D_RW(parent)->items[p], D_RO(rsb)->items[0]);
		
		// BUG //

	/* the nodes are not necessarily leafs, so copy also the slot */
	TX_ADD_FIELD(node, slots[D_RO(node)->n]);
	PM_EQU(D_RW(node)->slots[D_RO(node)->n], D_RO(rsb)->slots[0]);

	TX_ADD(rsb);
	PM_EQU(D_RW(rsb)->n, D_RO(rsb)->n - 1); /* it loses one element, but still > min */

	/* move all existing elements back by one array slot */
	PM_MEMMOVE(D_RW(rsb)->items, D_RO(rsb)->items + 1,
		sizeof(struct tree_map_node_item) * (D_RO(rsb)->n));
	PM_MEMMOVE(D_RW(rsb)->slots, D_RO(rsb)->slots + 1,
		sizeof(TOID(struct tree_map_node)) * (D_RO(rsb)->n + 1));
}
'''.
