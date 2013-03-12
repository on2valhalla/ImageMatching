
//Have to put this here for the stupid MOC of QT
#include "dendnode.h"

std::ostream& operator<< (std::ostream &o, const DendNode &n)
{
	o << "{ ";
	for (std::vector<int>::const_iterator it = n.idxs.begin(); it != n.idxs.end(); it++)
		o << *it << ", ";
	o << "}  d:" << n.distance;
	return o;
}


void DendNode::toJson(std::string fileName)
{
	std::ofstream outputFile;
	std::stringstream links;
	outputFile.open(fileName.c_str(), std::ofstream::trunc | std::ofstream::out);

	outputFile << "{\"nodes\":[\n";
	links << "\"links\":[\n";

	int group = 0;
	float distTrack = 0;
	recursiveWriter(this, outputFile, links, &group, &distTrack);

	//join the nodes and the links

	//get rid of final commas
	long pos = outputFile.tellp();
	outputFile.seekp(pos-3);
	pos = links.tellp();
	links.seekp(pos-3);
	links << "\n";

	std::string linkStr = links.str();
	outputFile << "\n],\n" << linkStr << "]}";

	outputFile.close();
}


int DendNode::recursiveWriter(DendNode *node, std::ofstream &file, std::stringstream &links, int *group, float *distTrack)
{
	// leaf node terminating condition
	if (node->distance == -1)
	{
		int idx = node->idxs[0];
		std::string addr;
		if(idx + 1< 10)
			addr = ", \"img\":\"http://0.0.0.0:8000/img/i0";
		else
			addr = ", \"img\":\"http://0.0.0.0:8000/img/i";


		file << "\t{\"name\":" << "\"" << idx << "\", \"group\":" << *group
				<< addr << idx +1 << ".jpg\"}, \n";

		return idx;
	}

	if (fabs(*distTrack - node->distance) > .05)
	{
		*distTrack = node->distance;
		(*group)++;
	}

	int left = recursiveWriter(node->left, file, links, group, distTrack);
	int right = recursiveWriter(node->right, file, links, group, distTrack);

	links << "\t{\"source\":" << left << ", \"target\":" << right
				<< ", \"value\":" << (1-node->distance) * VALUE << "}, \n";

	return (left > right) ? left : right;
}









