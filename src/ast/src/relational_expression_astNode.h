/**********************************************************************/
//! ASTNode relational_expression_astNode implementation for NvPcomp
/*!
* \class relational_expression_astNode
*
* Description: The relational_expression_astNode implementation for Abstract Syntax Tree in NvPcomp
*
* \author CMT, DRJ & BFB
*
*/
/**********************************************************************/
#ifndef RELATIONAL_EXPRESSION_ASTNODE_H_
#define RELATIONAL_EXPRESSION_ASTNODE_H_

#include <astNode.h>
#include <expression_astNode.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <position.hh>
#include <location.hh>
#include <tacTree.h>
#include <NvPcomp_logger.h>

class relational_expression_astNode:public expression_astNode {
public:
	relational_expression_astNode();
	relational_expression_astNode(std::string _nodeString, NvPcomp::location _loc, NvPcomp::tacTree *tree);
	~relational_expression_astNode();
public:
	virtual void output3AC();
};

#endif /* RELATIONAL_EXPRESSION_ASTNODE_H_ */
