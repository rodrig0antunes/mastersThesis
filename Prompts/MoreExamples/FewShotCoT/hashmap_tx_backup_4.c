
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

## Example 1

''' C

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {

		// BUG //

		*map = TX_ZNEW(struct hashmap_tx);

		// BUG //

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

===== assistant =====

## Correction 1

''' C

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {

		// BUG //

		TX_ADD_DIRECT(map);
		*map = TX_ZNEW(struct hashmap_tx);

		// BUG //
		
		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

## Explanation
In the example, the bug to repair is located in the area of the code delimited by the two '// BUG //' expressions. 
In the correction of the example, the bug fix is put in the area of the code delimited by the two '// BUG //' expressions.
In the correction, a new line 'TX_ADD_DIRECT(map);' is added before allocating memory for the new struct 'hashmap_tx'. 
This line tells the transaction system that it is intended to modify the 'map' pointer during the transaction. It essentially registers 'map' as a part of the transaction, making sure that if the transaction aborts, the changes to 'map' are rolled back as well.
By adding 'TX_ADD_DIRECT(map);', the transaction system ensures that the assignment of the new struct 'hashmap_tx' to 'map' is properly recorded. 
If the transaction is successful, this change will be committed, and if the transaction aborts, it will be rolled back, maintaining the consistency of your persistent memory data structures.

===== user =====

## Example 2

''' C

void write_hello_string (char *buf, char *path)
{
	PMEMobjpool *pop;
	
    pop = pmemobj_create(path, LAYOUT, PMEMOBJ_MIN_POOL, 0666);
	
    if (pop == NULL) 
	{
		perror(path);
		exit(1);
	}

	PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);
    
	// BUG //

    rootp->len = strlen(buf);
    
	// BUG //

    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

===== assistant =====

## Correction 2

''' C

void write_hello_string (char *buf, char *path)
{
	PMEMobjpool *pop;
	
    pop = pmemobj_create(path, LAYOUT, PMEMOBJ_MIN_POOL, 0666);
	
    if (pop == NULL) 
	{
		perror(path);
		exit(1);
	}

	PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);
    
	// BUG //

    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
	// BUG //

    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

## Explanation
In the example, the bug to repair is located in the area of the code delimited by the two '// BUG //' expressions. 
In the correction of the example, the bug fix is put in the area of the code delimited by the two '// BUG //' expressions.
In the correction, after the update 'rootp->len', it needs to be included 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' to ensure that the value of 'rootp->len' is durably stored in the persistent memory pool. 
By adding 'pmemobj_persist(pop, &rootp->len, sizeof(rootp->len));', it is guaranteed that the assignment to 'rootp->len' is properly preserved, and the data remains consistent. This correction ensures data durability in persistent memory.


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
