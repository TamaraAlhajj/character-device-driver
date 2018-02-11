/* 
 * Tamara Alhajj
 * 100948027
 * COMP 3000 Ex4
 */
 
#include <linux/init.h>           
#include <linux/module.h>         
#include <linux/device.h>                        
#include <linux/kernel.h>               
#include <linux/fs.h>            
#include <linux/uaccess.h>    

#define DEVICE_NAME "tamara_prime"        
#define CLASS_NAME "comp3000"       
                                                             
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tamara Alhajj");      
MODULE_DESCRIPTION("A Linux character driver, to generate infinite primes.");
MODULE_VERSION("0.1");
                                                                                                 
// automatically determined device number
static int majorNumber;

// data from userspace
static char message[256] = {0};

// its size
static short size_of_message;

// number of times the device is opened
static int numberOpens = 0;

// driver class struct pointer
static struct class* driverClass = NULL;

// driver device struct pointer
static struct device* driverDevice = NULL;

// driver prototype functions
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);


//Driver-operation associations
static struct file_operations fops = {
 .open = dev_open,
 .read = dev_read,
 .release = dev_release,
};

/* 
 * Inefficiently, compute successive prime numbers.  
 * Return the nth prime number.  
 */
int compute_prime (int n) {
	int factor;
	int candidate = 2;
	
	if(n <= 0) return -1; //fail
    
    while (true) {
		int is_prime = true;
       /* test primality by successive division */
		for (factor = 2; factor < candidate; ++factor)
			if (candidate % factor == 0) {
				is_prime = false;
				break;
			}
		/* is prime number we're looking for?  */
		if (is_prime) {
 			if (--n == 0) {
				/* define the result  */
				return candidate;
			}
		}
		++candidate;
	}
}

/* Initialization function
*/
static int __init comp3000_init(void) {
	printk(KERN_INFO "comp3000: initializing\n");
	// dynamically allocate a major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0) {
		printk(KERN_ALERT "comp3000: failed to allocate major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "comp3000: registered with major number %d\n", majorNumber);
	// register device class
	driverClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(driverClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "comp3000: failed to register device class\n");
		return PTR_ERR(driverClass);
	}
	printk(KERN_INFO "comp3000: device class registered\n");
	// register device driver
	driverDevice = device_create(driverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(driverDevice)) {
		// if error, cClean up
		class_destroy(driverClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "comp3000: failed to create device\n");
		return PTR_ERR(driverDevice);
	}
	printk(KERN_INFO "comp3000: device class created\n");
	return 0;
}

/* Cleanup */
static void __exit comp3000_exit(void) {
	// remove device
	device_destroy(driverClass, MKDEV(majorNumber, 0));
	// unregister device class
	class_unregister(driverClass);
	// remove device class
	class_destroy(driverClass);
	// unregister major number
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "comp3000: closed\n");
}

/* Called each time the device is opened.
* inodep = pointer to inode
* filep = pointer to file object
*/
static int dev_open(struct inode *inodep, struct file *filep) {
	// increment numberOpens counter
	numberOpens++;
	printk(KERN_INFO "comp3000: device opened %d time(s)\n", numberOpens);
	return 0;
}

/* Called when device is read.
* filep = pointer to a file
* buffer = pointer to the buffer to which this function writes the data
* len = length of buffer
* offset = offset in buffer
*/
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
	int error_count = 0;
	int input = 0;
	int ans = 0;
	
	if(kstrtouint (buffer, 10, &input) != 0) return -1; //err
	ans = compute_prime(input);	//get user's desired number of primes
	sprintf(message, "%d", ans); //display
	size_of_message = strlen(message);
		
	// copy_to_user has format ( * to, *from, size), returns 0 on success
	error_count = copy_to_user(buffer, message, size_of_message);
	if (error_count==0) { // success!
		printk(KERN_INFO "comp3000: sent a total of %d primes to user\n", input);
		return (size_of_message=0); // reset position and return 0
	}
	else {
		printk(KERN_INFO "comp3000: failed to send prime %d to the user\n", input);
		return -EFAULT; // failure!
	}

}

/* Called when device is closed/released.
* inodep = pointer to inode
* filep = pointer to a file
*/
static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "comp3000: released\n");
	return 0;
}

module_init(comp3000_init);
module_exit(comp3000_exit);  
