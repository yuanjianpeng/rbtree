#include "rbtree.h"
#include <stddef.h>

void rb_init(struct rb_tree *tree, int (*cmp)(struct rb_node *, struct rb_node *))
{
	tree->root = NULL;
	tree->cmp = cmp;
}

struct rb_node *rb_find(struct rb_tree *tree, struct rb_node *key)
{
	struct rb_node *node = tree->root;
	int ret;

	while (node) {
		ret = tree->cmp(node, key);
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
	if (old == tree->root)
		tree->root = new;
	else if (old == old->parent->left)
		old->parent->left = new;
	else
		old->parent->right = new;
	if (new)
		new->parent = old->parent;
}

static void inline rotate(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *p = x->parent;
	struct rb_node *g = p->parent;

	if (p == g->left) {
		// Left left case
		if (x == p->left) {
			p->color = RB_BLACK;
			g->color = RB_RED;
			g->left = p->right;
			if (p->right)
				p->right->parent = g;
			p->right = g;
			replace(tree, g, p);
			g->parent = p;
		}

		// Left right case
		else {
			x->color = RB_BLACK;
			g->color = RB_RED;
			p->right = x->left;
			if (x->left)
				x->left->parent = p;
			g->left = x->right;
			if (x->right)
				x->right->parent = g;
			x->left = p;
			x->right = g;
			replace(tree, g, x);
			p->parent = g->parent = x;
		}
	}

	else {
		// Right left case
		if (x == p->left) {
			x->color = RB_BLACK;
			g->color = RB_RED;
			p->left = x->right;
			if (x->right)
				x->right->parent = p;
			g->right = x->left;
			if (x->left)
				x->left->parent = g;
			x->left = g;
			x->right = p;
			replace(tree, g, x);
			p->parent = g->parent = x;
		}

		// Right right case
		else {
			p->color = RB_BLACK;
			g->color = RB_RED;
			g->right = p->left;
			if (p->left)
				p->left->parent = g;
			p->left = g;
			replace(tree, g, p);
			g->parent = p;
		}
	}
}

int rb_insert(struct rb_tree *tree, struct rb_node *node)
{
	struct rb_node **tmp = &tree->root;
	struct rb_node *parent = NULL;
	int ret;

	// make the colour of newly inserted nodes as RED
	node->color = RB_RED;
	node->left = NULL;
	node->right = NULL;

	// Perform standard BST insertion
	while (*tmp) {
		parent = *tmp;
		ret = tree->cmp(parent, node);
		if (ret < 0)
			tmp = &parent->left;
		else if (ret > 0)
			tmp = &parent->right;
		else
			return 0;
	}
	*tmp = node;
	node->parent = parent;

	// Condition 1, If x is the root, change the colour of x as BLACK
	if (tree->root == node) {
		node->color = RB_BLACK;
		return 1;
	}

	// Condition 2, If parent is BLACK, insert done

	// Condition 3, parent is red
	while (parent->color == RB_RED) {
		struct rb_node *uncle, *grandpa;

		grandpa = parent->parent;
		uncle = (parent == grandpa->left) ? grandpa->right : grandpa->left;

		// 3.1 uncle is red
		if (uncle && uncle->color == RB_RED) {

			// change parent and uncle to BLACK
			parent->color = RB_BLACK;
			uncle->color = RB_BLACK;
			
			// 3.1.1 grandpa is root, insert done
			if (grandpa == tree->root)
				break;

			// 3.1.2 grandpa is not root, change its color to RED
			grandpa->color = RB_RED;

			// set x to grandpa, and continue to check
			node = grandpa;
			parent = node->parent;

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
		int color = x->color;

		m = x->right;	// m is right child
		n = m;
		while (n->left)
			n = n->left; // n is leftmost node

		x->color = n->color;
		n->color = color;

		n->left = x->left;
		if (x->left)
			x->left->parent = n;
		x->left = NULL;
		x->right = n->right;

		n->right = m;
		m->parent = n;

		m = n->parent;	// cache leftmost's parent to m
		replace(tree, x, n);

		if (n->right == n) {	// leftmost is x's right child
			n->right = x;
			x->parent = n;
		}
		else {
			m->left = x;
			x->parent = m;
		}

		// fallthrough to process leftmost deletion
	}

	// Condition 2, has only one child
	if (x->left || x->right)
	{
		m = x->left ? : x->right;
		replace(tree, x, m);

		// if any is red, delete done
		if (x->color == RB_RED || m->color == RB_RED) {
			m->color = RB_BLACK;
			return;
		}

		// if both is black, fallthrough to fixup
		p = m;
		s = NULL;
	}

	// Condition 1, no child
	else {
		p = x->parent;
		if (p)
			s = (x == p->left) ? p->right : p->left;

		replace(tree, x, NULL);

		// 1.1 the deleted node is red, delete done
		if (x->color == RB_RED)
			return;

		// 1.2 the deleted node is black, fallthrough to fixup
	}

	// rotating and recoloring
	while (p) {
		// sibling is black
		if (!s || s->color == RB_BLACK)
		{
			// b) its both children are black
			if (!s || ((!s->left || s->left->color == RB_BLACK) &&
					(!s->right || s->right->color == RB_BLACK)))
			{
				if (s)
					s->color = RB_RED;

				if (p->color == RB_RED) {
					p->color = RB_BLACK;
					return;
				}

				if (p->parent)
					s = (p == p->parent->left) ? p->parent->right : p->parent->left;
				p = p->parent;
				continue;
			}

			// a) at least one child is red
			if (s == p->right) {
				if (s->right && s->right->color == RB_RED) {
					s->right->color = RB_BLACK;
					s->color = p->color;
					p->color = RB_BLACK;
					p->right = s->left;
					if (s->left)
						s->left->parent = p;
					s->left = p;
					replace(tree, p, s);
					p->parent = s;
				}
				else {
					m = s->left;
					m->color = p->color;
					p->color = RB_BLACK;
					replace(tree, p, m);
					p->right = m->left;
					if (m->left)
						m->left->parent = p;
					s->left = m->right;
					if (m->right)
						m->right->parent = s;
					m->left = p;
					p->parent = m;
					m->right = s;
					s->parent = m;
				}
			}
			else {
				if (s->left && s->left->color == RB_RED) {
					s->left->color = RB_BLACK;
					s->color = p->color;
					p->color = RB_BLACK;
					p->left = s->right;
					if (s->right)
						s->right->parent = p;
					s->right = p;
					replace(tree, p, s);
					p->parent = s;
				}
				else {
					m = s->right;
					m->color = p->color;
					p->color = RB_BLACK;
					replace(tree, p, m);
					p->left = m->right;
					if (m->right)
						m->right->parent = p;
					s->right = m->left;
					if (m->left)
						m->left->parent = s;
					m->right = p;
					p->parent = m;
					m->left = s;
					s->parent = m;
				}
			}

			return;
		}

		else {
			// c) sibling is red
			replace(tree, p, s);

			p->parent = s;
			p->color = RB_RED;
			s->color = RB_BLACK;

			if (s == p->right) {
				p->right = s->left;
				if (s->left)
					s->left->parent = p;
				s->left = p;
				s = p->right; // update p s and continue
			}
			else {
				p->left = s->right;
				if (s->right)
					s->right->parent = p;
				s->right = p;
				s = p->left;
			}
		}
	}
}

