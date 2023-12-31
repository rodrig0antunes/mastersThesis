/*
 * ctree_map_remove -- removes key-value pair from the map
 */
PMEMoid
ctree_map_remove(PMEMobjpool *pop, TOID(struct ctree_map) map, uint64_t key)
{
	struct tree_map_entry *parent = NULL;
	struct tree_map_entry *leaf = ctree_map_get_leaf(map, key, &parent);
	if (leaf == NULL)
		return OID_NULL;

	PMEMoid ret = leaf->slot;

	if (parent == NULL) { /* root */
		
		// BUG //
		
		TX_BEGIN(pop) {
			pmemobj_tx_add_range_direct(leaf, sizeof(*leaf));
			PM_EQU(leaf->key, 0);
		} TX_END
		PM_EQU(leaf->slot, OID_NULL);
		
		// BUG //

	} else {
		/*
		 * In this situation:
		 *	 parent
		 *	/     \
		 *   LEFT   RIGHT
		 * there's no point in leaving the parent internal node
		 * so it's swapped with the remaining node and then also freed.
		 */
		TX_BEGIN(pop) {
			struct tree_map_entry *dest = parent;
			TOID(struct tree_map_node) node;
			TOID_ASSIGN(node, parent->slot);
			pmemobj_tx_add_range_direct(dest, sizeof(*dest));
			PM_EQU(*dest, D_RW(node)->entries[
				D_RO(node)->entries[0].key == leaf->key]);

			TX_FREE(node);
		} TX_END
	}

	return ret;
}