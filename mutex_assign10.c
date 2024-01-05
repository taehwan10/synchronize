#include "calclock.h"

DEFINE_MUTEX(list_mutex);

LIST_HEAD(my_list);


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

	struct timespec localclcok[2];
	struct my_node *new, *first = NULL;

	printk(KERN_INFO "thread #%d range: %d ~ %d\n",
			thread_id, range_bound[0], range_bound[1]);
	
	int i;
	mutex_lock(&list_mutex);
	for (i = range_bound[0]; i < range_bound[1] + 1; i++){
		getrawmonotonic(&localclock[0]);
		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		if (first == NULL)
			first = new;

		new->data = i;
		list_add_tail(&new->list, &my_list);
		getrawmonotonic(&localclock[1]);

	}
	mutex_unlock(&list_mutex);

	return first;
}


int search_list(int thread_id, void *data, int range_bound[]){

	struct timespec localclock[2];
	struct my_node *cur = (struct my_node *) data, *tmp;

	mutex_lock(&list_mutex);
	list_for_each_entry_safe_from(cur, tmp, &my_list, list){
		getrawmonotonic(&localclock[0]);
		if(cur->data == range_bound[1]){
			getrawmonotonic(localclock[1]);
			calclock(localclock, var1, var2);
			break;
		}

		getrawmonotonic(localclock[1]);
		calclock(localclock, var1, var2);
	}
	mutex_unlock(&list_mutex);

	return 0;
}

int delete_from_list(int thread_id, int range_bound[]){

	struct my_node *cur, *tmp;
	struct timespec localclock[2];

	mutex_lock(&list_mutex);
	list_for_each_entry_safe(cur, tmp, &my_list, list){
		getrawmonotonic(&localclock[0]);
		if(cur->data == localclock[1]){
			list_del(&cur->list);
			kfree(cur);
			getrawmonotonic(&localclock[1]);
			calclock(localclock, var1, var2);
			break;
		}
		list_del(&cur->list);
		kfree(cur);
		getrawmonotonic(localclock[1]);
		calclock(localclock, var1, var2);

	}
	mutex_unlock(&list_mutex);

	return 0;
}
