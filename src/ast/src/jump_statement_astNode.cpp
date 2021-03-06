/***********************************************************************
* jump_statement_astNode - Syntax Tree Node
* Copyright (C) 2010 CMT & DRJ & BFB
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* \author CMT, DRJ & BFB
**********************************************************************/

#include <jump_statement_astNode.h>
#include <ast.h>

using namespace std;

jump_statement_astNode::jump_statement_astNode()
	:astNode() {
	nodeType = "jump_statement";
}

jump_statement_astNode::jump_statement_astNode(std::string _nodeString, NvPcomp::location _loc, NvPcomp::tacTree *tree)
	:astNode(_nodeString, _loc, tree) {
	nodeType = "jump_statement";
	LOG(ASTLog, logLEVEL1) << "===== Creating astNode ==== " << nodeType << " " << nodeString;
}

void jump_statement_astNode::output3AC() {
	if (nodeString == "RETURN_TK expression SEMICOLON_TK")
	{
	  std::string expression = "op1";
	  NvPcomp::tacNode * ac_node;
	  
	  // evaluate expression
	  getChild(1)->output3AC();
	  expression = getChild(1)->ret3ac;

	  
	  ac_node = new NvPcomp::tacNode("", OP_RETURN, "", "", expression, loc);
	  acTree->addNode(ac_node);	  
	}
	LOG(ASTLog, logLEVEL1) << nodeType << " is not supported at this time" << nodeString;
}

