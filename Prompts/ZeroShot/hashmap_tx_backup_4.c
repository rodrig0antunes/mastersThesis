
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
 * hm_tx_remove -- removes specified value from the hashmap,
 * returns:
 * - key's value if successful,
 * - OID_NULL if value didn't exist or if something bad happened
 */
PMEMoid
hm_tx_remove(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap, uint64_t key)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var, prev = TOID_NULL(struct entry);

	uint64_t h = hash(&hashmap, &buckets, key);
	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			prev = var, var = D_RO(var)->next) {
		if (D_RO(var)->key == key)
			break;
	}

	if (TOID_IS_NULL(var))
		return OID_NULL;

	TX_BEGIN(pop) {
		if (TOID_IS_NULL(prev))
		
		// BUG //
		
			TX_ADD_FIELD(D_RO(hashmap)->buckets, bucket[h]);
		else
			TX_ADD_FIELD(prev, next);
		
		// BUG //

		if (TOID_IS_NULL(prev))
			PM_EQU(D_RW(buckets)->bucket[h], D_RO(var)->next);
		else
			PM_EQU(D_RW(prev)->next, D_RO(var)->next);
		PM_EQU(D_RW(hashmap)->count, D_RO(hashmap)->count - 1);
		TX_FREE(var);
	} TX_ONABORT {
		fprintf(stderr, "transaction aborted: %s\n",
			pmemobj_errormsg());
		return OID_NULL;
	} TX_END

	if (D_RO(hashmap)->count < D_RO(buckets)->nbuckets)
		hm_tx_rebuild(pop, hashmap, D_RO(buckets)->nbuckets / 2);

	return D_RO(var)->value;
}
'''.
    
