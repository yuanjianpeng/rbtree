
objs := test.o rbtree.o rbtree-kernel-tst.o rbtree-kernel.o

VPATH := ../
CFLAGS := -O0 -fprofile-arcs -ftest-coverage -fPIC -O0

a.out: ${objs}
	cc $(CFLAGS) -o a.out ${objs}
clean:
	rm -fr *.o *.gcov *gcda *gcno a.out

