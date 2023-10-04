
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
 * btree_map_remove_from_node -- (internal) removes element from node
 */
static void
btree_map_remove_from_node(TOID(struct btree_map) map,
	TOID(struct tree_map_node) node,
	TOID(struct tree_map_node) parent, int p)
{

		// BUG //
		
	if (TOID_IS_NULL(D_RO(node)->slots[0])) { /* leaf */
		// TX_ADD(node);
		if (D_RO(node)->n == 1 || p == BTREE_ORDER - 2)
			PM_EQU(D_RW(node)->items[p], EMPTY_ITEM);
		
		// BUG //

		else if (D_RO(node)->n != 1) {
			PM_MEMMOVE(&D_RW(node)->items[p],
				&D_RW(node)->items[p + 1],
				sizeof(struct tree_map_node_item) *
				(D_RO(node)->n - p));
		}

		PM_EQU(D_RW(node)->n, D_RO(node)->n - 1);
		return;
	}

	/* can't delete from non-leaf nodes, remove successor */
	TOID(struct tree_map_node) rchild = D_RW(node)->slots[p + 1];
	TOID(struct tree_map_node) lp = node;
	TOID(struct tree_map_node) lm =
		btree_map_get_leftmost_leaf(map, rchild, &lp);

	TX_ADD_FIELD(node, items[p]);
	PM_EQU(D_RW(node)->items[p], D_RO(lm)->items[0]);

	btree_map_remove_from_node(map, lm, lp, 0);

	if (D_RO(lm)->n < BTREE_MIN) /* right child can be deficient now */
		btree_map_rebalance(map, lm, lp,
			TOID_EQUALS(lp, node) ? p + 1 : 0);
}
'''.
