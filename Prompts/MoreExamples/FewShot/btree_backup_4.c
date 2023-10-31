
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

	/* the first element of the right sibling is the new separator */
	TX_ADD_FIELD(parent, items[p]);
	PM_EQU(D_RW(parent)->items[p], D_RO(rsb)->items[0]);

	/* the nodes are not necessarily leafs, so copy also the slot */
		
		// BUG //

	TX_ADD_FIELD(node, slots[D_RO(node)->n]);
	PM_EQU(D_RW(node)->slots[D_RO(node)->n], D_RO(rsb)->slots[0]);


	PM_EQU(D_RW(rsb)->n, D_RO(rsb)->n - 1); /* it loses one element, but still > min */
		
		// BUG //
		
	/* move all existing elements back by one array slot */
	PM_MEMMOVE(D_RW(rsb)->items, D_RO(rsb)->items + 1,
		sizeof(struct tree_map_node_item) * (D_RO(rsb)->n));
	PM_MEMMOVE(D_RW(rsb)->slots, D_RO(rsb)->slots + 1,
		sizeof(TOID(struct tree_map_node)) * (D_RO(rsb)->n + 1));
}
'''.
