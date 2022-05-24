#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main()
{
    int result;
    long int ret = syscall(334, 1, 3, &result);
    printf("cs3753_add stores the sum of (1 + 3) = %d\n", result);
    if (ret != 0)
    {
        printf("System call failed\n");
    }
    return 0;
}