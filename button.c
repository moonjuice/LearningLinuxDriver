#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define MSG(format, arg...) printk(KERN_INFO "moon: " format "\n", ##arg)
int irqNumber = -1;
static int ledState = 1;

static irqreturn_t button_isr(int irq, void *data) {
  // local_irq_save(flags);
  // printk("button_isr !!!!\n");
  gpio_set_value(17, ledState);
  ledState = 1 - ledState;
  // led_trigger = led_trigger ? (0) : (1);
  // local_irq_restore(flags);
  MSG("button irq!!");
  return IRQ_HANDLED;
}

int device_init(void) {
  MSG("Hello, Button!!");
  int result = gpio_request(17, "LED");
  if (result < 0) {
    MSG("request gpio for LED failed!");
    return result;
  }
  gpio_direction_output(17, 0);

  result = gpio_request(27, "BTN");
  if (result < 0) {
    MSG("request gpio for BTN failed!");
    return result;
  }
  result = gpio_direction_input(27);
  if (result < 0) {
    MSG("request gpio for BTN failed!");
    return result;
  }
  irqNumber = gpio_to_irq(27);
  if (irqNumber < 0) {
    MSG("request gpio to irq failed!");
    return irqNumber;
  }
  if (request_irq(irqNumber, button_isr, IRQF_TRIGGER_RISING, "button_test",
                  "button_device_name"))
    return -1;
  return 0;
}

void device_exit(void) {
  gpio_free(17);
  free_irq(irqNumber, "button_device_name");
  gpio_free(27);
  // device_destroy(led_class, devno);
  // class_destroy(led_class);
  // cdev_del(led_cdev);
  // unregister_chrdev_region(MKDEV(device_major, 0), 1);
  MSG("driver un-installed\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");