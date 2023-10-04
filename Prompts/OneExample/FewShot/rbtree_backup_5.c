
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
 * rbtree_map_repair_branch -- (internal) restores red-black tree in one branch
 */
static TOID(struct tree_map_node)
rbtree_map_repair_branch(TOID(struct rbtree_map) map,
	TOID(struct tree_map_node) n, enum rb_children c)
{

		// BUG //

	TOID(struct tree_map_node) sb = NODE_PARENT_AT(n, !c); /* sibling */
	if (D_RO(sb)->color == COLOR_RED) {
		PM_EQU(D_RW(sb)->color, COLOR_BLACK);
		TX_SET(NODE_P(n), color, COLOR_RED);
		rbtree_map_rotate(map, NODE_P(n), c);
		sb = NODE_PARENT_AT(n, !c);
		
		// BUG //
		
	}

	if (D_RO(D_RO(sb)->slots[RB_RIGHT])->color == COLOR_BLACK &&
		D_RO(D_RO(sb)->slots[RB_LEFT])->color == COLOR_BLACK) {
		TX_SET(sb, color, COLOR_RED);
		return D_RO(n)->parent;
	} else {
		if (D_RO(D_RO(sb)->slots[!c])->color == COLOR_BLACK) {
			TX_SET(D_RW(sb)->slots[c], color, COLOR_BLACK);
			TX_SET(sb, color, COLOR_RED);
			rbtree_map_rotate(map, sb, !c);
			sb = NODE_PARENT_AT(n, !c);
		}
		TX_SET(sb, color, D_RO(NODE_P(n))->color);
		TX_SET(NODE_P(n), color, COLOR_BLACK);
		TX_SET(D_RW(sb)->slots[!c], color, COLOR_BLACK);
		rbtree_map_rotate(map, NODE_P(n), c);

		return RB_FIRST(map);
	}

	return n;
}
'''.
