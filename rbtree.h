/*
 * red black tree in c language
 * 2024/5, Yuan Jianpeng <yuanjp89@163.com>
 * Ref https://linuxdev.cc/article/a0gffr.html
 */

#ifndef RBTREE_H
#define RBTREE_H

#define RB_BLACK 0
#define RB_RED 1

struct rb_node {
	int color;
	struct rb_node *parent;
	struct rb_node *left;
	struct rb_node *right;
};

struct rb_tree {
	struct rb_node *root;
	int (*cmp)(struct rb_node *, struct rb_node *);
};

void rb_init(struct rb_tree *tree, int (*cmp)(struct rb_node *, struct rb_node *));

struct rb_node *rb_find(struct rb_tree *tree, struct rb_node *key);

int rb_insert(struct rb_tree *tree, struct rb_node *node);

void rb_delete(struct rb_tree *tree, struct rb_node *node);

#endif

