#ifdef __sw_host__
#include "parallel_eval.h"
#include <mpi.h>
static reduce_req_t *first_req = NULL;
void push_reduce_req(reduce_req_t *req){
  req->next = first_req;
  first_req = req;
}
void flush_reduce_req() {
  for (reduce_req_t *req = first_req; req; req = req->next){
    if (req->all){
      MPI_Allreduce(req->send_buf, req->recv_buf, req->count, req->type, req->op, req->comm);
    } else {
      MPI_Reduce(req->send_buf, req->recv_buf, req->count, req->type, req->op, req->root, req->comm);
    }
  }
  first_req = NULL;
}
#endif
