
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
    
