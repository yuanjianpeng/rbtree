/*
 * red black tree in c language
 * 2024/5, Yuan Jianpeng <yuanjp89@163.com>
 * Ref https://linuxdev.cc/article/a0gffr.html
 */

#ifndef RBTREE_H
#define RBTREE_H

#define RB_RED 0
#define RB_BLACK 1

struct rb_node {
	unsigned long parent;
	struct rb_node *left;
	struct rb_node *right;
};

struct rb_tree {
	struct rb_node *root;
};

#define rb_parent(n) ((struct rb_node *)(n->parent & ~ 3))
#define rb_color(n) ((n)->parent & 1)

static int inline rb_empty(struct rb_tree *tree)
{
	return tree->root == (void *)0;
}

void rb_init(struct rb_tree *tree);

struct rb_node *rb_first(struct rb_tree *tree);

struct rb_node *rb_next(struct rb_node *node);

struct rb_node *rb_next_from(struct rb_tree *tree, const void *key,
		int (*cmp)(struct rb_node *, const void *));

struct rb_node *rb_find(struct rb_tree *tree, const void *key,
		int (*cmp)(struct rb_node *, const void *));

int rb_insert(struct rb_tree *tree, struct rb_node *node,
		int (*cmp)(struct rb_node *, struct rb_node *));

void rb_delete(struct rb_tree *tree, struct rb_node *node);

#define rb_for_each(node, tree)	\
	for (node = rb_first(tree); node; node = rb_next(node))

#define rb_entry(node, type, member) container_of(node, type, member)

struct rb_node *rb_first_postorder(struct rb_tree *tree);
struct rb_node *rb_next_postorder(struct rb_node *node);

#define rb_entry_safe(ptr, type, member) \
        ({ typeof(ptr) ____ptr = (ptr); \
	           ____ptr ? rb_entry(____ptr, type, member) : NULL; \
		           })

#define rb_for_each_entry(pos, tree, member)	\
	for (pos = rb_entry_safe(rb_first(tree), typeof(*pos), member); \
		pos; \
		pos = rb_entry_safe(rb_next(&pos->member), typeof(*pos), member))

#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
	for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
			pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
					typeof(*pos), field); 1; }); \
					pos = n)

#endif

