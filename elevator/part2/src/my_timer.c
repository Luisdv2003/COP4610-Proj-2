#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

#define ENTRY_NAME "timer"
#define PERMS 0644
#define PARENT NULL
#define BUF_LEN 256

MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry;

static struct timespec64 past_time;

static int read = 1;

static ssize_t procfile_read(
		struct file *file,
		char __user *ubuf,
		size_t count,
		loff_t *ppos)
{
	char buffer[BUF_LEN];
	int len;

	struct timespec64 present_time;
	struct timespec64 elapsed;

	if (*ppos >0){
		return 0;}

	
	ktime_get_real_ts64(&present_time);

	if(read){
		len=snprintf(
				buffer,
				BUF_LEN,
				"current time: %lld.%09ld\n",
				(long long)present_time.tv_sec,
				present_time.tv_nsec
			    );

		read = 0;
	}
	else{
		elapsed.tv_sec = 
			present_time.tv_sec - past_time.tv_sec;

		elapsed.tv_nsec=
			present_time.tv_nsec - past_time.tv_nsec;

		if(elapsed.tv_nsec <0){
			elapsed.tv_sec--;
			elapsed.tv_nsec += 1000000000;
		}

		len=snprintf(
				buffer,
				BUF_LEN,
				"current time: %lld.%09ld\n"
				"elapsed time: %lld.%09ld\n",
				(long long) present_time.tv_sec,
				present_time.tv_nsec,
				(long long)elapsed.tv_sec,
				elapsed.tv_nsec
			    );

	}

	past_time = present_time;

	if(len > count)
		len = count;

	if(copy_to_user(ubuf, buffer,len)){
		return -EFAULT;}


	*ppos=len;

	return len;
}
static const struct proc_ops timer_fops = {
	.proc_read=procfile_read,
};

static int __init my_timer_init(void)
{
	proc_entry=proc_create(
			ENTRY_NAME,
			PERMS,
			PARENT,
			&timer_fops
			);
	if(proc_entry ==NULL)
		return -ENOMEM;

	return 0;
}

static void __exit my_timer_exit(void)
{
	proc_remove(proc_entry);
}

module_init(my_timer_init);
module_exit(my_timer_exit);
