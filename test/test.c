#include <stdio.h>
#include <stdlib.h>
#include "../rbtree.h"
#include <string.h>

#define N 200
#define M 1000

struct rb_tree tree;

struct rb_node2
{
	unsigned long  rb_parent_color;
	struct rb_node *right;
	struct rb_node *left;
};

struct my_node {
	struct rb_node node;
	int v;
};

struct kernel_node
{
	struct rb_node2 node;
	int v;
};

int cmp(struct rb_node *l, struct rb_node *r)
{
	return ((struct my_node *)r)->v - ((struct my_node *)l)->v;
}

struct my_node my_nodes[N];
struct kernel_node kernel_nodes[N];
int val[N];
int del[N];

void kernel_init();
int kernel_insert(struct kernel_node *data);
void kernel_del(struct kernel_node *data);
struct kernel_node *kernel_root();

#define kernel_parent(r)   ((struct kernel_node *)((r)->rb_parent_color & ~3))

int check(int n)
{
	int i;
	int ret = 0;
	int my = 0, k = 0;

	printf("======================\n");

	if (tree.root)
		my = ((struct my_node *)tree.root)->v;
	if (kernel_root())
		k = kernel_root()->v;

	printf("my root %d\n", my);
	printf("kn root %d\n", k);

	if (my != k)
		ret = -1;

	for (i = 0; i < n; i++) {
		int ml = 0, mr = 0, kl = 0, kr = 0;
		int mp = 0, kp = 0;
		int my_color = my_nodes[i].node.color;
		int kn_color = 1 - (kernel_nodes[i].node.rb_parent_color & 1);

		if (my_nodes[i].v == 0 && kernel_nodes[i].v == 0)
			continue;

		printf("===\n");

		if (my_nodes[i].node.parent)
			mp = ((struct my_node *)(my_nodes[i].node.parent))->v;
		if (my_nodes[i].node.left)
			ml = ((struct my_node *)(my_nodes[i].node.left))->v;
		if (my_nodes[i].node.right)
			mr = ((struct my_node *)(my_nodes[i].node.right))->v;

		printf("my node %d, p %d left %d right %d red %d\n", my_nodes[i].v, mp, ml, mr, my_color);

		if (kernel_parent(&kernel_nodes[i].node))
			kp = kernel_parent(&kernel_nodes[i].node)->v;
		if (kernel_nodes[i].node.left)
			kl = ((struct kernel_node *)(kernel_nodes[i].node.left))->v;
		if (kernel_nodes[i].node.right)
			kr = ((struct kernel_node *)(kernel_nodes[i].node.right))->v;

		printf("kn node %d, p %d left %d right %d red %d\n", kernel_nodes[i].v, kp, kl, kr, kn_color);

		if (ml != kl || mr != kr || my_color != kn_color || mp != kp) {
			fprintf(stderr, "\tdiffer\n");
			ret = -1;
		}
	}

	return ret;
}

int main()
{
	int n = 0;
	int i;
	int ret;

	while (n++ < M) {

		n++;
		fprintf(stderr, "run %d times\n", n);

		for (i = 0; i < N; i++) {
			val[i] = i + 1;
			del[i] = i;
		}

		for (i = 0; i < N; i++) {
			int a = rand() % N, b = rand() % N;
			int c = val[a], d = val[b];
			val[a] = d;
			val[b] = c;
		}

		for (i = 0; i < N; i++) {
			int a = rand() % N, b = rand() % N;
			int c = del[a], d = del[b];
			del[a] = d;
			del[b] = c;
		}

		memset(my_nodes, 0, sizeof(my_nodes));
		memset(kernel_nodes, 0, sizeof(kernel_nodes));

		for (i = 0; i < N; i++) {
			my_nodes[i].v = val[i];
			kernel_nodes[i].v = val[i];
		}

		rb_init(&tree, cmp);
		kernel_init();

		for (i = 0; i < N; i++) {

			ret = kernel_insert(&kernel_nodes[i]);

			ret = rb_insert(&tree, &my_nodes[i].node);
			if (ret != 1)
				fprintf(stderr, "insert failed\n");

			ret = check(i+1);
			if (ret < 0) {
				fprintf(stderr, "check %d failed\n", i+1);
				return 1;
			}
		}

		for (i = 0; i < N; i++) {
			int j = del[i];
			printf("del %d\n", my_nodes[j].v);
			rb_delete(&tree, &my_nodes[j].node);
			my_nodes[j].v = 0;
			kernel_del(&kernel_nodes[j]);
			kernel_nodes[j].v = 0;
			ret = check(N);
			if (ret < 0) {
				fprintf(stderr, "check failed\n");
				return 1;
			}
		}
	}
 
	fprintf(stderr, "passed\n");
	return 0;
}

