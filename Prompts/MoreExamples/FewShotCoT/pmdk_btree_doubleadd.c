
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

## Correction 1
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
    
    rootp->len = strlen(buf);
    
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
    
    rootp->len = strlen(buf);
    
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));
	
    printf("\nWrite the (%s) string to persistent-memory.\n", rootp->buf);
	pmemobj_close(pop);	

	return;
}

'''.

## Explanation
In the correction, after the update 'rootp->len', it needs to be included 'pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));' to ensure that the value of 'rootp->len' is durably stored in the persistent memory pool. 
By adding 'pmemobj_persist(pop, &rootp->len, sizeof(rootp->len));', it is guaranteed that the assignment to 'rootp->len' is properly preserved, and the data remains consistent. This correction ensures data durability in persistent memory.


===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM
''' C
int main(int argc, const char *argv[]) {
	if (argc < 3) {
		printf("usage: %s "
			"<ctree|btree|rbtree|hashmap_atomic|hashmap_tx>"
			" file-name [nops]\n", argv[0]);
		return 1;
	}

	const char *type = argv[1];
	const char *path = argv[2];
	const struct map_ops *map_ops = parse_map_type(type);
	if (!map_ops) {
		fprintf(stderr, "invalid map type -- '%s'\n", type);
		return 1;
	}

//	int nops = MAX_INSERTS;
	int nops = atoi(argv[3]);

	if (argc > 3) {
		nops = atoi(argv[3]);
		if (nops <= 0 || nops > MAX_INSERTS) {
			fprintf(stderr, "number of operations must be "
				"in range 1..%u\n", MAX_INSERTS);
			return 1;
		}
	}

	struct timeval tv_start;
	struct timeval tv_end;

	gettimeofday(&tv_start, 0);

	PMEMobjpool *pop;
	//srand(time(NULL));

	if (access(path, F_OK) != 0) {
		if ((pop = pmemobj_create(path, POBJ_LAYOUT_NAME(data_store),
			PMEMOBJ_MIN_POOL, 0666)) == NULL) {
			perror("failed to create pool\n");
			return 1;
		}
	} else {
		printf("open existing file\n");
		if ((pop = pmemobj_open(path,
				POBJ_LAYOUT_NAME(data_store))) == NULL) {
			perror("failed to open pool\n");
			return 1;
		}
	}
	
	gettimeofday(&tv_end, 0);
	printf("obj init time=%lu us\n", 1000000 * (tv_end.tv_sec - tv_start.tv_sec) + 
		(tv_end.tv_usec - tv_start.tv_usec));

	void *p = C_createVeriInstance();
	TOID(struct store_root) root = POBJ_ROOT(pop, struct store_root);

	struct map_ctx *mapc = map_ctx_init(map_ops, pop);
	if (!mapc) {
		perror("cannot allocate map context\n");
		return 1;
	}
	/* delete the map if it exists */
	//if (!map_check(mapc, D_RW(root)->map))
	//	map_delete(mapc, &D_RW(root)->map);

	/* insert random items in a transaction */
	int aborted = 0;

	void *metadataVectorPtr[(nops>nkeys ? nops : nkeys)];
	for (int i = 0; i < nops; i++) {
		metadataVectorPtr[i] = C_createMetadataVector();
	}

	gettimeofday(&tv_start, 0);
	
	//TX_BEGIN(pop) {
		map_new(mapc, &D_RW(root)->map, NULL);

		for (int i = 0; i < nops; ++i) {
			/* new_store_item is transactional! */
			metadataPtr = metadataVectorPtr[i];
			PMTest_START;
			if (strcmp("hashmap_atomic", type))
				PMTest_CHECKER_START;
			TX_BEGIN(pop) {
			map_insert(mapc, D_RW(root)->map, rand(),
					new_store_item().oid);
			} TX_END
			if (strcmp("hashmap_atomic", type))
				PMTest_CHECKER_END;
			PMTest_END;
			C_execVeri(p, metadataPtr);
		}
	//} TX_ONABORT {
	//	perror("transaction aborted\n");
	//	map_ctx_free(mapc);
	//	aborted = 1;
	//} TX_END


	gettimeofday(&tv_end, 0);
	C_getVeri(p, (void *)(0));

	for (int i = 0; i < nops; i++) {
		C_deleteMetadataVector(metadataVectorPtr[i]);
	}
	
	printf("@insertion time = %lu\n", 1000000 * (tv_end.tv_sec - tv_start.tv_sec) + 
		(tv_end.tv_usec - tv_start.tv_usec));
	if (aborted)
		return -1;


	/* count the items */
	map_foreach(mapc, D_RW(root)->map, get_keys, NULL);

	for (int i = 0; i < nkeys; i++) {
		metadataVectorPtr[i] = C_createMetadataVector();
	}

	gettimeofday(&tv_start, 0);
	/* remove the items without outer transaction */
	for (int i = 0; i < nkeys; ++i) {
		metadataPtr = metadataVectorPtr[i];
		PMTest_START;

		// BUG //

		if (strcmp("hashmap_atomic", type))
			PMTest_CHECKER_START;

		PMEMoid item;
		if (i % 2 == 0)
			item = map_remove(mapc, D_RW(root)->map, keys[i/2]);
		else
			item = map_remove(mapc, D_RW(root)->map, keys[nkeys-(i+1)/2]);
			
		// BUG //

		if (strcmp("hashmap_atomic", type))
			PMTest_CHECKER_END;
		PMTest_END;
		C_execVeri(p, metadataPtr);

		assert(!OID_IS_NULL(item));
		if(!OID_INSTANCEOF(item, struct store_item)) {
			printf("%lu, %lu\n", TOID_TYPE_NUM(struct store_item), pmemobj_type_num(item));
		}
		assert(OID_INSTANCEOF(item, struct store_item));

	}
	
	gettimeofday(&tv_end, 0);

	C_getVeri(p, (void *)(0));
	for (int i = 0; i < nkeys; i++) {
		C_deleteMetadataVector(metadataVectorPtr[i]);
	}

	printf("delete time=%lu us\n", 1000000 * (tv_end.tv_sec - tv_start.tv_sec) + 
		(tv_end.tv_usec - tv_start.tv_usec));
	
	C_deleteVeriInstance(p);

	uint64_t old_nkeys = nkeys;

	/* tree should be empty */
	map_foreach(mapc, D_RW(root)->map, dec_keys, NULL);
	assert(old_nkeys == nkeys);

	map_ctx_free(mapc);
	pmemobj_close(pop);

	return 0;
}
'''.
