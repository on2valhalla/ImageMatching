
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
