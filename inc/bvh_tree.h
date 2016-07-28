#pragma once

#include "bvh_node.h"
#include "surface.h"

namespace plu {

	namespace surfaces {

		struct bvh_tree : public surface {
			bvh_node root;

			bvh_tree(vector<shared_ptr<surface>>& s //had to leave here) {}
				//need to sort surface list somehow
				//set left node == bvh_tree of left list, right == bvh_tree of right list
				//recursive base case: list size == 0, 1, or 2
				//set parent of both nodes == root
		}
	}
}
