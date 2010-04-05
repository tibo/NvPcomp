/**********************************************************************/
//! ASTNode declaration_astNode implementation for NvPcomp
/*!
* \class declaration_astNode
*
* Description: The declaration_astNode implementation for Abstract Syntax Tree in NvPcomp
*
* \author CMT, DRJ & BFB
*
*/
/**********************************************************************/
#ifndef DECLARATION_ASTNODE_H_
#define DECLARATION_ASTNODE_H_

#include <ast.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <position.hh>
#include <location.hh>
#include <NvPcomp_logger.h>

class declaration_astNode:public astNode {
public:
	declaration_astNode();
	declaration_astNode(std::string _nodeString, NvPcomp::location _loc);
	~declaration_astNode();
public:
	virtual void output3AC();
};

#endif /* DECLARATION_ASTNODE_H_ */