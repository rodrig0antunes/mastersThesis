/*
 * btree_map_merge -- (internal) merges node and right sibling
 */
static void
btree_map_merge(TOID(struct btree_map) map, TOID(struct tree_map_node) rn,
	TOID(struct tree_map_node) node,
	TOID(struct tree_map_node) parent, int p)
{
	struct tree_map_node_item sep = D_RO(parent)->items[p];

	TX_ADD(node);
	/* add separator to the deficient node */
	PM_EQU(D_RW(node)->items[D_RW(node)->n], sep);
	PM_EQU(D_RW(node)->n, D_RO(node)->n + 1);
	
	/* copy right sibling data to node */
	PM_MEMCPY(&D_RW(node)->items[D_RO(node)->n], D_RO(rn)->items,
	sizeof(struct tree_map_node_item) * D_RO(rn)->n);
	PM_MEMCPY(&D_RW(node)->slots[D_RO(node)->n], D_RO(rn)->slots,
	sizeof(TOID(struct tree_map_node)) * (D_RO(rn)->n + 1));

	PM_EQU(D_RW(node)->n, D_RO(node)->n + D_RO(rn)->n);

		// BUG //

	TX_FREE(rn); /* right node is now empty */

	// TX_ADD(parent);
	PM_EQU(D_RW(parent)->n, D_RO(parent)->n - 1);
		
		// BUG //
		
	/* move everything to the right of the separator by one array slot */
	PM_MEMMOVE(D_RW(parent)->items + p, D_RW(parent)->items + p + 1,
	sizeof(struct tree_map_node_item) * (D_RO(parent)->n - p));

	PM_MEMMOVE(D_RW(parent)->slots + p + 1, D_RW(parent)->slots + p + 2,
	sizeof(TOID(struct tree_map_node)) * (D_RO(parent)->n - p + 1));

	/* if the parent is empty then the tree shrinks in height */
	if (D_RO(parent)->n == 0 && TOID_EQUALS(parent, D_RO(map)->root)) {
		TX_ADD(map);
		TX_FREE(D_RO(map)->root);
		PM_EQU(D_RW(map)->root, node);
	}
}