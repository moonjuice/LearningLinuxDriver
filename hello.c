#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define DEV_MAJOR 168
#define DEV_NAME "LED_0"
#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)

static int device_major = DEV_MAJOR;
static dev_t devno;
static struct cdev *led_cdev;
static struct class *led_class;

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
  MSG("Hello, LED!!");
  devno = MKDEV(DEV_MAJOR, 0);
  int result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
  if (result < 0) {
    MSG("register led failed!");
    return result;
  }
  MSG("register led succeed!");
  device_major = MAJOR(devno);
  led_cdev = cdev_alloc();
  led_cdev->owner = THIS_MODULE;
  led_cdev->ops = &fops;
  result = -1;
  result = cdev_add(led_cdev, devno, 1);
  if (result < 0) {
    MSG("cdev_add failed!");
    return result;
  }
  MSG("cdev_add succeed!");
  led_class = class_create(THIS_MODULE, DEV_NAME);
  if (IS_ERR(led_class)) {
    MSG("class_create failed");
    // ret = PTR_ERR(led_class);
    return -1;
  }
  if (device_create(led_class, NULL, devno, NULL, DEV_NAME) == NULL) {
    MSG("device_create failed");
  }
  return 0;
}

void device_exit(void) {
  device_destroy(led_class, devno);
  class_destroy(led_class);
  cdev_del(led_cdev);
  unregister_chrdev_region(MKDEV(device_major, 0), 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");