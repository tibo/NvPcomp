/**********************************************************************/
//! ASTNode leaf_astNode implementation for NvPcomp
/*!
* \class leaf_astNode
*
* Description: The leaf_astNode implementation for Abstract Syntax Tree in NvPcomp
*
* \author CMT, DRJ & BFB
*
*/
/**********************************************************************/
#ifndef LEAF_ASTNODE_H_
#define LEAF_ASTNODE_H_

#include <ast.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <position.hh>
#include <location.hh>
#include <NvPcomp_logger.h>

class leaf_astNode:public astNode {
public:
	leaf_astNode();
	leaf_astNode(std::string _nodeString, NvPcomp::location _loc);
	~leaf_astNode();
public:
	virtual void output3AC();
};

#endif /* LEAF_ASTNODE_H_ */