
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

## Example 1
''' C

PMEMoid retoid = D_RO(var)->value;
	TX_BEGIN(pop) {
		if (TOID_IS_NULL(prev))
			TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		else
            // TX_ADD_FIELD(prev, next);
		TX_ADD_FIELD(hashmap, count);

		if (TOID_IS_NULL(prev))
			D_RW(buckets)->bucket[h] = D_RO(var)->next;
		else
			D_RW(prev)->next = D_RO(var)->next;
            TX_ADD_FIELD(prev, next);
		D_RW(hashmap)->count--;
		TX_FREE(var);
    }

'''.

===== assistant =====

## Correction 1
''' C

PMEMoid retoid = D_RO(var)->value;
	TX_BEGIN(pop) {
		if (TOID_IS_NULL(prev))
			TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		else
			TX_ADD_FIELD(prev, next);
		TX_ADD_FIELD(hashmap, count);

		if (TOID_IS_NULL(prev))
			D_RW(buckets)->bucket[h] = D_RO(var)->next;
		else
			D_RW(prev)->next = D_RO(var)->next;
		D_RW(hashmap)->count--;
		TX_FREE(var);
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
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    rootp->len = strlen(buf);

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

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * btree_map_find_dest_node -- (internal) finds a place to insert the new key at
 */
static TOID(struct tree_map_node)
btree_map_find_dest_node(TOID(struct btree_map) map,
	TOID(struct tree_map_node) n, TOID(struct tree_map_node) parent,
	uint64_t key, int *p)
{
	if (D_RO(n)->n == BTREE_ORDER - 1) { /* node is full, perform a split */
		struct tree_map_node_item m;
		TOID(struct tree_map_node) right =
			btree_map_create_split_node(n, &m);

		if (!TOID_IS_NULL(parent)) {
			btree_map_insert_node(parent, *p, m, n, right);
			if (key > m.key) /* select node to continue search */
				n = right;
		} else { /* replacing root node, the tree grows in height */
			TOID(struct tree_map_node) up =
				TX_ZNEW(struct tree_map_node);
			PM_EQU(D_RW(up)->n, 1);
			PM_EQU(D_RW(up)->items[0], m);
			PM_EQU(D_RW(up)->slots[0], n);
			PM_EQU(D_RW(up)->slots[1], right);
			
		// BUG //

			PM_EQU(D_RW(map)->root, up);
			TX_ADD_FIELD(map, root);
			
			n = up;
			
		// BUG //

		}
	}

	int i;
	for (i = 0; i < BTREE_ORDER - 1; ++i) {
		*p = i;

		/*
		 * The key either fits somewhere in the middle or at the
		 * right edge of the node.
		 */
		if (D_RO(n)->n == i || D_RO(n)->items[i].key > key) {
			return TOID_IS_NULL(D_RO(n)->slots[i]) ? n :
				btree_map_find_dest_node(map,
					D_RO(n)->slots[i], n, key, p);
		}
	}

	/*
	 * The key is bigger than the last node element, go one level deeper
	 * in the rightmost child.
	 */
	return btree_map_find_dest_node(map, D_RO(n)->slots[i], n, key, p);
}
'''.
