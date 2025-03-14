#include "rbtree.h"
#include <stddef.h>

static inline void rb_set_parent(struct rb_node *node, struct rb_node *parent)
{
	node->parent = rb_color(node) + (unsigned long)parent;
}

static inline void rb_set_color(struct rb_node *node, int color)
{
	node->parent = (node->parent & ~1) + color;
}

static inline void rb_set_parent_color(struct rb_node *node, struct rb_node *parent, int color)
{
	node->parent = (unsigned long)parent + color;
}

void rb_init(struct rb_tree *tree)
{
	tree->root = NULL;
}

// first is left most node
struct rb_node *rb_first(struct rb_tree *tree)
{
	struct rb_node *node = tree->root;
	if (!node)
		return NULL;
	while (node->left)
		node = node->left;
	return node;
}

struct rb_node *rb_next(struct rb_node *node)
{
	struct rb_node *parent;

	if (node->right) {
		node = node->right;
		while (node->left)
			node = node->left;
		return node;
	}

	while ((parent = rb_parent(node)) && node == parent->right)
		node = parent;

	return parent;
}

struct rb_node *rb_next_from(struct rb_tree *tree, const void *key,
		int (*cmp)(struct rb_node *, const void *))
{
	struct rb_node *node = tree->root, *want = NULL;
	int ret;

	while (node) {
		ret = cmp(node, key);
		if (ret < 0) {
			want = node;
			node = node->left;
		} else {
			node = node->right;
		}
	}

	return want;
}

// post order: left first, right next, self last
static struct rb_node *rb_left_deepest_node(struct rb_node *node)
{
	while (1) {
		if (node->left)
			node = node->left;
		else if (node->right)
			node = node->right;
		else
			return node;
	}
}

struct rb_node *rb_next_postorder(struct rb_node *node)
{
	struct rb_node *parent;

	if (!node)
		return NULL;

	parent = rb_parent(node);

	if (parent && node == parent->left && parent->right)
		return rb_left_deepest_node(parent->right);
	else
		return parent;
}

struct rb_node *rb_first_postorder(struct rb_tree *tree)
{
	struct rb_node *node = tree->root;

	if (!node)
		return NULL;

	return rb_left_deepest_node(node);
}

struct rb_node *rb_find(struct rb_tree *tree, const void *key,
		int (*cmp)(struct rb_node *, const void *))
{
	struct rb_node *node = tree->root;
	int ret;

	while (node) {
		ret = cmp(node, key);
		if (ret < 0)
			node = node->left;
		else if (ret > 0)
			node = node->right;
		else
			return node;
	}

	return NULL;
}

static inline void replace(struct rb_tree *tree, struct rb_node *old, struct rb_node *new)
{
	struct rb_node *parent = rb_parent(old);

	if (old == tree->root)
		tree->root = new;
	else if (old == parent->left)
		parent->left = new;
	else
		parent->right = new;
	if (new)
		rb_set_parent(new, parent);
}

static void inline rotate(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *p = rb_parent(x);
	struct rb_node *g = rb_parent(p);

	if (p == g->left) {
		// Left left case
		if (x == p->left) {
			rb_set_color(p, RB_BLACK);
			rb_set_color(g, RB_RED);
			g->left = p->right;
			if (p->right)
				rb_set_parent(p->right, g);
			p->right = g;
			replace(tree, g, p);
			rb_set_parent(g, p);
		}

		// Left right case
		else {
			rb_set_color(x, RB_BLACK);
			rb_set_color(g, RB_RED);
			p->right = x->left;
			if (x->left)
				rb_set_parent(x->left, p);
			g->left = x->right;
			if (x->right)
				rb_set_parent(x->right, g);
			x->left = p;
			x->right = g;
			replace(tree, g, x);
			rb_set_parent(p, x);
			rb_set_parent(g, x);
		}
	}

	else {
		// Right left case
		if (x == p->left) {
			rb_set_color(x, RB_BLACK);
			rb_set_color(g, RB_RED);
			p->left = x->right;
			if (x->right)
				rb_set_parent(x->right, p);
			g->right = x->left;
			if (x->left)
				rb_set_parent(x->left, g);
			x->left = g;
			x->right = p;
			replace(tree, g, x);
			rb_set_parent(p, x);
			rb_set_parent(g, x);
		}

		// Right right case
		else {
			rb_set_color(p, RB_BLACK);
			rb_set_color(g, RB_RED);
			g->right = p->left;
			if (p->left)
				rb_set_parent(p->left, g);
			p->left = g;
			replace(tree, g, p);
			rb_set_parent(g, p);
		}
	}
}

int rb_insert(struct rb_tree *tree, struct rb_node *node,
		int (*cmp)(struct rb_node *, struct rb_node *))
{
	struct rb_node **tmp = &tree->root;
	struct rb_node *parent = NULL;
	int ret;

	// make the colour of newly inserted nodes as RED
	node->left = NULL;
	node->right = NULL;

	// Perform standard BST insertion
	while (*tmp) {
		parent = *tmp;
		ret = cmp(parent, node);
		if (ret < 0)
			tmp = &parent->left;
		else if (ret > 0)
			tmp = &parent->right;
		else
			return 0;
	}
	*tmp = node;
	rb_set_parent_color(node, parent, RB_RED);

	// Condition 1, If x is the root, change the colour of x as BLACK
	if (tree->root == node) {
		rb_set_color(node, RB_BLACK);
		return 1;
	}

	// Condition 2, If parent is BLACK, insert done

	// Condition 3, parent is red
	while (rb_color(parent) == RB_RED) {
		struct rb_node *uncle, *grandpa;

		grandpa = rb_parent(parent);
		uncle = (parent == grandpa->left) ? grandpa->right : grandpa->left;

		// 3.1 uncle is red
		if (uncle && rb_color(uncle) == RB_RED) {

			// change parent and uncle to BLACK
			rb_set_color(parent, RB_BLACK);
			rb_set_color(uncle, RB_BLACK);

			// 3.1.1 grandpa is root, insert done
			if (grandpa == tree->root)
				break;

			// 3.1.2 grandpa is not root, change its color to RED
			rb_set_color(grandpa, RB_RED);

			// set x to grandpa, and continue to check
			node = grandpa;
			parent = rb_parent(node);

			continue;
		}

		// 3.2 uncle is BLACK, we need recoloring and rotating
		rotate(tree, node);

		// after recoloring and rotating, the tree is balanced
		break;
	}

	return 1;
}

void rb_delete(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *s, *p;
	struct rb_node *m, *n;

	// Conditon 3, the deleted node has 2 childs
	if (x->left && x->right)
	{
		int color = rb_color(x);

		m = x->right;	// m is right child
		n = m;
		while (n->left)
			n = n->left; // n is leftmost node

		rb_set_color(x, rb_color(n));
		rb_set_color(n, color);

		n->left = x->left;
		if (x->left)
			rb_set_parent(x->left, n);
		x->left = NULL;
		x->right = n->right;

		n->right = m;
		rb_set_parent(m, n);

		m = rb_parent(n);	// cache leftmost's parent to m
		replace(tree, x, n);

		if (n->right == n) {	// leftmost is x's right child
			n->right = x;
			rb_set_parent(x, n);
		}
		else {
			m->left = x;
			rb_set_parent(x, m);
		}

		// fallthrough to process leftmost deletion
	}

	// Condition 2, has only one child
	if (x->left || x->right)
	{
		m = x->left ? : x->right;
		replace(tree, x, m);

		// if any is red, delete done
		if (rb_color(x) == RB_RED || rb_color(m) == RB_RED) {
			rb_set_color(m, RB_BLACK);
			return;
		}

		// if both is black, fallthrough to fixup
		p = m;
		s = NULL;
	}

	// Condition 1, no child
	else {
		p = rb_parent(x);
		if (p)
			s = (x == p->left) ? p->right : p->left;
		else
			s = NULL;

		replace(tree, x, NULL);

		// 1.1 the deleted node is red, delete done
		if (rb_color(x) == RB_RED)
			return;

		// 1.2 the deleted node is black, fallthrough to fixup
	}

	// rotating and recoloring
	while (p) {
		// sibling is black
		if (!s || rb_color(s) == RB_BLACK)
		{
			// b) its both children are black
			if (!s || ((!s->left || rb_color(s->left) == RB_BLACK) &&
					(!s->right || rb_color(s->right) == RB_BLACK)))
			{
				struct rb_node *parent;

				if (s)
					rb_set_color(s, RB_RED);

				if (rb_color(p) == RB_RED) {
					rb_set_color(p, RB_BLACK);
					return;
				}

				parent = rb_parent(p);
				if (parent)
					s = (p == parent->left) ? parent->right : parent->left;
				p = parent;
				continue;
			}

			// a) at least one child is red
			if (s == p->right) {
				if (s->right && rb_color(s->right) == RB_RED) {
					rb_set_color(s->right, RB_BLACK);
					rb_set_color(s, rb_color(p));
					rb_set_color(p, RB_BLACK);
					p->right = s->left;
					if (s->left)
						rb_set_parent(s->left, p);
					s->left = p;
					replace(tree, p, s);
					rb_set_parent(p, s);
				}
				else {
					m = s->left;
					rb_set_color(m, rb_color(p));
					rb_set_color(p, RB_BLACK);
					replace(tree, p, m);
					p->right = m->left;
					if (m->left)
						rb_set_parent(m->left, p);
					s->left = m->right;
					if (m->right)
						rb_set_parent(m->right, s);
					m->left = p;
					rb_set_parent(p, m);
					m->right = s;
					rb_set_parent(s, m);
				}
			}
			else {
				if (s->left && rb_color(s->left) == RB_RED) {
					rb_set_color(s->left, RB_BLACK);
					rb_set_color(s, rb_color(p));
					rb_set_color(p, RB_BLACK);
					p->left = s->right;
					if (s->right)
						rb_set_parent(s->right, p);
					s->right = p;
					replace(tree, p, s);
					rb_set_parent(p, s);
				}
				else {
					m = s->right;
					rb_set_color(m, rb_color(p));
					rb_set_color(p, RB_BLACK);
					replace(tree, p, m);
					p->left = m->right;
					if (m->right)
						rb_set_parent(m->right, p);
					s->right = m->left;
					if (m->left)
						rb_set_parent(m->left, s);
					m->right = p;
					rb_set_parent(p, m);
					m->left = s;
					rb_set_parent(s, m);
				}
			}

			return;
		}

		else {
			// c) sibling is red
			replace(tree, p, s);

			rb_set_parent(p, s);
			rb_set_color(p, RB_RED);
			rb_set_color(s, RB_BLACK);

			if (s == p->right) {
				p->right = s->left;
				if (s->left)
					rb_set_parent(s->left, p);
				s->left = p;
				s = p->right; // update p s and continue
			}
			else {
				p->left = s->right;
				if (s->right)
					rb_set_parent(s->right, p);
				s->right = p;
				s = p->left;
			}
		}
	}
}

