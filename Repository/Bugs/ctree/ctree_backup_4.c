/*
 * ctree_map_insert_leaf -- (internal) inserts a new leaf at the position
 */
static void
ctree_map_insert_leaf(struct tree_map_entry *p,
	struct tree_map_entry e, int diff)
{
	TOID(struct tree_map_node) new_node = TX_NEW(struct tree_map_node);
	//isPersistent(&(D_RW(new_node)->diff), sizeof(D_RW(new_node)->diff));
	PM_EQU(D_RW(new_node)->diff, diff);

	int d = BIT_IS_SET(e.key, D_RO(new_node)->diff);

	/* insert the leaf at the direction based on the critical bit */
	D_RW(new_node)->entries[d] = e;

	/* find the appropriate position in the tree to insert the node */
	TOID(struct tree_map_node) node;
	while (OID_INSTANCEOF(p->slot, struct tree_map_node)) {
		TOID_ASSIGN(node, p->slot);

		/* the critical bits have to be sorted */
		if (D_RO(node)->diff < D_RO(new_node)->diff)
			break;

		p = &D_RW(node)->entries[BIT_IS_SET(e.key, D_RO(node)->diff)];
	}

		// BUG //

	/* insert the found destination in the other slot */
	PM_EQU(D_RW(new_node)->entries[!d], *p);
	
	
	PM_EQU(p->key, 0);
	PM_EQU(p->slot, new_node.oid);

	pmemobj_tx_add_range_direct(p, sizeof(*p));
	
		// BUG // 
}