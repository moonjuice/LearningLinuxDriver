#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define DEV_MAJOR 168
#define DEV_NAME "LED_0"
#define BUF_SIZE 5
#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)

static int device_major = DEV_MAJOR;
static dev_t devno;
static struct cdev *led_cdev;
static struct class *led_class;
static char led_state[2];

static int device_open(struct inode *inode, struct file *filp) {
  MSG("led open");
  return 0;
}
static int device_release(struct inode *inode, struct file *filp) {
  MSG("led release");
  return 0;
}
static ssize_t device_read(struct file *filp, char __user *buf, size_t count,
                           loff_t *ppos) {
  if (*ppos) return 0;
  MSG("led read");
  led_state[1] = '\0';
  int ret = 0;
  ret = copy_to_user(buf, led_state, 2);
  if (ret != 0) {
    MSG("Copy failed!");
    return -EFAULT;
  }
  *ppos += 2;
  return 2;
}
static ssize_t device_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *ppos) {
  MSG("led write");
  unsigned char value;
  if (copy_from_user(&value, buf, 1)) return -EFAULT;
  MSG("value is : %x", value);
  if (value & 0x01) {
    led_state[0] = '1';
    gpio_set_value(17, 1);
  } else {
    led_state[0] = '0';
    gpio_set_value(17, 0);
  }
  return 1;
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
  int result = gpio_request(17, DEV_NAME);
  if (result < 0) {
    MSG("request gpio failed!");
    return result;
  }
  gpio_direction_output(17, 0);
  devno = MKDEV(DEV_MAJOR, 0);
  result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
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
  gpio_free(17);
  device_destroy(led_class, devno);
  class_destroy(led_class);
  cdev_del(led_cdev);
  unregister_chrdev_region(MKDEV(device_major, 0), 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");