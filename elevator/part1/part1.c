#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    /* syscall() enters the kernel directly, avoiding cached libc results. */
    syscall(SYS_getpid);
    syscall(SYS_getppid);
    syscall(SYS_getuid);
    syscall(SYS_geteuid);
    syscall(SYS_getgid);

    return 0;
}
