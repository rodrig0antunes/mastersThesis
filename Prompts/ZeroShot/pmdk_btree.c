
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
 * btree_map_create_split_node -- (internal) splits a node into two
 */
static TOID(struct tree_map_node)
btree_map_create_split_node(TOID(struct tree_map_node) node,
	struct tree_map_node_item *m)
{

		// BUG //

	TOID(struct tree_map_node) right = TX_ZNEW(struct tree_map_node);
	
	
	int c = (BTREE_ORDER / 2);
	*m = D_RO(node)->items[c - 1]; /* select median item */
		
		// BUG //

	PM_EQU(D_RW(node)->items[c - 1], EMPTY_ITEM);

	/* move everything right side of median to the new node */
	for (int i = c; i < BTREE_ORDER; ++i) {
		if (i != BTREE_ORDER - 1) {
			PM_EQU(D_RW(right)->items[D_RW(right)->n],
				D_RO(node)->items[i]);
			PM_EQU(D_RW(right)->n, D_RO(right)->n + 1);
			PM_EQU(D_RW(node)->items[i], EMPTY_ITEM);
		}
		PM_EQU(D_RW(right)->slots[i - c], D_RO(node)->slots[i]);
		PM_EQU(D_RW(node)->slots[i], TOID_NULL(struct tree_map_node));
	}
	PM_EQU(D_RW(node)->n, c - 1);

	return right;
}
'''.
    
