/**********************************************************************/
//! ASTNode unary_expression_astNode implementation for NvPcomp
/*!
* \class unary_expression_astNode
*
* Description: The unary_expression_astNode implementation for Abstract Syntax Tree in NvPcomp
*
* \author CMT, DRJ & BFB
*
*/
/**********************************************************************/
#ifndef UNARY_EXPRESSION_ASTNODE_H_
#define UNARY_EXPRESSION_ASTNODE_H_

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

class unary_expression_astNode:public expression_astNode {
public:
	unary_expression_astNode();
	unary_expression_astNode(std::string _nodeString, NvPcomp::location _loc, NvPcomp::tacTree *tree);
	~unary_expression_astNode();
public:
	virtual void output3AC();
};

#endif /* UNARY_EXPRESSION_ASTNODE_H_ */
