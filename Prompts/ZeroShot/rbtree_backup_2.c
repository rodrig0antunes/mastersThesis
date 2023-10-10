
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
 * rbtree_map_rotate -- (internal) performs a left/right rotation around a node
 */
static void
rbtree_map_rotate(TOID(struct rbtree_map) map,
	TOID(struct tree_map_node) node, enum rb_children c)
{
	TOID(struct tree_map_node) child = D_RO(node)->slots[!c];
	TOID(struct tree_map_node) s = D_RO(map)->sentinel;
		
		// BUG //

	TX_ADD(node);

	
	PM_EQU(D_RW(node)->slots[!c], D_RO(child)->slots[c]);
		
		// BUG //

	if (!TOID_EQUALS(D_RO(child)->slots[c], s))
		TX_SET(D_RW(child)->slots[c], parent, node);
	
	PM_EQU(NODE_P(child), NODE_P(node));

	TX_SET(NODE_P(node), slots[NODE_LOCATION(node)], child);

	PM_EQU(D_RW(child)->slots[c], node);
	PM_EQU(D_RW(node)->parent, child);
}
'''.
    
