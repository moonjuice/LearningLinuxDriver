#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>

#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)
#define PIN_PD_SCK 17
#define PIN_DOUT 27
#define PIN_RESET 22
#define DATA_READY_DELAY_NS 1  // 1000000000/1500000=666
#define DEV_NAME "HX711"
static dev_t devno;
static struct cdev *hx711_cdev;
static struct class *hx711_class;
int irqNumber = -1;
int offset = 167213;
#define DEV_NAME_OFFSET "HX711_OFFSET"
static dev_t devno_offset;
static struct cdev *hx711_offset_cdev;
static struct class *hx711_offset_class;
spinlock_t mr_lock;

static int hx711_cycle(void) {
  // unsigned long flags;
  // local_irq_save(flags);
  gpio_set_value(PIN_PD_SCK, 1);
  udelay(DATA_READY_DELAY_NS);
  gpio_set_value(PIN_PD_SCK, 0);
  // local_irq_restore(flags);
  udelay(DATA_READY_DELAY_NS);
  return gpio_get_value(PIN_DOUT);
}

static int read_hx711(void) {
  int i, ret, j, sum;
  int value = 0;
  sum = 0;
  spin_lock_irq(&mr_lock);
  for (i = 0; i < 10; i++) {
    value = 0;
    while (gpio_get_value(PIN_DOUT))
      ;
    for (j = 0; j < 24; j++) {
      value <<= 1;
      ret = hx711_cycle();
      if (ret) value++;
    }
    // value = (value & (1L << 23) ? value | ((-1L) << 24) : value);
    // value ^= 0x800000;
    hx711_cycle();
    // MSG("value: %d ", value);
    sum += value;
  }
  spin_unlock_irq(&mr_lock);
  return sum / 10;
}

static irqreturn_t button_isr(int irq, void *data) {
  unsigned int pinval;
  pinval = gpio_get_value(PIN_RESET);
  if (pinval) {
    MSG("button DOWN!!");
  } else {
    offset = read_hx711();
    MSG("button UP!!");
  }
  return IRQ_HANDLED;
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
  if (*ppos) return 0;
  int result = read_hx711();
  MSG("AVG: %d ", result);
  char result_str[7];
  snprintf(result_str, 7, "%d", result);
  result_str[6] = '\0';
  copy_to_user(buf, result_str, sizeof(result_str));
  *ppos += sizeof(result_str);
  return sizeof(result_str);
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
};

static int device_offset_open(struct inode *inode, struct file *filp) {
  MSG("device_offset_open");
  return 0;
}
static int device_offset_release(struct inode *inode, struct file *filp) {
  MSG("device_offset_release");
  return 0;
}
static ssize_t device_offset_read(struct file *filp, char __user *buf,
                                  size_t count, loff_t *ppos) {
  if (*ppos) return 0;
  char result_str[7];
  snprintf(result_str, 7, "%d", offset);
  result_str[6] = '\0';
  copy_to_user(buf, result_str, sizeof(result_str));
  *ppos += sizeof(result_str);
  return sizeof(result_str);
}

static struct file_operations fops_offset = {
    .owner = THIS_MODULE,
    .open = device_offset_open,
    .release = device_offset_release,
    .read = device_offset_read,
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
  // reset
  gpio_set_value(PIN_PD_SCK, 1);
  udelay(60);
  gpio_set_value(PIN_PD_SCK, 0);
  udelay(60);
  // reset button
  result = gpio_request(PIN_RESET, "BTN");
  if (result < 0) {
    MSG("request gpio for BTN failed!");
    return result;
  }
  result = gpio_direction_input(PIN_RESET);
  if (result < 0) {
    MSG("request gpio for BTN failed!");
    return result;
  }
  irqNumber = gpio_to_irq(PIN_RESET);
  if (irqNumber < 0) {
    MSG("request gpio to irq failed!");
    return irqNumber;
  }
  if (request_irq(irqNumber, button_isr,
                  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "button_test",
                  "button_device_name"))
    return -1;

  devno_offset = MKDEV(168, 0);
  result = alloc_chrdev_region(&devno_offset, 0, 1, DEV_NAME_OFFSET);
  if (result < 0) {
    MSG("register HX711_OFFSET failed!");
    return result;
  }
  MSG("register HX711_OFFSET succeed!");
  hx711_offset_cdev = cdev_alloc();
  hx711_offset_cdev->owner = THIS_MODULE;
  hx711_offset_cdev->ops = &fops_offset;
  result = -1;
  result = cdev_add(hx711_offset_cdev, devno_offset, 1);
  if (result < 0) {
    MSG("cdev_add failed!");
    return result;
  }
  MSG("cdev_add succeed!");
  hx711_offset_class = class_create(THIS_MODULE, DEV_NAME_OFFSET);
  if (IS_ERR(hx711_offset_class)) {
    MSG("class_create failed");
    return -1;
  }
  if (device_create(hx711_offset_class, NULL, devno_offset, NULL,
                    DEV_NAME_OFFSET) == NULL) {
    MSG("device_create failed");
  }
  spin_lock_init(&mr_lock);
  return 0;
}

void device_exit(void) {
  gpio_free(PIN_PD_SCK);
  gpio_free(PIN_DOUT);
  device_destroy(hx711_class, devno);
  class_destroy(hx711_class);
  cdev_del(hx711_cdev);
  unregister_chrdev_region(devno, 1);
  free_irq(irqNumber, "button_device_name");
  gpio_free(PIN_RESET);
  device_destroy(hx711_offset_class, devno_offset);
  class_destroy(hx711_offset_class);
  cdev_del(hx711_offset_cdev);
  unregister_chrdev_region(devno_offset, 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");