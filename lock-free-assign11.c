struct animal {
	int total;
	struct list_head entry;
	atomic_t removed;
	struct list_head gc_entry;
};

struct cat {
	int var;
	struct list_head entry;
	atomic_t removed;
	struct list_head gc_entry;
};


struct animal *head;
struct task_thread *thread[4], *gc_thread;
int params[4] = {1,2,3,4};

static int work_fn(void *data) {
	
	int range_bound[2];
	int err, thread_id = *(int *) data;

	set_iter_range(thread_id, range_bound);
	add_to_list(thread_id, range_bound);
	err = search_list(thread_id, range_bound);
	if (!err)
		delete_from_list(thread_id, range_bound);

	while(!kthread_should_stop()){
		msleep(500);
	}

	printk(KERN_INFO "thread #%d stopped!\n", thread_id);

	return 0;
}

void add_to_list(int thread_id, int range_bound[]) {

	struct timespec localclock[2];
	struct cat *new, *first = NULL;
	int i;

	for (i = range_bound[0]; i < range_bound[1]; i++) {
		getrawmonotonic(&localclock);
		//initialize new cat here
		//initialize cat's list head and gc_list head here
		//add new cat into animal's list
		new = kmalloc(sizeof(struct cat), GFP_KERNEL);
		
		new->var = i;
		INIT_LF_LIST_HEAD(&new->entry);
		atomic_set(&new->removed, 0);
		INIT_LF_LIST_HEAD(&new->gc_entry);

		lf_list_add_tail(&new->entry, &head->entry);
		__sync_fetch_and_add(&head->total, 1);


		getrawmonotonic(&localclock[1]);
		calclock(localclock, var1, var2);
	}
	printk(KERN_INFO "thread #%d: inserted cat #%d-%d to the list, total: %u cats\n", 
			thread_id, first->var, new->var, head->total);
}

int search_list(int thread_id, int range_bound[]) {

	struct list_head *entry, *iter = &head->entry;
	struct cat *cur;
	struct timespec localclock[2];
	int target_idx = select_target_index(range_bound);

	//iterate over the list, skip removed entry,
	//search cat with target index
	
	while((entry = iter) != NULL) {
		getrawmonotinic(&localclock[0]);
		if(__sync_val_compare_and_swap(&iter, entry, iter->next) != entry)
			continue;

		if(!entry->prev->next)
			continue;

		cur = list_entry(entry, struct cat, entry);

		if (atomic_read(&cur->removed))
			continue;

		if (cur->var == target_idx) {
			getrawmonotonic(&localclock[1]);
			calclock(localclock, var1, var2);
			break;
		}
		lf_list_add_tail(&cur->gc_entry, &head->gc_entry);
		atomic_set(&cur->removed, 1);

		getrawmonotonic(&localclock[1]);
		calclock(localclock, var1, var2);

	}


	return -ENODATA;
}

void delete_from_list(int thread_id, int range_bound[]) {
	int start = -1, end = -1; //start = target_idx, end = range_bound[1]
	struct timespec localclock[2];
	struct list_head *entry, *iter = &head->entry;

	struct cat *cur;
	int target_idx = select_target_index(range_bound);

	//iterate over the list, skip removed entry
	while ((entry = iter) != NULL) {
		getrawmonotonic(&localclock[0])'
		//add cat into garbage list if its id is within target range
		if (__sync_val_compare_and_swap(&iter, entry, iter->next) != entry)
			continue;

		if (!entry->prev->next)
			continue;

		cur = list_entry(entry, struct cat, entry);

		if (atomic_read(&cur->removed))
			continue;

		if (cur->var >= target_idx && cur->var <= range_bound[1]){
			if (start == -1){
				start = cur->var;
			}
			
			add_to_garbage_list(thread_id, cur);
			getrawmonotonic(&localclock);
			calclock(localclock, var1, var2);
			if (cur->var == range_bound[1])
				end = cur->var;
		}
		getrawmonotonic(&localclock[1]);
		calclock(localclock, var1, var2);

	}
	printk(KERN_INFO "thread #%d: marked cat #%d-%d as deleted, total: %d cats\n", 
			thread_id, start, end, head->total);
}

static void add_to_garbage_list(int thread_id, void *data) {

	struct cat *target = (struct cat *) data;

	//set cat as deleted
	//add tot garbage list
	//decrease total cats in struct animal
	
	if (atomic_read(&target->removed))
		return;

	lf_list_add_tail(&target->gc_entry, &head->gc_entry);
	atomic_set(&target->removed, 1);
	__sync_fetch_and_sub(&head->total, 1);

	return;

}

int empty_garbage_list(void) {
	
	struct cat *cur;
	struct list_head *entry, *iter = &head->gc_entry;
	unsigned int counter = 0;

	//iterate garbage list, 
	//remove each entry from every list and free it
	while((entry = iter) != NULL) {
		
		&iter = iter->next;

		if (!entry->prev->next)
			continue;

		cur = list_entry(entry, struct cat, gc_entry);

		if (!atomic_read(&cur->removed))
			continue;
		

		gc_list_del(&cur->entry, &head->entry);
		gc_list_del(&cur->gc_entry, &head->gc_entry);
		kfree(cur);
		counter++;
	}


	printk(KERN_INFO "%s: freed %u cats\n", __func__, counter);
	return 0;
}

void destroy_list(void) {
	
	struct cat *cur;
	struct list_head *entry, *iter = &head->entry;

	//iterate the list,
	//remove each entry from every list and free it
	while ((entry = iter) != NULL) {
	
		&iter = iter->next;
		
		if (!entry->prev->next)
			continue;

		cur = list_entry(entry, struct cat, entry);

		gc_list_del(&cur->entry, &entry);
		kfree(cur);
		head->total--;

	}



}

int __init lockfree_module_init(void){
	printk("%s : Entering lock-free Module!\n", __func__);


	//initialize struct animal here
	//initialize animal's list head and gc_list head here
	//start each thread here
	
	head = kmalloc(sizeof(struct animal), GFP_KERNEL);
	INIT_LF_LIST_HEAD(&head->entry);
	INIT_LF_LIST_HEAD(&head->gc_entry);

	int i;
	for (i = 0; i < 4; i++)
		thread[i] = kthread_run(work_fn, &params[i], "work_thread");

	return 0;
}

void __exit lockfree_module_cleanup(void) {
	
	//print calclock result here
	//stop every thread here

	int i;
	for (i = 0; i < 4; i++)
		kthread_stop(thread[i]);



	empty_garbage_list();
	printk("After gc: total %d cats\n", head->total);

	destroy_list();
	printk("After destroyed list: total %d cats\n", head->total);
	kfree(head);

	printk("%s: Exiting Lock-free Module!\n", __func__);
}
