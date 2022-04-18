ifeq ($(target), ocn)
CC=sw9gcc -D__sw_ocn__ -g
ALIGN=64
endif
ifeq ($(target), thl)
CC=sw5gcc -D__sw_thl__ -g
ALIGN=32
endif
OBJS=qthread_cpe.o qthread_mpe.o qthread_asm_mpe.o qthread_asm_cpe.o
ifeq ($(target), thl)
OBJS+=spc_mpe.o
endif
all: libqthread.a test
libqthread.a: $(OBJS)
	ar cr $@ $^
test: qthread_test.o $(OBJS)
	$(CC) -mhybrid $^ -o $@
%_cpe.o: %.c
	$(CC) -mslave -msimd -faddress_align=$(ALIGN) -I. -c $< -o $@ -O3
%_mpe.o: %.c
	$(CC) -mhost -msimd -faddress_align=$(ALIGN) -I. -c $< -o $@ -O3
%_cpe.o: %_cpe.S
	$(CC) -mslave -msimd -faddress_align=$(ALIGN) -c $< -o $@ -O3
%_mpe.o: %_mpe.S
	$(CC) -mhost -msimd -faddress_align=$(ALIGN) -c $< -o $@ -O3
%.o: %.c
	$(CC) -mhost -msimd -faddress_align=$(ALIGN) -I. -c $< -o $@ -O3
clean:
	-rm -f $(OBJS) test libqthread.a
