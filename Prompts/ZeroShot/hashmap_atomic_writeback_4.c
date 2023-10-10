
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
 * hm_atomic_insert -- inserts specified value into the hashmap,
 * returns:
 * - 0 if successful,
 * - 1 if value already existed,
 * - -1 if something bad happened
 */
int
hm_atomic_insert(PMEMobjpool *pop, TOID(struct hashmap_atomic) hashmap,
		uint64_t key, PMEMoid value)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);
	int num = 0;

	POBJ_LIST_FOREACH(var, &D_RO(buckets)->bucket[h], list) {
		if (D_RO(var)->key == key)
			return 1;
		num++;
	}

	PM_EQU(D_RW(hashmap)->count_dirty, 1);
	pmemobj_persist(pop, &D_RW(hashmap)->count_dirty,
			sizeof(D_RW(hashmap)->count_dirty));

	struct entry_args args = {
		.key = key,
		.value = value,
	};
	PMEMoid oid = POBJ_LIST_INSERT_NEW_HEAD(pop,
			&D_RW(buckets)->bucket[h],
			list, sizeof(struct entry), create_entry, &args);
	if (OID_IS_NULL(oid)) {
		fprintf(stderr, "failed to allocate entry: %s\n",
			pmemobj_errormsg());
		return -1;
	}

		// BUG //

	PM_EQU(D_RW(hashmap)->count, (D_RW(hashmap)->count + 1));
		
		// BUG //
		
	PMTest_isPersistedBefore(&D_RW(hashmap)->count_dirty, sizeof(D_RW(hashmap)->count_dirty), &D_RW(hashmap)->count, sizeof(D_RW(hashmap)->count));


	PM_EQU(D_RW(hashmap)->count_dirty, 0);
	pmemobj_persist(pop, &D_RW(hashmap)->count_dirty,
			sizeof(D_RW(hashmap)->count_dirty));

	PMTest_isPersistedBefore(&D_RW(hashmap)->count, sizeof(D_RW(hashmap)->count), &D_RW(hashmap)->count_dirty, sizeof(D_RW(hashmap)->count_dirty));


	num++;
	if (num > MAX_HASHSET_THRESHOLD ||
			(num > MIN_HASHSET_THRESHOLD &&
			D_RO(hashmap)->count > 2 * D_RO(buckets)->nbuckets))
		hm_atomic_rebuild(pop, hashmap, D_RW(buckets)->nbuckets * 2);

	PMTest_isPersistent(&D_RW(hashmap)->count_dirty, sizeof(D_RW(hashmap)->count_dirty));
	PMTest_isPersistent(&D_RW(hashmap)->count, sizeof(D_RW(hashmap)->count));

	return 0;
}
'''.
    
