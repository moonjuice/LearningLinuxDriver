#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#define DEV_MAJOR 168
#define DEV_NAME "LED_0"
#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)

static int device_major = DEV_MAJOR;

static int device_open(struct inode *inode, struct file *filp) { return 0; }
static int device_release(struct inode *inode, struct file *filp) { return 0; }
static ssize_t device_read(struct file *filp, char __user *buf, size_t count,
                           loff_t *ppos) {
  return 0;
}
static ssize_t device_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *ppos) {
  return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

int device_init(void) {
  MSG("Hello, World!!");
  // if (register_chrdev(DEV_MAJOR, DEV_NAME, &fops) < 0) {
  //   MSG("Couldn't register a device.");
  //   return -1;
  // }
  dev_t devno = MKDEV(DEV_MAJOR, 0);
  int result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
  if (result < 0) {
    MSG("register devno failed!");
    return result;
  } else {
    MSG("register devno succeed!");
    device_major = MAJOR(devno);
  }
  return 0;
}

void device_exit(void) {
  //   if (unregister_chrdev(DEV_MAJOR, DEV_NAME) < 0)
  //     MSG("failed to unregister driver");
  //   else
  unregister_chrdev_region(MKDEV(device_major, 0), 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");