
// #include <qthread.h>
#include <stdio.h>
extern void slave_qthread_dummy();
extern void slave_puts();
int main()
{

    // puts("sp set");

    // puts("employed");
    // printf("%p\n", slave_qthread_init);
    puts("before init");
    qthread_init();
    puts("after init");
    long st, ed;
    asm volatile("rtc %0"
        : "=r"(st));
    for (int i = 0; i < 100; i ++){
        void* n = qthread_spawn(slave_qthread_dummy, "test");
        qthread_join();
    }
    asm volatile("rtc %0"
        : "=r"(ed));
    printf("%ld %ld %ld\n", ed - st, st, ed);
    puts("slave done");
}
