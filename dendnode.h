/*

Created by: Jason Carlisle Mann (on2valhalla | jcm2207@columbia.edu)

A Node class for making dendrograms, as well as displaying with D3

*/
#ifndef DENDNODE_H
#define DENDNODE_H

#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <cmath>

#include <opencv2/core/core.hpp>

class DendNode
{
	
public:
	static const float VALUE = 1.0;
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


	// finds the minimum match between two nodes (possibly trees)
	// by checking the lookup Mat provided
	float minMatch(const DendNode* node, const cv::Mat &lookup)
	{
		float min = 2;

		std::vector<int>::iterator it = idxs.begin();
		std::vector<int>::const_iterator jt;

		for (; it != idxs.end(); it++)
			for (jt = node->idxs.begin(); jt != node->idxs.end(); jt++)
				if (lookup.at<float>(*it, *jt) < min)
					min = lookup.at<float>(*it, *jt);

		return min;
	}

	int getIdx()
	{
		if(idxs.size() > 1)
			return -1;
		return idxs[0];
	}

	void toJson(std::string fileName);
    int recursiveWriter(DendNode *node, std::ofstream &file, 
    			std::stringstream &links, int *group, float *distTrack);
	
};



#endif // DENDNODE_H
