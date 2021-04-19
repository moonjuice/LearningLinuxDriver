#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)
#define PIN_PD_SCK 27
#define PIN_DOUT 17
#define DATA_READY_DELAY_NS 250
#define DEV_NAME "HX711"
static dev_t devno;
static struct cdev *hx711_cdev;
static struct class *hx711_class;

static int hx711_cycle(void) {
  unsigned long flags;
  local_irq_save(flags);
  gpio_set_value(PIN_PD_SCK, 1);
  ndelay(DATA_READY_DELAY_NS);
  gpio_set_value(PIN_PD_SCK, 0);
  local_irq_restore(flags);
  ndelay(DATA_READY_DELAY_NS);
  return gpio_get_value(PIN_DOUT);
}

static int device_open(struct inode *inode, struct file *filp) {
  MSG("device_open");
  return 0;
}
static int device_release(struct inode *inode, struct file *filp) {
  MSG("device_release");
  return 0;
}
static ssize_t device_read(struct file *filp, char __user *buf, size_t count,
                           loff_t *ppos) {
  int i, ret, j, sum;
  int value = 0;
  if (gpio_get_value(PIN_DOUT)) {
    MSG("not ready!");
  } else {
    MSG("---start---");
    sum = 0;
    for (i = 0; i < 10; i++) {
      value = 0;
      while (!gpio_get_value(PIN_DOUT))
        ;
      for (j = 0; j < 24; j++) {
        value <<= 1;
        ret = hx711_cycle();
        if (ret) value++;
      }
      // value = (value & (1L << 23) ? value | ((-1L) << 24) : value);
      // value ^= 0x800000;
      hx711_cycle();
      MSG("value: %d ", value);
      sum += value;
      mdelay(100);
    }
    MSG("AVG: %d ", sum / 10);
    MSG("--- end ---");
  }
  return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
};

int device_init(void) {
  MSG("Hello, HX711!!");
  int result = gpio_request(PIN_PD_SCK, "PD_SCK");
  if (result < 0) {
    MSG("request gpio for PD_SCK failed!");
    return result;
  }
  gpio_direction_output(PIN_PD_SCK, 0);

  result = gpio_request(PIN_DOUT, "DOUT");
  if (result < 0) {
    MSG("request gpio for DOUT failed!");
    return result;
  }
  result = gpio_direction_input(PIN_DOUT);
  if (result < 0) {
    MSG("request gpio for DOUT failed!");
    return result;
  }
  devno = MKDEV(168, 0);
  result = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
  if (result < 0) {
    MSG("register HX711 failed!");
    return result;
  }
  MSG("register HX711 succeed!");
  hx711_cdev = cdev_alloc();
  hx711_cdev->owner = THIS_MODULE;
  hx711_cdev->ops = &fops;
  result = -1;
  result = cdev_add(hx711_cdev, devno, 1);
  if (result < 0) {
    MSG("cdev_add failed!");
    return result;
  }
  MSG("cdev_add succeed!");
  hx711_class = class_create(THIS_MODULE, DEV_NAME);
  if (IS_ERR(hx711_class)) {
    MSG("class_create failed");
    return -1;
  }
  if (device_create(hx711_class, NULL, devno, NULL, DEV_NAME) == NULL) {
    MSG("device_create failed");
  }
  return 0;
}

void device_exit(void) {
  gpio_free(PIN_PD_SCK);
  gpio_free(PIN_DOUT);
  device_destroy(hx711_class, devno);
  class_destroy(hx711_class);
  cdev_del(hx711_cdev);
  unregister_chrdev_region(devno, 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");