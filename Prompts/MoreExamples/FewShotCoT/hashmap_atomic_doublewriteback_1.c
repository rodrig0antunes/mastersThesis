
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
    
    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

===== assistant =====

## Correction 1
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
    
    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

## Explanation
In the correction, one of the 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' calls has been removed because it is redundant.
By removing the redundant 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' call, the unnecessary overhead is eliminated, while still maintaining the correct behaviour of persisting the 'len' field, ensuring that this specific data is correctly persisted in the persistent memory pool.

===== user =====

## Example 2
''' C

int main(int argc, char *argv[]) {

    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, NULL, PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid value = pmemobj_tx_alloc(sizeof(int), 0);
    if (PMEMOID_IS_NULL(value)) {
        perror("pmemobj_tx_alloc");
        pmemobj_close(pop);
        return 1;
    }

    int *value_ptr = pmemobj_direct(value);

    if (*value_ptr == 42) {
        TX_ADD(value);
        TX_ADD(value);
    }

    pmemobj_close(pop);

    return 0;
}

'''.

===== assistant =====

## Correction 2
''' C
 
int main(int argc, char *argv[]) {
    
    const char *pmem_file = argv[1];
    PMEMobjpool *pop;

    if ((pop = pmemobj_create(pmem_file, NULL, PMEMOBJ_MIN_POOL, 0666)) == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid value = pmemobj_tx_alloc(sizeof(int), 0);
    if (PMEMOID_IS_NULL(value)) {
        perror("pmemobj_tx_alloc");
        pmemobj_close(pop);
        return 1;
    }

    int *value_ptr = pmemobj_direct(value);

    if (*value_ptr == 42) {
        TX_ADD(value);
    }

    pmemobj_close(pop);

    return 0;
}

'''.

## Explanation
In the correction, one of the 'TX_ADD(value);' calls has been removed because it is redundant.
The correction eliminates unnecessary overhead and maintains the correct behaviour intended in the code, including the value in the transaction if the condition is met, preserving the correct transactional behaviour.


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
		
		// BUG //
		
	PM_EQU(D_RW(hashmap)->count_dirty, 1);
	pmemobj_persist(pop, &D_RW(hashmap)->count_dirty,
			sizeof(D_RW(hashmap)->count_dirty));
	pmemobj_persist(pop, &D_RW(hashmap)->count_dirty,
			sizeof(D_RW(hashmap)->count_dirty));
		
		// BUG //

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

	PM_EQU(D_RW(hashmap)->count, (D_RW(hashmap)->count + 1));
	pmemobj_persist(pop, &D_RW(hashmap)->count,
			sizeof(D_RW(hashmap)->count));

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