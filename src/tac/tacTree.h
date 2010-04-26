/**********************************************************************/
//! tacTree - Three Address Code Tree
/*!
 * \class tacTree
 *
 * Description: tac Tree 
 *
 * \author CMT, DRJ & BFB
 *
 */
/**********************************************************************/
#ifndef TACTREE_H_
#define TACTREE_H_

#include <tacNode.h>
#include <vector>
#include <optokentypes.h>
#include <string>

namespace NvPcomp {
	class tacTree {

		public:
			tacTree();
			~tacTree();
		public:
			void addNode(tacNode *node);
			void outputASM();
			// For now, just displaying...will add logger later
			void displayTree();
		private:
			std::vector<tacNode> _tree;
			std::string getOp(int op);
			std::vector<std::string> translateOps;
	};
};

#endif /* TACNODE_H_ */
