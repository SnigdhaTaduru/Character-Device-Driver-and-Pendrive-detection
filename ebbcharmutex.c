
#include <linux/init.h>           
#include <linux/module.h>         
#include <linux/device.h>         
#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <asm/uaccess.h>          
#include <linux/mutex.h>	  
#define  DEVICE_NAME "ebbchar"    
#define  CLASS_NAME  "ebb"        

MODULE_LICENSE("GPL");            
MODULE_DESCRIPTION("A simple Linux char driver for the BBB");  
MODULE_VERSION("0.1");        

static int    majorNumber;    
static char   message[256] = {0}; 
static short  size_of_message;   
static int    numberOpens = 0;   
static struct class*  ebbcharClass  = NULL; 
static struct device* ebbcharDevice = NULL; 

static DEFINE_MUTEX(ebbchar_mutex);	    

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init ebbchar_init(void){
   printk(KERN_INFO "EBBChar: Initializing the EBBChar LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "EBBChar failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", majorNumber);

   ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(ebbcharClass)){           // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(ebbcharClass);     // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "EBBChar: device class registered correctly\n");

   // Register the device driver
   ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(ebbcharDevice)){          // Clean up if there is an error
      class_destroy(ebbcharClass);      // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(ebbcharDevice);
   }
   printk(KERN_INFO "EBBChar: device class created correctly\n"); 
   mutex_init(&ebbchar_mutex);          // Initialize the mutex dynamically
   return 0;
}

static void __exit ebbchar_exit(void){
   mutex_destroy(&ebbchar_mutex);                       
   device_destroy(ebbcharClass, MKDEV(majorNumber, 0)); 
   class_unregister(ebbcharClass);                      
   class_destroy(ebbcharClass);                         
   unregister_chrdev(majorNumber, DEVICE_NAME);         
   printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep){

   if(!mutex_trylock(&ebbchar_mutex)){                  
	printk(KERN_ALERT "EBBChar: Device in use by another process");
	return -EBUSY;
   }
   numberOpens++;
   printk(KERN_INFO "EBBChar: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){           
      printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0); // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;      // Failed -- return a bad address message (i.e. -14)
   }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){

   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "EBBChar: Received %s %zu characters from the user\n",message, len);
   return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
   mutex_unlock(&ebbchar_mutex);                      // release the mutex (i.e., lock goes up)
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);
