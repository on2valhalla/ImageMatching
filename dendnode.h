#ifndef DENDNODE_H
#define DENDNODE_H

#include <string>

class DendNode
{

public:
	double distance;
	int index;

	Node()
	{
		distance = 0;
		numChildren = 0;
		index = -1;
		leftChild = NULL;
		rightChild = NULL;
	}
	Node(int idx)
	{
		distance = 0;
		numChildren = 0;
		index = idx;
		leftChild = NULL;
		rightChild = NULL;
	}
	Node(const Node &node)
	{
		distance = node.distance;
		numChildren = node.numChildren;
		*leftChild = node.leftChild;
		*rightChild = node.rightChild;
	}

	~Node() 
	{
		delete leftChild;
		delete rightChild;
	}

	void addChild(const Node &child)
	{
		if (numChildren == 2)
		{
			cout << "full node";
			return;
		}
		if (leftChild != NULL)
		{
			leftChild = new Node();
			leftChild = child;
		}
		else
		{
			rightChild = new Node();
			rightChild = child;
		}
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
		sstm << leftChild.toString() << "\t" << distance << "\t" << rightChild.toString();
		return sstm.str();
	}

private:

	int numChildren;
	Node *leftChild;
	Node *rightChild;

};

#endif // DENDNODE_H