#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENCE("GPL");
MODULE_AUTHOR("Ud");
MODULE_DESCRIPTION("my first linux kernel module");
static int __init hello_init(void){
    pr_info("kernel-lab : Hello kernel\n");
    return 0;
}
static void __exit hello_exit(void){
    pr_info("kernel-lab : Goodbye,kernel!\n");
}
module_init(hello_init);
module_exit(hello_exit);
