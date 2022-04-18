jst = -16
nj = 0
njv = 0
import numpy
mat = numpy.zeros([5016, 15681], dtype=bool)

class cache_stat:
    def __init__(self, n, m):
        self.tags = [-1 for i in range(n)]
        self.n = n
        self.m = m
        self.miss = 0
    def check(self, j):
        gpage = j // self.m
        ipage = gpage % self.n
        if self.tags[ipage] != gpage:
            self.tags[ipage] = gpage
            self.miss += 1
cachei1_256 = cache_stat(128, 16)
cachei1_512 = cache_stat(256, 16)
for line in open("neigh.txt"):
    i, j = map(int, line.split())
    nj += 1
    if j >= jst + 8 or j < jst:
        njv += 1
        jst = j & ~7
    cachei1_256.check(j)
    cachei1_512.check(j)
    mat[i,j] = True
njtv = 0
list2 = []
cachei8_256 = cache_stat(128, 16)
cachei8_512 = cache_stat(256, 16)

for ist in range(0, 5016, 8):
    for i in range(1, 8):
        mat[ist] = mat[ist] + mat[ist + i]
    list2.append([])
    for j in range(15681):
        if mat[ist, j]:
            list2[-1].append(j)
            cachei8_256.check(j)
            cachei8_512.check(j)
    njtv += numpy.sum(mat[ist])
print("%d %d %d" % (nj, njv, njtv))

print("using i1: %d %d" % (cachei1_256.miss, cachei1_512.miss))
print("using i8: %d %d" % (cachei8_256.miss, cachei8_512.miss))

# 邻接表存储：i // 8 -> (j,jmask)... j: j原子序号，mask -> 8bit，表示j与8个i中的哪些有临接关系
# for (ist = 0; ist < nlocal; ist += 8){
#   jlist = neigh_new[ist >> 3];
#   for (i = 0; i < 8; i ++){ //向量化
#     if (mask & 1 << i) ILP(ist + i, j)
#   }
# }
