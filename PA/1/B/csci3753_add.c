#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_csci3753_add(int input1, int input2, int * result)
{
    int sum;
    printk(KERN_ALERT "Adding %d and %d\n", input1, input2);
    sum = input1 + input2;
    printk(KERN_ALERT "The result is %d\n", sum);
    
    copy_to_user(result, &sum, sizeof(sum));

    return 0;
}