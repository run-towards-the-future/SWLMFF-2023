#include <cstdlib>
void init_prof(const char *path);
void start_prof();
void pause_prof();
void init_prof_(int *id);
void stop_prof();
extern "C"{
  void stop_mlog();
  void init_mlog_(int *id);
  void init_mlog(const char *path);
  void init_pool(size_t esize, size_t nents);
}
