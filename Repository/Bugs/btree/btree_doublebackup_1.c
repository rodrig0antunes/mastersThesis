/*
 * btree_map_insert_node -- (internal) inserts and makes space for new node
 */
static void
btree_map_insert_node(TOID(struct tree_map_node) node, int p,
	struct tree_map_node_item item,
	TOID(struct tree_map_node) left, TOID(struct tree_map_node) right)
{
	TX_ADD(node);
	if (D_RO(node)->items[p].key != 0) { /* move all existing data */
		PM_MEMMOVE(&D_RW(node)->items[p + 1], &D_RW(node)->items[p],
		sizeof(struct tree_map_node_item) * ((BTREE_ORDER - 2 - p)));
			// BUG //
	
		PM_MEMMOVE(&D_RW(node)->slots[p + 1], &D_RW(node)->slots[p],
		sizeof(TOID(struct tree_map_node)) * ((BTREE_ORDER - 1 - p)));
	}
	TX_ADD(node);
	PM_EQU(D_RW(node)->slots[p], left);
	PM_EQU(D_RW(node)->slots[p + 1], right);
	btree_map_insert_item_at(node, p, item);

			// BUG //

}