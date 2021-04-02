#include <linux/module.h>
#include <linux/kernel.h>

int hello_init(void) {
    printk("Hello, World!!");
    return 0;
}

void hello_exit(void) {
    printk("Goodbye!!");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");