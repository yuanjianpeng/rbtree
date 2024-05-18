/*
 * =============================================================================
 *
 *       Filename:  rbtree-tst.c
 *
 *    Description:  rbtree testcase.
 *
 *        Created:  09/02/2012 11:39:34 PM
 *
 *         Author:  Fu Haiping (forhappy), haipingf@gmail.com
 *        Company:  ICT ( Institute Of Computing Technology, CAS )
 *
 * =============================================================================
 */

#include "rbtree-kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct mynode {
	struct rb_node node;
	int v;
};

struct rb_root mytree;

struct mynode *kernel_root()
{
	return (struct mynode *)mytree.rb_node;
}

void kernel_init()
{
	mytree = RB_ROOT;
}

void kernel_del(struct mynode *data)
{
	rb_erase(&data->node, &mytree);
	data->v = 0;
}

int kernel_insert(struct mynode *data)
{
  	struct rb_node **new = &(mytree.rb_node), *parent = NULL;

  	/* Figure out where to put new node */
  	while (*new) {
  		struct mynode *this = container_of(*new, struct mynode, node);
  		int result = data->v - this->v;

		parent = *new;
  		if (result < 0)
  			new = &((*new)->rb_left);
  		else if (result > 0)
  			new = &((*new)->rb_right);
  		else
  			return 0;
  	}

  	/* Add new node and rebalance tree. */
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, &mytree);

	return 1;
}

