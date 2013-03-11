#ifndef DENDNODE_H
#define DENDNODE_H

#include <string>
#include <vector>
#include <iostream>

#include <opencv2/core/core.hpp>

class DendNode
{
	
public:
	double distance;
	DendNode *left;
	DendNode *right;
	std::vector<int> idxs;

	DendNode()
	{
		distance = -1;
		left = NULL;
		right = NULL;
	}
	DendNode(int idx)
	{
		distance = -1;
		idxs.push_back(idx);
		left = NULL;
		right = NULL;
	}
	DendNode(DendNode *l, DendNode *r, double matchD)
	{
		distance = matchD;
		left = l;
		right = r;
		idxs.insert(idxs.begin(), 
			l->idxs.begin(), l->idxs.end());
		idxs.insert(idxs.begin(), 
			r->idxs.begin(), r->idxs.end());
	}
	DendNode(const DendNode &node)
	{
		distance = node.distance;
		left = node.left;
		right = node.right;
		idxs = node.idxs;
	}

	~DendNode() 
	{
		if( left != NULL)
			delete left;
		if( right != NULL)
			delete right;
	}

	friend std::ostream& operator<< (std::ostream &o, const DendNode &n);


	// finds the maximum match between two nodes (possibly trees)
	// by checking the lookup Mat provided
	float maxMatch(const DendNode* node, const cv::Mat &lookup)
	{
		float max = 0;

		std::vector<int>::iterator it = idxs.begin();
		std::vector<int>::const_iterator jt;

		for (; it != idxs.end(); it++)
			for (jt = node->idxs.begin(); jt != node->idxs.end(); jt++)
				if (lookup.at<float>(*it, *jt) > max)
					max = lookup.at<float>(*it, *jt);

		return max;
	}
};

std::ostream& operator<< (std::ostream &o, const DendNode &n)
{
	o << "{ ";
	for (std::vector<int>::const_iterator it = n.idxs.begin(); it != n.idxs.end(); it++)
		o << *it << ", ";
	o << "}  d:" << n.distance;
	return o;
}



#endif // DENDNODE_H
