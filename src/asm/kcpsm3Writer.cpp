/***********************************************************************
 *   kcpsm3Writer
 *   Copyright (C) 2010  CMT, DRJ & BFB
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 **********************************************************************/

#include <kcpsm3Writer.h>

using namespace std;

kcpsm3Writer::kcpsm3Writer(NvPcomp::tacTree* treeIn, \
							ast* asTree, \
							const char *outputFileName, \
							const char *inputFileName) {

	_asTree = asTree;
	_tacTree = treeIn;
	_variables = asTree->getVariableTable();
	_functions = asTree->getFunctionTable();
	_outputFileName = outputFileName;
	_inputFileName = inputFileName;
	_lastBufferLine = 0;
	_buffer.openFile(inputFileName);
	_tabLevel = 1;
	insertMult = false;
	insertDiv = false;
	insertedArray = false;
}

kcpsm3Writer::~kcpsm3Writer() {
	_buffer.closeFile();
}

void kcpsm3Writer::genASM(void) {
		
	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator iter;
	functionDefinition *funcDef;
	
	try {
	
		_out = fopen(_outputFileName, "w");
		// Output the Header.
		outputHeader();		
		// Start with the variable declarations.
		outputVariables();
		outputRegisters();
		// Address Start
		outputStart();
		// Begin initializations
		//outputInitializations();
		
		// We need to find the main function and start with it.
		outputMain();
		
		tree = _tacTree->getTree();
		// Get the other Functions
		for(iter = tree->begin(); iter < tree->end(); iter++) {
			if(iter->_op == OP_PROCENTRY && iter->_add1 != "main1") {
				if(_functions->search(iter->_add1, funcDef)) {
					
					_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
					
					_os << TABS(_tabLevel) << ";    Function: " << iter->_add1 << endl;
					_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
					
					_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(funcDef->loc.begin.line, funcDef->loc.end.line, COMMENTCHAR);
					_lastBufferLine = funcDef->loc.end.line;
					outputFunction(tree, iter);					
				} else {
					cout << "kcpsm3Writer: Did not find" << iter->_add1 << endl;
				}
			} 			
		}

		if(insertMult) {
			outputMultiply8bit();
		}
		
		if(insertDiv) {
			outputDivide8bit();
		}

		// Finish up with the Interrupt Servie Routine.
		outputISR();

	} catch (exception e) {
		LOG(ERRORLog, logLEVEL1) << "genASM error: " << e.what() << endl;
	}
	
	fprintf(_out, "%s", _os.str().c_str());
	fflush(_out);
	fclose(_out);
	
}

void kcpsm3Writer::outputHeader() {
	_os << TABS(_tabLevel) << ";" << endl;
	_os << TABS(_tabLevel) << ";    This file was automatically generated by NvPcomp" << endl;
	_os << TABS(_tabLevel) << ";" << endl;	
}

void kcpsm3Writer::outputStart() {
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Start at reset vector 000 " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;	
	_os << TABS(_tabLevel) << "ADDRESS     000" << endl;
}

void kcpsm3Writer::outputVariables() {

	map<string, variableInfo *>::iterator map_iter;
	map< std::string, variableInfo *> *table;
	int currentMemLoc = 0;
	int i;
	table = _variables->getTable();
	
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Begin Variable Declarations " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;

	for(map_iter = table->begin(); map_iter != table->end(); map_iter++) {
		if((*map_iter).second->isArray) {
			_os << TABS(_tabLevel) << "constant" << TABS(1) << (*map_iter).first << ",";
			_os <<  string(9-(*map_iter).first.size(), ' ') << TABS(1) << hex << setfill('0') << setw(2)<< currentMemLoc;
			_os << TABS(1) << TABS(1) << "; " << (*map_iter).second->size*(*map_iter).second->elements << " Bytes" << endl;
			
			(*map_iter).second->memLocation = currentMemLoc;
			currentMemLoc += (*map_iter).second->size*(*map_iter).second->elements;
			
		} else {
			_os << TABS(_tabLevel) << "constant" << TABS(1) << (*map_iter).first << ",";
			_os <<  string(9-(*map_iter).first.size(), ' ') << TABS(1) << hex << setfill('0') << setw(2)<< currentMemLoc;
			_os << TABS(1) << TABS(1) << "; " << (*map_iter).second->size << " Bytes" << endl;
			
			for(i=1; i<(*map_iter).second->size; i++) {
				_os << TABS(_tabLevel) << "constant" << TABS(1) << (*map_iter).first << i << ",";
				_os <<  string(8-(*map_iter).first.size(), ' ') << TABS(1) << hex << setfill('0') << setw(2)<< currentMemLoc+i << endl;
			}
			
			(*map_iter).second->memLocation = currentMemLoc;
			currentMemLoc += (*map_iter).second->size;
		}
	}
		
}

void kcpsm3Writer::outputMain() {
		
	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator iter;
	functionDefinition *funcDef;
		
	tree = _tacTree->getTree();
	
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Main " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	
	for(iter = tree->begin(); iter < tree->end(); iter++) {
		if(iter->_op == OP_PROCENTRY && iter->_add1 == "main1") {
			// Just make sure the declaration made it to the function table.
			if(_functions->search(iter->_add1, funcDef)) {
				_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(funcDef->loc.begin.line, funcDef->loc.end.line, COMMENTCHAR);
				_lastBufferLine = funcDef->loc.end.line;
				outputFunction(tree, iter);
				iter = tree->end();
			}
		}
	}
}

void kcpsm3Writer::outputISR() {
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Interrupt Service Routine (ISR) " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	
	_os << TABS(_tabLevel) << "ISR:" << endl;
	
	_tabLevel += 2;
	_os << TABS(_tabLevel) << "RETURNI ENABLE" << endl;
	_tabLevel -= 2;

	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Interrupt Vector " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;	
	_os << TABS(_tabLevel) << "ADDRESS     3FF" << endl;
	_os << TABS(_tabLevel) << "JUMP        ISR" << endl;
}

void kcpsm3Writer::outputFunction(vector<NvPcomp::tacNode> *tree, vector<NvPcomp::tacNode>::iterator &iter) {

	//The iterator should point to the entry point.
	_os << TABS(_tabLevel) << iter->_add1 << ":" << endl;
	++iter;
	
	_tabLevel += 2;
	
	int args = findNumArgs();
	cout << "args" << args << endl;
	while(OP_ENDPROC != iter->_op && iter < tree->end() ) {
		
		if(iter->_op == OP_ASSIGN){	
			
			if(args > 0) {
				if(_lastBufferLine < iter->_loc.end.line) {
					_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
					_lastBufferLine = iter->_loc.end.line;
				}
				_os << TABS(_tabLevel) << "LOAD     temp_reg,    func_lsb" << endl;
				_os << TABS(_tabLevel) << "STORE    temp_reg,    " << iter->_add3 << endl;
				_os << TABS(_tabLevel) << "STORE    temp_reg,    " << iter->_add1 << endl;
			} else {
				
				if(_lastBufferLine < iter->_loc.end.line) {
					_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
					_lastBufferLine = iter->_loc.end.line;
				}
				outputAssignment(iter);
			}			
		} else if(iter->_op >= OP_ADD && iter->_op <= OP_NEG) {
			if(_lastBufferLine < iter->_loc.end.line) {
				_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
				_lastBufferLine = iter->_loc.end.line;
			}
			outputArithmetic(iter);
		} else if(iter->_op == OP_ARRAY){	
			insertArray(iter);
		} else if(iter->_op < OP_EQ){	
			if(_lastBufferLine < iter->_loc.end.line) {
				_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
				_lastBufferLine = iter->_loc.end.line;
			}
			// Just print out the raw 3-address code for now.
			outputGeneric3AC(iter);
		} else if(iter->_op == OP_LABEL) {
			outputLabel(iter);
		} else if(iter->_op == OP_BR) {
			outputBranch(iter);		
		} else if(iter->_op == OP_RETURN) {
			outputReturn();
		} else if(iter->_op == OP_CALL)	{
			outputCall(iter);
		} else if(iter->_op == OP_ARGS)	{
			outputArgs(iter);	
		} else if(iter->_op >= OP_EQ && iter->_op <= OP_NE) {
			if(_lastBufferLine < iter->_loc.end.line) {
				_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
				_lastBufferLine = iter->_loc.end.line;
			}
			outputCompareAndBranch(iter);
		} else {
			outputGeneric3AC(iter);
		}

		++iter;
	}
	
	_tabLevel -= 2;
		
}

void kcpsm3Writer::outputReturn() {
	_os << TABS(3) << "RETURN" << endl;
}

int kcpsm3Writer::findNumArgs() {
	int retVal = 0;
	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator iter;
	functionDefinition *funcDef;
		
	tree = _tacTree->getTree();
	
	for(iter = tree->begin(); iter < tree->end(); iter++) {
		if(iter->_op == OP_ARGS) {
			retVal = atoi(iter->_add1.c_str());
			iter = tree->end();
		} 
	}		
	
	return retVal;
	
}

void kcpsm3Writer::outputArgs(vector<NvPcomp::tacNode>::iterator &iter) {

	int numArgs = atoi(iter->_add1.c_str());
	int i;

	for(i=0; i< numArgs; i++) {
		iter++;
		if(iter->_op == OP_REFOUT) {
			
		} else if(iter->_op == OP_VALOUT) {
			if(isValue(iter->_add1)) {
				_os << TABS(_tabLevel) << "LOAD     func_lsb,     " << iter->_add1 << endl;
			} else {
				_os << TABS(_tabLevel) << "FETCH    func_lsb,     " << iter->_add1 << endl;
			}
		}
		
	}
}

void kcpsm3Writer::insertArray(vector<NvPcomp::tacNode>::iterator &iter) {	
	//pair<map<string, NvPcomp::tacNode *>::iterator, bool> ret;
	//_arrayMap.insert(make_pair(iter->_add3, &(*iter)));	
	//cout << "Adding Array: " << iter->_add3 << endl;
	
	variableInfo *info;
	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator it;
	int offset = atoi(iter->_add2.c_str());
	int memLocation;
	bool flag = false;
			
	tree = _tacTree->getTree();
		
	// Get the memory location from the table
	if(_variables->search(iter->_add1, info)) {
		_os << TABS(_tabLevel) << "FETCH    array_lsb,     " << hex << setfill('0') << setw(2) << info->memLocation + offset << endl;	
		_os << TABS(_tabLevel) << "LOAD     array_addr,    " << hex << setfill('0') << setw(2) << info->memLocation + offset << endl;
		insertedArray = true;
	}
	
	//cout << "iter->_add3: " << iter->_add3 << endl;
	
	it = iter + 1;
	// loop through and replace	the register with array_lsb.
	while(!flag && it < tree->end() ) {
		if(it->_add1 == iter->_add3) {
			it->_add1 = "array_lsb";
			flag = true;
		} else if(it->_add2 == iter->_add3) {
			it->_add2 = "array_lsb";
			flag = true;
		} else if(it->_add3 == iter->_add3) {
			it->_add3 = "array_addr";
			flag = true;
		}		
		++it;
	}
	
}

void kcpsm3Writer::outputCall(vector<NvPcomp::tacNode>::iterator &iter) {
	
	_os << TABS(3) << "CALL     " << iter->_add1 << "1" << endl;
	
	if((iter+1)->_op == OP_ASSIGN) {
		if(insertedArray) {
			_os << TABS(_tabLevel) << "STORE    result_lsb,    (array_addr)" << endl;
			insertedArray = false;
		} else {
			_os << TABS(_tabLevel) << "STORE    result_lsb,    " << hex << setfill('0') << setw(2) << (iter+1)->_add3<< endl;
		}	 
		iter++;
		
	}
	
}

void kcpsm3Writer::outputLabel(vector<NvPcomp::tacNode>::iterator &iter) {
	_os << TABS(1) << iter->_add3 << ":" << endl;
}

//ToDo: Fix Long Values.  Floats come after.
void kcpsm3Writer::outputAssignment(vector<NvPcomp::tacNode>::iterator &iter) {

	variableInfo* info;
	
	if(_variables->search(iter->_add3, info)) {
		if(isRegister(iter->_add1) && isRegister(iter->_add3)) {
			if(info->tokenType == INT_TK || info->tokenType == SHORT_TK || info->tokenType == CHAR_TK) {
				_os << TABS(_tabLevel) << "STORE    result_lsb, (resAdr_lsb)" << endl;
			} else if(info->tokenType == LONG_TK) {
				_os << TABS(_tabLevel) << "STORE    result_msb, (resAdr_lsb)" << endl;
				_os << TABS(_tabLevel) << "STORE    result_lsb, (resAdr_msb)" << endl;
			}
		} else {		
			if(info->tokenType == INT_TK || info->tokenType == SHORT_TK || info->tokenType == CHAR_TK) {
				_os << TABS(_tabLevel) << "LOAD     temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add1 << endl;
				_os << TABS(_tabLevel) << "STORE    temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add3 << endl;
			} else if(info->tokenType == LONG_TK) {
				_os << TABS(_tabLevel) << "LOAD     temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add1.substr(0,2) << endl;
				_os << TABS(_tabLevel) << "STORE    temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add3 << endl;
				_os << TABS(_tabLevel) << "LOAD     temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add1.substr(2,2) << endl;
				_os << TABS(_tabLevel) << "STORE    temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add3 << "1" << endl;
			}
		}
	} else if(iter->_add3 == "array_addr") {
		if(isRegister(iter->_add1)) {			
			_os << TABS(_tabLevel) << "LOAD     temp_reg,    result_lsb" << endl;
		} else {
			_os << TABS(_tabLevel) << "LOAD     temp_reg,    " << hex << setfill('0') << setw(2) << iter->_add1 << endl;
		}
		_os << TABS(_tabLevel) << "STORE    temp_reg,    (array_addr)" << endl;
		
	} else {
		_os << TABS(_tabLevel) << "LOAD     " << hex << setfill('0') << setw(2) << iter->_add1 << ",    " << iter->_add3 << endl;
	} 	
}

void kcpsm3Writer::outputRegisters() {
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Reserved Registers " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;

	_os << TABS(_tabLevel) << "NAMEREG     s0,           ref_addr" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s1,           array_addr" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s2,           array_msb" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s3,           array_lsb" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s4,           resAdr_msb" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s5,           resAdr_lsb" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s6,           stack_ptr" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s7,           func_msb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     s8,           func_lsb" << endl;
	_os << TABS(_tabLevel) << "NAMEREG     s9,           result_msb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sA,           result_lsb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sB,           op1_msb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sC,           op1_lsb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sD,           op2_msb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sE,           op2_lsb" << endl;	
	_os << TABS(_tabLevel) << "NAMEREG     sF,           temp_reg" << endl;	
}

void kcpsm3Writer::outputBranch(vector<NvPcomp::tacNode>::iterator &iter) {
	_os << TABS(_tabLevel) << "JUMP     " << iter->_add3 << endl;	
}

void kcpsm3Writer::outputFetch(vector<NvPcomp::tacNode>::iterator &iter) {
	
	if(isRegister(iter->_add1)) {
		_os << TABS(_tabLevel) << "FETCH    temp_reg,    ";
		_os << "(" << iter->_add1 << ")" << endl;		
	} else {
		_os << TABS(_tabLevel) << "FETCH    temp_reg,    ";
		_os << iter->_add1 << endl;
	}	
}

void kcpsm3Writer::outputCompareAndBranch(vector<NvPcomp::tacNode>::iterator &iter) {
	
	int token = iter->_op;
	string label1 = _asTree->genLabel();
	string label2 = _asTree->genLabel();
	
	outputFetch(iter);
	
	int i = atoi(iter->_add2.c_str());
	
	_os << TABS(_tabLevel) << "COMPARE  temp_reg,      " << hex << setfill('0') << setw(2) << i << endl;
	++iter;
	
	// These are wrong for the demo.
	switch(token) {
		case OP_EQ:
			_os << TABS(_tabLevel) << "JUMP Z, " << iter->_add3 << endl;
			break;
		//case OP_GT:
		case OP_LT:
			_os << TABS(_tabLevel) << "JUMP NC, " << label1 << endl;
			_os << TABS(_tabLevel) << "JUMP     " << label2 << endl;
			_os << TABS(1) << label1 << ":" << endl;
			_os << TABS(_tabLevel) << "JUMP Z, " << iter->_add3 << endl;
			_os << TABS(1) << label2 << ":" << endl;
			break;
		//case OP_LT:
		case OP_GT:
			_os << TABS(_tabLevel) << "JUMP C, " << iter->_add3 << endl;
			break;
		//case OP_GE:
		case OP_LE:
			_os << TABS(_tabLevel) << "JUMP NC, " << iter->_add3 << endl;
			break;
		//case OP_LE:
		case OP_GE:
			_os << TABS(_tabLevel) << "JUMP C, " << iter->_add3 << endl;
			_os << TABS(_tabLevel) << "JUMP NZ, " << iter->_add3 << endl;
			break;
		case OP_NE:
			_os << TABS(_tabLevel) << "JUMP NZ, " << iter->_add3 << endl;
			break;
		default:
			break;
	}

}

void kcpsm3Writer::outputInitializations() {

	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator iter;
	functionDefinition *funcDef;
		
	tree = _tacTree->getTree();
	
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Initializations " << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	
	for(iter = tree->begin(); iter < tree->end(); iter++) {
		if(iter->_op == OP_PROCENTRY) {
			while(OP_ENDPROC != iter->_op && iter < tree->end() ) {
				iter++;
			}
		} else {						
			if(iter->_op == OP_ASSIGN){	
				if(_lastBufferLine < iter->_loc.end.line) {
					_os << TABS(_tabLevel) << _buffer.bufferGetLineCommented(iter->_loc.begin.line, iter->_loc.end.line, COMMENTCHAR);
					_lastBufferLine = iter->_loc.end.line;
				}
				outputAssignment(iter);
			}
		}
	}		
}

void kcpsm3Writer::outputArithmetic(vector<NvPcomp::tacNode>::iterator &iter) {
	
	variableInfo* info;	
	string lhs = iter->_add1;
	string rhs = iter->_add2;
	int token = -1;
	arithmeticType type;
	
	if(isValue(lhs) && isValue(rhs)) {
		// Both are values figure out what kind the first is.		
		token = getValueType(lhs);
		type = Type1;
	} else if(isValue(lhs) && !isValue(rhs)) {
		// RHS is an identifier
		if(_variables->search(rhs, info)) {
			token = info->tokenType;
		}		
		type = Type2;
	} else if(!isValue(lhs) && isValue(rhs)) {
		// LHS is an identifier
		if(_variables->search(lhs, info)) {
			token = info->tokenType;
		}
		type = Type3;
	} else {
		// Both LHS and RHS are identifiers
		if(_variables->search(lhs, info)) {
			token = info->tokenType;
		}
		type = Type4;
	}
		
	if(token == INT_TK || token == CHAR_TK || token == SHORT_TK) {
		integerArithmetic(iter, type);
	} else if(token == LONG_TK) {
		longArithmetic(iter, type);
	} else if(token == FLOAT_TK) {
		floatArithmetic(iter, type);
	}

}

void kcpsm3Writer::integerArithmetic(vector<NvPcomp::tacNode>::iterator &iter, arithmeticType type) {
	
	variableInfo* info;
	int token = iter->_op;
	string lhs = iter->_add1;
	string rhs = iter->_add2;
	
	int lhs_val = atoi(iter->_add1.c_str());
	int rhs_val = atoi(iter->_add2.c_str());
	
	string reg = iter->_add3;
	string opCode;
	
	switch(type) {
		case Type1:
			// Both are values.
			_os << TABS(_tabLevel) << "LOAD     op1_lsb,     " << hex << setfill('0') << setw(2) << lhs_val << endl;
			_os << TABS(_tabLevel) << "LOAD     op2_lsb,     " << hex << setfill('0') << setw(2) << rhs_val << endl;
			break;
		case Type2:
			// LHS is a value, RHS is an identifier
			_os << TABS(_tabLevel) << "LOAD     op1_lsb,     " << hex << setfill('0') << setw(2) << lhs_val << endl;
			_os << TABS(_tabLevel) << "FETCH    op2_lsb,     " << rhs << endl;

			break;
		case Type3:			
			_os << TABS(_tabLevel) << "FETCH    op1_lsb,     " << lhs << endl;
			_os << TABS(_tabLevel) << "LOAD     op2_lsb,     " << hex << setfill('0') << setw(2) << rhs_val << endl;
			break;
		case Type4:
			_os << TABS(_tabLevel) << "FETCH    op1_lsb,     " << lhs << endl;
			_os << TABS(_tabLevel) << "FETCH    op2_lsb,     " << rhs << endl;		
			break;
		default:
			break;
	}
	
	switch(token) {
		case OP_ADD:
			_os << TABS(_tabLevel) << "ADD      op1_lsb,     op2_lsb" << endl;
			_os << TABS(_tabLevel) << "LOAD     result_lsb,  op1_lsb" << endl;
			break;
		case OP_SUB:
			_os << TABS(_tabLevel) << "SUB      op1_lsb,     op2_lsb" << endl;
			_os << TABS(_tabLevel) << "LOAD     result_lsb,  op1_lsb" << endl;	
			break;
		case OP_MULT:
			_os << TABS(_tabLevel) << "CALL     mult_8x8" << endl;
			insertMult = true;
			break;
		case OP_DIV:
			_os << TABS(_tabLevel) << "CALL     div_8by8" << endl;
			insertDiv = true;
			break;
		case OP_NEG:
			break;
		default:
			break;
	}

	
	++iter;
	
	if(iter->_op == OP_RETURN) {
		outputReturn();
	} else {
		if(iter->_add3 == "array_addr" || isRegister(iter->_add3)) {
			_os << TABS(_tabLevel) << "STORE    result_lsb,    (" << iter->_add3 << ")" << endl;
		} else {
			_os << TABS(_tabLevel) << "STORE    result_lsb,    " << iter->_add3 << endl;
		}
	}
}

void kcpsm3Writer::longArithmetic(vector<NvPcomp::tacNode>::iterator &iter, arithmeticType type) {
	
	variableInfo* info;
	int token = iter->_op;
	string lhs = iter->_add1;
	string rhs = iter->_add2;
	string reg = iter->_add3;
	string opCode;
	
	switch(type) {
		case Type1:
			// Both are values.
			break;
		case Type2:
			// LHS is a value, RHS is an identifier
			break;
		case Type3:
			// LHS is an identifier, RHS is a value.
			break;
		case Type4:
			// Both are Identifiers.
			break;
		default:
			break;
	}
	
	switch(token) {
		case OP_ADD:
			opCode = "ADD";
			break;
		case OP_SUB:
			opCode = "SUB";
			break;
		case OP_MULT:
			break;
		case OP_DIV:
			break;
		case OP_NEG:
			break;
		default:
			break;
	}
		
}

void kcpsm3Writer::floatArithmetic(vector<NvPcomp::tacNode>::iterator &iter, arithmeticType type) {

	variableInfo* info;
	int token = iter->_op;
	string lhs = iter->_add1;
	string rhs = iter->_add2;
	string reg = iter->_add3;
	
	switch(type) {
		case Type1:
			// Both are values.
			break;
		case Type2:
			// LHS is a value, RHS is an identifier
			break;
		case Type3:
			// LHS is an identifier, RHS is a value.
			break;
		case Type4:
			// Both are Identifiers.
			break;
		default:
			break;
	}
	
	switch(token) {
		case OP_ADD:
			break;
		case OP_SUB:
			break;
		case OP_MULT:
			break;
		case OP_DIV:
			break;
		case OP_NEG:
			break;
		default:
			break;
	}
	
}

int kcpsm3Writer::getValueType(string str) {
	// Just assume it's an int for now
	return INT_TK;	
}

bool kcpsm3Writer::isRegister(std::string str) {
	bool retVal = false;
	
	//it's late and I'm grasping at straws here.
		
	retVal = 	str.substr(0,1) == "s" || \
				str == "resAdr_msb" || \
				str == "resAdr_lsb" || \
				str == "stack_ptr" || \
				str == "func_msb" || \
				str == "func_lsb" || \
				str == "result_msb" || \
				str == "result_lsb" || \
				str == "op1_msb" || \
				str == "op1_lsb" || \
				str == "op2_msb" || \
				str == "op2_lsb" || \
				str == "array_lsb" || \
				str == "array_msb" || \
				str == "array_addr" || \
				str == "temp_reg";				
	
	return retVal;
		
}

bool kcpsm3Writer::isValue(std::string str) {
	return isdigit(str[0]);
}

void kcpsm3Writer::outputMultiply8bit() {
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Built-in 8-bitx8-bit multiplier" << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	
	// multiplicand = op1_lsb
	// multiplier = op2_lsb
	// bit_mask = temp_reg
	
	
	_os << TABS(1) << "mult_8x8:" << endl;
	_os << TABS(3) << "LOAD     temp_reg,    01        ; (lsb)" << endl;
	_os << TABS(3) << "LOAD     result_msb,  00        ; clear msb" << endl;
	_os << TABS(3) << "LOAD     result_lsb,  00        ; clear lsb" << endl;
	_os << TABS(1) << "mult_loop:" << endl;
	_os << TABS(3) << "TEST     op2_lsb,     temp_reg  ; Check if bit is set" << endl;
	_os << TABS(3) << "JUMP     Z,           no_add    ; if not set, skip addition" << endl;
	_os << TABS(3) << "ADD      result_msb,  op1_lsb   ; addition only occurs in MSB" << endl;
	_os << TABS(1) << "no_add:" << endl;
	_os << TABS(3) << "SRA      result_msb             ; shift MSB right, CARRY into bit 7, lsb into CARRY" << endl;
	_os << TABS(3) << "SRA      result_lsb             ; shift LSB right, lsb from result_msb into bit 7" << endl;
	_os << TABS(3) << "SL0      temp_reg               ; shift bit_mask left to examine next bit in mult" << endl;
	_os << TABS(3) << "JUMP     NZ,          mult_loop ; bit mask will be zero when finished." << endl;
	_os << TABS(3) << "RETURN                          ; return." << endl;


}

void kcpsm3Writer::outputDivide8bit() {
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;
	_os << TABS(_tabLevel) << ";    Built-in 8-bit by 8-bit divider" << endl;
	_os << TABS(_tabLevel) << ";-----------------------------------------------------------" << endl;

	// dividend = op1_lsb 
	// divisor = op2_lsb
	// quotient = result_msb	
	// remainder = result_lsb
	// bit_mask = temp_reg
	
	_os << TABS(1) << "div_8by8:" << endl;
	_os << TABS(3) << "LOAD     result_lsb,  00        ; clear lsb" << endl;	
	_os << TABS(3) << "LOAD     temp_reg,    80        ; (lsb)" << endl;

	_os << TABS(1) << "div_loop:" << endl;
	_os << TABS(3) << "TEST     op1_lsb,     temp_reg  ; Check if bit is set" << endl;
	_os << TABS(3) << "SLA      result_lsb             ; shift CARRY into lsb of remainder" << endl;
	_os << TABS(3) << "SL0      result_msb             ; multiply by 2" << endl;
	_os << TABS(3) << "COMPARE  result_lsb,  op2_lsb   ; is the remainder > divisor" << endl;	
	_os << TABS(3) << "JUMP     C,           no_sub    ; if not set, skip subtraction" << endl;
	_os << TABS(3) << "SUB      result_lsb,  op2_lsb   ; " << endl;
	_os << TABS(3) << "ADD      result_msb,  01        ; add one to quotient" << endl;

	_os << TABS(1) << "no_sub:" << endl;
	_os << TABS(3) << "SR0      temp_reg               ; " << endl;
	_os << TABS(3) << "JUMP     NZ,          div_loop  ; bit mask will be zero when finished." << endl;
	// Just for the demo we're going to switch quotient with remainder
	_os << TABS(3) << "LOAD     temp_reg,    result_msb" << endl;
	_os << TABS(3) << "LOAD     result_msb,  result_lsb" << endl;
	_os << TABS(3) << "LOAD     result_lsb,  temp_reg" << endl;
	_os << TABS(3) << "RETURN                          ; return." << endl;
		
}

void kcpsm3Writer::analyzeFunctions() {
	vector<NvPcomp::tacNode> *tree;
	vector<NvPcomp::tacNode>::iterator iter;
	functionDefinition *funcDef;
	
	for(iter = tree->begin(); iter < tree->end(); iter++) {
		if(iter->_op == OP_PROCENTRY) {
			while(iter->_op != OP_ENDPROC) {
				
				
				if(iter->_op == OP_RETURN) {
					
				}
				++iter;
			}
		}
	}
}

void kcpsm3Writer::outputGeneric3AC(vector<NvPcomp::tacNode>::iterator &iter) {
	_os << "***" << TABS(_tabLevel) << _tacTree->getOp(iter->_op) << string(12 - _tacTree->getOp(iter->_op).size(), ' ');
	_os << iter->_add1 << string(12 - iter->_add1.size(), ' ');
	_os << iter->_add2 << string(12 - iter->_add2.size(), ' ');
	_os << iter->_add3 << string(12 - iter->_add3.size(), ' ');
	_os << endl;	
}




/*
typedef struct functionDefinition {
    astNode *declaration;
    astNode *definition;
	NvPcomp::location loc;
	std::vector<std::string> registers;
	std::vector<std::string> return_registers;
	int returnType;
} functionDefinition;
*/