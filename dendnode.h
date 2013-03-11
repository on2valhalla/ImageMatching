#ifndef DENDNODE_H
#define DENDNODE_H

#include <string>
#include <list>

class DendNode
{

private:


public:
	double distance;
	int index;
	DendNode *left;
	DendNode *right;
	list<int> childIndicies;

	DendNode()
	{
		distance = -1;
		index = -1;
		left = NULL;
		right = NULL;
	}
	DendNode(int idx)
	{
		distance = -1;
		index = idx;
		left = NULL;
		right = NULL;
	}
	DendNode(const DendNode *l, const DendNode *r, double matchD)
	{
		distance = matchD;
		index = -1;
		left = l;
		right = r;
		childIndicies.insert(childIndicies.begin(), 
			l->childIndicies.begin(), l->childIndicies.end());
		childIndicies.insert(childIndicies.begin(), 
			r->childIndicies.begin(), r->childIndicies.end());
	}
	DendNode(const DendNode &node)
	{
		distance = node.distance;
		left = node.left;
		right = node.right;
		index = node.index;
		childIndicies = node.childIndicies;
	}

	~DendNode() 
	{
		delete left;
		delete right;
	}

	string toString()
	{
		stringstream sstm;
		if (left != NULL)
			sstm << left->toString()<< ":L::";
		sstm << index << "," << distance;
		if (right != NULL)
			sstm << "::R:" << right->toString();
		return sstm.str();
	}




};



std::list<int> getIndicies(DendNode *root)
{
	list<DendNode*> stack;
	list<int> indicies;
	stack.push_back(root);

	while(stack.size() > 0)
	{
		DendNode *node = stack.back();
		stack.pop_back();

		if (node->index != 1)
			indicies.push_back(node->index);

		if(node->right != NULL)
			stack.push_back(node->right);
		if(node->left != NULL)
			stack.push_back(node->left);
	}

	return indicies;
}





#endif // DENDNODE_H
