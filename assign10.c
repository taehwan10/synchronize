#include "calclock.h"



static int work_fn(void *data){

	int range_bound[2];
	int thread_id = *(int *) data;

	set_iter_range(thread_id, range_bound);
	void *ret = add_to_list(thread_id, range_bound);
	search_list(thread_id, ret, range_bound);
	delete_from_list(thread_id, range_bound);

	while(!kthread_should_stop()) {
		msleep(500);
	}

	printk(KERN_INFO "thread #%d stopped!\n", thread_id);

	return 0;
}

void *add_to_list(int thread_id, int range_bound[]){

	
	printk(KERN_INFO "thread #%d range: %d ~ %d\n",
			thread_id, range_bound[0], range_bound[1]);



	return first;
}


int search_list(int thread_id, void *data, int range_bound[]){

	return first;
}

int search_list(int thread_id, void *data, int range_bound[]){

	struct timespec localclock[2];

	struct my_node *cur = (struct my_node *) data, *tmp;

	return 0;
}

int delete_from_list(int thread_id, int range_bound[]){

	struct my_node *cur, *tmp;
	struct timespec localclock[2];

	return 0;
}
