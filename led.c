#include <linux/module.h> 
#include <linux/init.h>   
#include <linux/err.h>   
#include <linux/fs.h>   
#include <linux/device.h>   
#include <asm/uaccess.h>	//copy to user / copy from user
#include <linux/gpio.h>		// 鏍囧噯 GPIO_API
#include <linux/slab.h>		// kzalloc()

#define	DEMO_MAJOR	239
#define DEV_NAME	"char_dev"
#define BUF_SIZE	1024

#define LED_PIN		25

static struct class *demo_class ;

struct led_data {
	int pin ;
	char *label;
};
static struct led_data *led ;

static ssize_t demo_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{	
	ssize_t status ;
	int value ;
	struct led_data *led_rd ;
	printk("Now entering %s() \n", __FUNCTION__ );
	led_rd = file->private_data ;
	printk("LED-Pin is %d Label is %s \n",led_rd->pin , led_rd->label );
	value = gpio_get_value(led_rd->pin);
	status = copy_to_user(user_buf,&value,count);
	printk("user read data is %s\n",user_buf );
	return status ;
}

static ssize_t demo_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	ssize_t	status ;
	unsigned char data[BUF_SIZE];
	struct led_data *led_wr ;
	printk("Now entering %s() \n", __FUNCTION__ );
	led_wr = file->private_data ;
	printk("LED-Pin is %d Label is %s \n",led_wr->pin , led_wr->label );
	status = copy_from_user( data,user_buf,count);
	printk("user write data is %s\n",data);
	status = gpio_direction_output( led_wr->pin , data[0]&1);
	
	return  status ;
}

static int demo_open(struct inode *inode, struct file *file)
{	int status ;
	printk("Now entering %s() \n", __FUNCTION__ );
	led = kzalloc(sizeof(*led),GFP_KERNEL);
	if(!led)
		return	-ENOMEM ;
	led->pin = LED_PIN ;
	led->label = "LED"; 
	status = gpio_request( led->pin , led->label);
	if(status<0)
		printk("Error : request Pin-%d failed \n", LED_PIN);
	file->private_data = led ;
	printk("request Pin-%d success\n", LED_PIN);
	return status ;
}

static int demo_close(struct inode *inode, struct file *file)
{
	printk("Now entering %s() \n", __FUNCTION__ );
	gpio_free(LED_PIN);
	kfree(led);
	return 0 ;
}

struct file_operations demo_ops = {
	.owner = THIS_MODULE ,
	.open = demo_open ,
	.release = demo_close ,
	.write = demo_write ,
	.read = demo_read ,
};

static int __init hello_init(void){
	int status ;
	printk("Hello driver world\n");
	status = register_chrdev( DEMO_MAJOR , DEV_NAME , &demo_ops);
	if ( status < 0 ){
		printk("Failed to register char device - %d\n" , DEMO_MAJOR );
		return status ;
	}

	demo_class = class_create(THIS_MODULE , "demo-class");
	if (IS_ERR(demo_class)){
		unregister_chrdev( DEMO_MAJOR , DEV_NAME );
	}

	device_create(demo_class ,NULL , MKDEV(DEMO_MAJOR , 0), NULL ,  "hubuyu" );
	return 0;
}
	 
static void __exit hello_exit(void){
	printk("Goodbye driver world\n");
	device_destroy(demo_class , MKDEV(DEMO_MAJOR ,0));
    class_destroy(demo_class);
    unregister_chrdev(DEMO_MAJOR , DEV_NAME);

}

module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("hubuyu");
MODULE_DESCRIPTION("Lesson 4-4 : led on/off baes on file_operations");

