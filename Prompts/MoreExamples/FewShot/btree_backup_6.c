
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

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {

		// BUG //

		*map = TX_ZNEW(struct hashmap_tx);

		// BUG //

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

===== assistant =====

## Correction 1

''' C

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {

		// BUG //

		TX_ADD_DIRECT(map);
		*map = TX_ZNEW(struct hashmap_tx);

		// BUG //
		
		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

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
    
	// BUG //

    rootp->len = strlen(buf);
    
	// BUG //

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
    
	// BUG //

    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
	// BUG //

    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
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
