
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

## Example

''' C

int main(int argc, char *argv[]) {

    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, POBJ_LAYOUT_NAME(example), PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid root_oid = pmemobj_root(pop, sizeof(struct root));
    struct root *root_ptr = pmemobj_direct(root_oid);

    TX_BEGIN(pop) {

        // BUG //

        PM_EQU(D_RW(root_ptr->value, 42));

        // BUG //

    } TX_ONABORT {
        perror("Transaction aborted");
    } TX_END

    pmemobj_close(pop);

    return 0;
}

'''.

===== assistant =====

## Correction

''' C

int main(int argc, char *argv[]) {
    
    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, POBJ_LAYOUT_NAME(example), PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid root_oid = pmemobj_root(pop, sizeof(struct root));
    struct root *root_ptr = pmemobj_direct(root_oid);

    TX_BEGIN(pop) {
        
        // BUG //

        TX_SET(root_ptr->value, 42);

        // BUG //

    } TX_ONABORT {
        perror("Transaction aborted");
    } TX_END


    pmemobj_close(pop);

    return 0;
}

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
	
	TX_ADD(node);
	TX_ADD(child);

	PM_EQU(D_RW(node)->slots[!c], D_RO(child)->slots[c]);

	if (!TOID_EQUALS(D_RO(child)->slots[c], s))
		TX_SET(D_RW(child)->slots[c], parent, node);
		
		// BUG //

	PM_EQU(NODE_P(child), NODE_P(node));

	PM_EQU(D_RW(NODE_P(node))->slots[NODE_LOCATION(node)], child);

	PM_EQU(D_RW(child)->slots[c], node);
	PM_EQU(D_RW(node)->parent, child);
		
		// BUG //
		
}
'''.
