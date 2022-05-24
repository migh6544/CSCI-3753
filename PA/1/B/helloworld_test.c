#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

// Code influnced by: https://medium.com/anubhav-shrimal/adding-a-hello-world-system-call-to-linux-kernel-dad32875872
int main()
{
         long int amma = syscall(333);
         printf("System call sys_hello returned %ld\n", amma);
         return 0;
}