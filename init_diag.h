#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <signal.h>

#ifdef __cplusplus
extern "C"{
#endif
#include <athread.h>
extern int PMPI_Init(int *argc, char ***argv);
static int diag_rank = -1;
static int in_coll = 0;
int pool_malloc = 0;
extern void report(int);
extern void report_coll(int);
int MPI_Init(int *argc, char ***argv) {
  diag_rank = -2;
  struct sigaction sa;
  sigaction(40, NULL, &sa);
  sa.sa_flags |= SA_NODEFER;
  sa.sa_flags &= ~SA_SIGINFO;
  sa.sa_handler = report;
  sigaction(40, &sa, NULL);
  sa.sa_handler = report_coll;
  sigaction(41, &sa, NULL);
  int ret = PMPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &diag_rank);
  return ret;
}

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  in_coll = 1;
  pool_malloc ++;
  int ret = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
  in_coll = 0;
  pool_malloc --;
  return ret;
}
int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  in_coll = 1;
  pool_malloc ++;
  int ret = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  in_coll = 0;
  pool_malloc --;
  return ret;
}
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
  in_coll = 1;
  pool_malloc ++;
  int ret = PMPI_Bcast(buffer, count, datatype, root, comm);
  in_coll = 0;
  pool_malloc --;
  return ret;
}
int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
  pool_malloc ++;
  int ret = PMPI_Sendrecv(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf, recvcount, recvtype, source, recvtag, comm, status);
  pool_malloc --;
  return ret;
}
int MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  pool_malloc ++;
  int ret = PMPI_Send(buf, count, datatype, dest, tag, comm);
  pool_malloc --;
  return ret;
}
void report_coll(int sig){
  if (in_coll == 1){
    printf("%d enter collection communication\n", diag_rank);
  } else {
    printf("%d not in collection communication\n", diag_rank);
  }
  fflush(stdout);
}
void report(int sig){
  char hostname[2048];
  gethostname(hostname, 2048);

  if (diag_rank == -2){
    printf("init_diag: cpu=%s cg=%d start init\n", hostname, current_array_id());
  }

  if (diag_rank == -1){
    printf("init_diag: cpu=%s cg=%d not init\n", hostname, current_array_id());
  }
  if (diag_rank != -1 && sig == 40){
    printf("init_diag: %d is initialized\n", diag_rank);
  }
  fflush(stdout);
}

 
#ifdef __cplusplus
}
#endif
