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