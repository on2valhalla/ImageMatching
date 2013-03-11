#ifndef DENDNODE_H
#define DENDNODE_H

#include <string>
#include <list>

class DendNode
{

private:

    int numChildren;

public:
	double distance;
	int index;
    DendNode *left;
    DendNode *right;

	DendNode()
	{
		distance = 0;
		numChildren = 0;
		index = -1;
		left = NULL;
		right = NULL;
    }
    DendNode(const DendNode *node)
    {
        distance = node->distance;
        numChildren = node->numChildren;
        left = node->left;
        right = node->right;
        index = node->index;
    }
    DendNode(const DendNode &node)
    {
        distance = node.distance;
        numChildren = node.numChildren;
        left = node.left;
        right = node.right;
		index = node.index;
    }

	~DendNode() 
	{
		delete left;
		delete right;
	}

	void addChild(const DendNode *child)
	{
		if (numChildren == 2)
		{
			cout << "full node";
			return;
		}
        if (left == NULL)
            left = new DendNode(child);
        else
            right = new DendNode(child);
		numChildren++;
	}

	int size()
	{
		return numChildren;
	}

	bool isLeaf()
	{
		if ( index == -1)
			return false;
		return true;
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
