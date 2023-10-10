
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

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {
		*map = TX_ZNEW(struct hashmap_tx);

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

===== assistant =====

## Correction
''' C

int
hm_tx_create(PMEMobjpool *pop, TOID(struct hashmap_tx) *map, void *arg)
{
	struct hashmap_args *args = (struct hashmap_args *)arg;
	int ret = 0;
	TX_BEGIN(pop) {
		TX_ADD_DIRECT(map);
		*map = TX_ZNEW(struct hashmap_tx);

		uint32_t seed = args ? args->seed : 0;
		create_hashmap(pop, *map, seed);
	} TX_ONABORT {
		ret = -1;
	} TX_END

	return ret;
}

'''.

## Explanation
In the correction, a new line 'TX_ADD_DIRECT(map);' is added before allocating memory for the new struct 'hashmap_tx'. 
This line tells the transaction system that it is intended to modify the 'map' pointer during the transaction. It essentially registers 'map' as a part of the transaction, making sure that if the transaction aborts, the changes to 'map' are rolled back as well.
By adding 'TX_ADD_DIRECT(map);', the transaction system ensures that the assignment of the new struct 'hashmap_tx' to 'map' is properly recorded. 
If the transaction is successful, this change will be committed, and if the transaction aborts, it will be rolled back, maintaining the consistency of your persistent memory data structures.

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
/*
 * hm_tx_insert -- inserts specified value into the hashmap,
 * returns:
 * - 0 if successful,
 * - 1 if value already existed,
 * - -1 if something bad happened
 */
int
hm_tx_insert(PMEMobjpool *pop, TOID(struct hashmap_tx) hashmap,
	uint64_t key, PMEMoid value)
{
	TOID(struct buckets) buckets = D_RO(hashmap)->buckets;
	TOID(struct entry) var;

	uint64_t h = hash(&hashmap, &buckets, key);
	int num = 0;

	for (var = D_RO(buckets)->bucket[h];
			!TOID_IS_NULL(var);
			var = D_RO(var)->next) {
		if (D_RO(var)->key == key)
			return 1;
		num++;
	}
		// BUG //

	int ret = 0;
	TX_BEGIN(pop) {

		//TX_ADD_FIELD(buckets, bucket[h]);
		TX_ADD_FIELD(hashmap, count);

		// BUG //
		
		TOID(struct entry) e = TX_NEW(struct entry);
		PMTest_assign(&(D_RW(e)->key), sizeof(struct entry));
		(D_RW(e)->key) = (key);
		(D_RW(e)->value) = (value);
		(D_RW(e)->next) = (D_RO(buckets)->bucket[h]);
		/*
		PM_EQU((D_RW(e)->key), (key));
		PM_EQU((D_RW(e)->value), (value));
		PM_EQU((D_RW(e)->next), (D_RO(buckets)->bucket[h]));
		*/
		PM_EQU((D_RW(buckets)->bucket[h]), (e));
		PM_EQU((D_RW(hashmap)->count), (D_RW(hashmap)->count+1));
		num++;
	} TX_ONABORT {
		fprintf(stderr, "transaction aborted: %s\n",
			pmemobj_errormsg());
		ret = -1;
	} TX_END

	if (ret)
		return ret;

	if (num > MAX_HASHSET_THRESHOLD ||
			(num > MIN_HASHSET_THRESHOLD &&
			D_RO(hashmap)->count > 2 * D_RO(buckets)->nbuckets))
		hm_tx_rebuild(pop, hashmap, D_RO(buckets)->nbuckets * 2);

	return 0;
}
'''.
