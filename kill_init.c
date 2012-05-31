#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/signal.h>       //force_sig
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/interrupt.h>    //request_irq 
#include <linux/gpio.h>	        //OMAP_GPIO_IRQ
#include <plat/mux.h>	        //omap_cfg_reg
#include <linux/irq.h>	        //IRQ_TYPE_LEVEL_LOW
#include <linux/nsproxy.h>

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/device.h>
#include <mach/hardware.h>

MODULE_LICENSE("GPL");

//MMC2_DAT6, P9-pin5, system default GPIO input with pullup
#define OMAP3_GPIO138          (138)
int irq;

static irqreturn_t my_interrupt(int irq, void *dev_id){
	struct task_struct *p;
	printk(KERN_ALERT "my_interrupt executed!\n");
//	struct task_struct *p = find_task_by_vpid((pid_t)1);
    p = pid_task(find_pid_ns((pid_t)1, current->nsproxy->pid_ns), PIDTYPE_PID);

	force_sig(SIGTERM,p);
	printk(KERN_INFO "The linux system will shutdown");
	return IRQ_HANDLED;
}

static int hello_init(void) {
	int ret;

	//below function: Sets the Omap MUX and PULL_DWN registers based on the table and judge 'cpu_class_is_omap1'
	//omap_cfg_reg(OMAP3_GPIO138);
		
	ret = gpio_request(OMAP3_GPIO138, "OMAP3_GPIO138");
	if(ret < 0)
		printk(KERN_ALERT "gpio_request failed!\n");
		
	gpio_direction_input(OMAP3_GPIO138);	
	
	irq = OMAP_GPIO_IRQ(OMAP3_GPIO138);	//irq33 <-> GPIO module 5: includes gpio_138
	printk(KERN_ALERT "OMAP_GPIO_IRQ success! The irq = %d\n", irq);
	
	irq_set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);
	enable_irq(gpio_to_irq(OMAP3_GPIO138));
	
	ret = request_irq(irq, my_interrupt, IRQF_DISABLED, "my_interrupt_proc", NULL);
	if (ret==0)
        printk(KERN_ALERT "request_irq success!\n");
    else
        printk(KERN_ALERT "request_irq fail!\n"); 
		
    printk(KERN_ALERT "Hello, Tom Xue! From inside kernel driver!\n");
    return 0;
}

static void hello_exit(void)
{
    disable_irq(irq);
    free_irq(irq, NULL);
    gpio_free(OMAP3_GPIO138);
    printk(KERN_INFO "Goodbye, Tom Xue! From inside kernel driver!\n");
}

module_init(hello_init);
module_exit(hello_exit);
