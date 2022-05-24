#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 1024
#define DEVICE_NAME "pa2_char_device"
#define MAJOR_NUMBER 240

MODULE_LICENSE("GPL");


/* Define device_buffer and other global data structures you will need here */

char *device_buffer;
int openCount = 0;
int closeCount = 0;


ssize_t pa2_char_driver_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/* length is the length of the userspace buffer*/
	/* offset will be set to current position of the opened file after read*/
	/* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */

	int copyed;
	int readLength;
	int canRead = (BUFFER_SIZE - *offset);

	length = (canRead < length) ? canRead : length;

	copyed = copy_to_user(buffer, device_buffer + *offset, length);
	readLength = length - copyed;
	*offset += readLength;

	printk(KERN_ALERT "Read %d bytes.\n", readLength);

	return readLength;
}


ssize_t pa2_char_driver_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/* length is the length of the userspace buffer*/
	/* current position of the opened file*/
	/* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */

	int copyed;
	int writeLength;
	int canWrite = (BUFFER_SIZE - *offset);

	length = (canWrite < length) ? canWrite : length;

	copyed = copy_from_user(device_buffer + *offset, buffer, length);
	writeLength = length - copyed;
	*offset += writeLength;
	printk(KERN_ALERT "Wrote %d bytes.\n", writeLength);

	return writeLength;
}


int pa2_char_driver_open(struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/

	openCount++;

	return 0;
}


int pa2_char_driver_close(struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/

	closeCount++;
	printk(KERN_ALERT "Device has been closed %d time(s)\n", closeCount);

	return 0;
}


loff_t pa2_char_driver_seek(struct file *pfile, loff_t offset, int whence)
{
	/* Update open file position according to the values of offset and whence */

	loff_t position = 0;

	switch (whence)
	{
	case 0:
		position = offset;
		break;

	case 1:
		position = offset + pfile->f_pos;
		break;

	case 2:
		position = offset + BUFFER_SIZE;
		break;

	default:
		return EINVAL;
	}

	if (position < 0 || position >= BUFFER_SIZE)
	{
		return -1;
	}

	pfile->f_pos = position;
	return position;
}


struct file_operations pa2_char_driver_file_operations = {

	/* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/

	.owner = THIS_MODULE,
	.open = pa2_char_driver_open,
	.release = pa2_char_driver_close,
	.read = pa2_char_driver_read,
	.write = pa2_char_driver_write,
	.llseek = pa2_char_driver_seek
};


static int pa2_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	/* register the device */

	printk(KERN_ALERT "PA2 init-function called.\n");

	device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	memset(device_buffer, 0, BUFFER_SIZE);
	register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &pa2_char_driver_file_operations);
	return 0;
}


static void pa2_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the register_chrdev() function. */

	printk(KERN_ALERT "PA2 exit-function called.\n");
	kfree(device_buffer);

	unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
}


/* add module_init and module_exit to point to the corresponding init and exit function*/

module_init(pa2_char_driver_init);
module_exit(pa2_char_driver_exit);