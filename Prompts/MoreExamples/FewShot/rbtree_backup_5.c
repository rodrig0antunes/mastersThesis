
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
