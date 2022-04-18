#include <cstdlib>
#include <execinfo.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <ucontext.h>

static int out_fd;
static void **out_buffer;
static int buffer_ptr;
static int buffer_cnt = 1024 * 16;
static timer_t timerid;
static jmp_buf portal;
static struct sigaction act;
static int in_alrm = 0;
static int memmgmt = 0, memmgmt_cnt = 0;
extern int pool_malloc;
// #ifdef __OPEN64__
// #include <swlu.h>
// //int swlu_backtrace(void **buffer, int size);
// #define backtrace(x, y) swlu_backtrace_context(context, x, y)
// #endif
void flush_buffer(){
  write(out_fd, out_buffer, buffer_ptr * sizeof(void*));
  buffer_ptr = 0;
}
void sig_segv(int a){
  longjmp(portal, 1);
}
void sig_alrm_prof(int a, siginfo_t *info, void *context){

  if (memmgmt) {
    out_buffer[buffer_ptr++] = (void*)malloc;
    out_buffer[buffer_ptr++] = 0;
    memmgmt_cnt += 1;
    return;
  }
  void **buffer = out_buffer + buffer_ptr;
  //#ifndef __OPEN64__
  if (in_alrm) return;
  in_alrm = 1;
  int error = setjmp(portal);
  int ntrace;
  if (!error){
    ntrace = backtrace(buffer, 100);
  } else {
    ntrace = 0;
  }
  buffer[ntrace] = 0;
  buffer_ptr += ntrace + 1;
  if (buffer_ptr + 101 >= buffer_cnt) {
    flush_buffer();
  }
  in_alrm = 0;
}
void sig_alrm_stub(int a, siginfo_t *info, void *context){
}

void init_prof(const char *path){
  out_buffer = (void**)malloc(buffer_cnt * sizeof(void *));
  buffer_ptr = 0;
  out_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
  struct sigevent sev;
  struct itimerspec its;
  
  timer_create(CLOCK_REALTIME, NULL, &timerid);
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1000000;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 1000000;
  timer_settime(timerid, 0, &its, NULL);

  stack_t sigstk;
  sigstk.ss_size = 0;
  sigstk.ss_flags = 0;
  sigstk.ss_sp = malloc (SIGSTKSZ);
  if (sigstk.ss_sp != NULL) {
    sigstk.ss_size = SIGSTKSZ;
    if (sigaltstack (&sigstk, 0) < 0) {
      sigstk.ss_size = 0;
      free (sigstk.ss_sp);
      perror("sigaltstack error");
    }
  } else {
    printf ("malloc (SIGSTKSZ) failed!\n");
  }
  sigaction(SIGALRM, NULL, &act);
  //act.sa_flags = SA_ONSTACK | SA_NODEFER;
  act.sa_flags |= SA_NODEFER | SA_ONSTACK | SA_SIGINFO;
  //act.sa_flags &= ~SA_NODEFER;
  act.sa_sigaction = sig_alrm_stub;
  sigaction(SIGALRM, &act, NULL);

  //signal(SIGALRM, sig_alrm_stub);
}
void start_prof(){
  //signal(SIGALRM, sig_alrm_prof);
  act.sa_sigaction = sig_alrm_prof;
  sigaction(SIGALRM, &act, NULL);

}
void pause_prof(){
  //signal(SIGALRM, sig_alrm_stub);
  act.sa_sigaction = sig_alrm_stub;
  sigaction(SIGALRM, &act, NULL);
}
void init_prof_(int *id) {
  char path[100];
  strcpy(path, "backtrace.");
  int iid = *id;
  int ptr = strlen(path);
  path[ptr + 0] = '0' + iid / 100000;
  path[ptr + 1] = '0' + iid % 100000 / 10000;
  path[ptr + 2] = '0' + iid % 10000 / 1000;
  path[ptr + 3] = '0' + iid % 1000 / 100;
  path[ptr + 4] = '0' + iid % 100 / 10;
  path[ptr + 5] = '0' + iid % 10;
  path[ptr + 6] = 0;
  strcat(path, ".bin");
  init_prof(path);
}
void stop_prof(){
  timer_delete(timerid);
  flush_buffer();
}
extern "C"{
  enum mem_mgmt_op{
    OP_MALLOC = 1,
    OP_FREE = 2
  };
  typedef struct mpool {
    size_t entry_size;
    char *pool_st, *pool_ed;
    char **slots_avail;
    size_t slot_ptr;
  } mpool_t;

  mpool_t pool = {0, NULL, NULL, NULL, 0};
  void init_pool(size_t esize, size_t nents){
    pool.pool_st = (char*)malloc(esize * nents);
    pool.pool_ed = pool.pool_st + esize * nents;
    pool.slots_avail = (char**)malloc(sizeof(char*) * nents);
    for (size_t i = 0; i < nents; i ++){
      pool.slots_avail[i] = pool.pool_st + esize * i;
    }
    pool.slot_ptr = nents;
    pool.entry_size = esize;
  }
  void *pool_allocate(){
    if (pool.slot_ptr > 0){
      return pool.slots_avail[--pool.slot_ptr];
    } else {
      puts("pool overflow, giving up");
      return NULL;
    }
  }
  int pool_deallocate(void *ptr){
    char *cptr = (char*)ptr;
    
    if (cptr >= pool.pool_st && cptr < pool.pool_ed) {
      //printf("%p %p %p\n", ptr, pool.pool_st, pool.pool_ed);
      //puts("returning to pool");
      pool.slots_avail[pool.slot_ptr ++] = cptr;
      return 1;
    } else {
      return 0;
    }
  }

  static int mlog_enable = 0;
  void init_mlog(const char *path){
    out_buffer = (void**)malloc(buffer_cnt * sizeof(void *));
    buffer_ptr = 0;
    out_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
    mlog_enable = 1;
  }
  void init_mlog_(int *id) {
    char path[100];
    strcpy(path, "mlog.");
    int iid = *id;
    int ptr = strlen(path);
    path[ptr + 0] = '0' + iid / 100000;
    path[ptr + 1] = '0' + iid % 100000 / 10000;
    path[ptr + 2] = '0' + iid % 10000 / 1000;
    path[ptr + 3] = '0' + iid % 1000 / 100;
    path[ptr + 4] = '0' + iid % 100 / 10;
    path[ptr + 5] = '0' + iid % 10;
    path[ptr + 6] = 0;
    strcat(path, ".bin");
    init_mlog(path);
  }
  void stop_mlog(){
    flush_buffer();
  }
  void log_mem(void *ptr, size_t size, int operation){
    if (mlog_enable != 1 || memmgmt > 1) return;
    void **buffer = out_buffer + buffer_ptr;
    int error = setjmp(portal);
    int ntrace;
    if (!error){
      ntrace = backtrace(buffer, 100);
    } else {
      ntrace = 0;
    }
    buffer[ntrace++] = ptr;
    buffer[ntrace++] = (void*)(size | (long)operation << 48);
    buffer[ntrace++] = 0;
    buffer_ptr += ntrace;
    if (buffer_ptr + 103 >= buffer_cnt) {
      flush_buffer();
    }
  }
  void *__real_malloc(size_t size);
  void __real_free(void *ptr);
  void *__real_realloc(void *__ptr, size_t __size);
  void *__real_calloc(size_t __nmemb, size_t __size);
  void *try_pool_malloc(size_t size){
    if (pool.entry_size > size){
      void *ret = pool_allocate();
      //printf("pool_allocate: %p %ld %ld\n", ret, pool.entry_size, size);
      if (ret) return ret;
    }
    return __real_malloc(size);
  }
  extern void try_pool_free(void *ptr){
    //printf("pool_free: %p\n", ptr);
    if (pool_deallocate(ptr)) return;
    __real_free(ptr);
  }
  void *__wrap_malloc(size_t size){
    memmgmt ++;
    //void *ret = __real_malloc(size);
    void *ret = try_pool_malloc(size);
    log_mem(ret, size, OP_MALLOC);
    memmgmt --;
    return ret;
  }
  void __wrap_free(void *ptr) {
    memmgmt ++;
    log_mem(ptr, 0, OP_FREE);
    //__real_free(ptr);
    try_pool_free(ptr);
    memmgmt --;
  }
  void *__wrap_realloc(void *__ptr, size_t __size){
    memmgmt ++;
    log_mem(__ptr, 0, OP_FREE);
    if ((char*)__ptr >= pool.pool_st && (char*)__ptr < pool.pool_ed){
      if (__size < pool.entry_size) return __ptr;
      char *ret = (char*)malloc(__size);
      memcpy(ret, __ptr, pool.entry_size);
      return ret;
      //puts("Realloc of ptr from pool");
      //exit(1);
    }
    void *ret = __real_realloc(__ptr, __size);
    log_mem(ret, __size, OP_MALLOC);
    memmgmt --;
    return ret;
  }
  void *__wrap_calloc(size_t __nmemb, size_t __size){
    memmgmt ++;
    void *ret = __real_calloc(__nmemb, __size);
    log_mem(ret, __nmemb * __size, OP_MALLOC);
    memmgmt --;
    return ret;
  }
}
// void stop_prof_() __attribute__((alias("stop_prof")));
// void start_prof_() __attribute__((alias("start_prof")));
// void pause_prof_() __attribute__((alias("pause_prof")));

#ifdef TEST
int main(){
  int id = 0;
  init_prof_(&id);
  start_prof();
  for (int i = 0; i < 100000; i ++){
    printf("%d\n", i);
  }
  pause_prof();
  stop_prof();
  return 0;
}
#endif
