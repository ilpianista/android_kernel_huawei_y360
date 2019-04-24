#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/mmprofile.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/wakelock.h> 

static DEFINE_MUTEX(flag_access);
static  rwlock_t  flag_lock ;
static struct proc_dir_entry *emerg_proc = NULL;
extern int bootbuf;//
extern int posix_fadvise(int fd, loff_t offset, loff_t len, int advice);
static int emerg_config_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *ptr = page;
 //   char temp_data[GTP_CONFIG_MAX_LENGTH + 2] = {0};
    int i;
           if(bootbuf==0x5a5a5a5a)
		ptr += sprintf(ptr, "%d\n", 1);
         else if (bootbuf==0x4a4a4a4a)
	ptr += sprintf(ptr, "%d\n", 2);
		else
		ptr += sprintf(ptr, "%d\n", 0);	
		//	bootbuf=0;
		*eof = 1;
    return (ptr - page);
}

static int emerg_config_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    s32 ret = 0;
    char temp; // for store special format cmd
    char mode_str[8] = {0};
    unsigned int mode; 
    u8 buf[1];
    
    /**********************************************/
    /* for store special format cmd  */
    if (copy_from_user((void*)&temp, buffer, sizeof(temp)))
		{
        printk("copy from user fail 2");
        return -EFAULT;
    }
	printk("brian*****proc_read page:%c\n", temp);
    sscanf(buffer, "%c",&temp);
    if (temp=='1')
		{
		mutex_lock(&flag_access);
		read_lock(&flag_lock);
		*((int*)0xF902FF00) = 0;
		bootbuf=0;
		read_unlock(&flag_lock);
		mutex_unlock(&flag_access);
		}else if(temp=='2')
	           bootbuf=0x4a4a4a4a;
		else
		bootbuf=0x5a5a5a5a;
		
//kernel_restart(NULL);
    if (ret < 0)
    {
        printk("send config failed.");
    }

    return count;
}

static int __init proc_emerg_init(void)
{
         rwlock_init(&flag_lock);
	    emerg_proc = create_proc_entry("emerg_data", 0777, NULL);

    if (emerg_proc == NULL)
    {
        printk("create_proc_entry %s failed", "emerg_data");
        goto out;
    }
    else
    {
        emerg_proc->read_proc = emerg_config_read_proc;
        emerg_proc->write_proc = emerg_config_write_proc;
    }
out:	
	return 0;
}
static int  __exit proc_emerg_exit(void)
{
	return 0;

}
module_init(proc_emerg_init);
module_exit(proc_emerg_exit);

MODULE_AUTHOR("brian.xu");
MODULE_DESCRIPTION("emerg_data");
MODULE_LICENSE("GPL");
