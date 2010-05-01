//! optokentypes.h
/*! 
 *
 * Description: Holds the token types for 3AC
 *
 * \author CMT, DRJ & BFB
 *
 */
#ifndef OPTOKENTYPES_H_
#define OPTOKENTYPES_H_

 enum optokentype {
  // Arithmetic Operations	 
  OP_ADD = 258, 		// op3 := op1 + op2
  OP_SUB = 259,			// op3 := op1 - op2
  OP_MULT = 260,		// op3 := op1 * op2
  OP_DIV  = 261,		// op3 := op1DIVop2
  OP_NEG = 262,			// op3 := -op1
  // Logical Operations
  OP_NOT = 263,			// if(op1 <> 0)op3 := 0 else op3 := 1
  OP_AND = 292,
  OP_OR = 293,
  // Relational Operations
  OP_EQ = 264,			// op3 := op1 = op2
  OP_GT = 265,			// op3 := op1 > op2
  OP_LT = 266,			// op3 := op1 < op2
  OP_GE = 267,			// op3 := op1 >= op2
  OP_LE = 268,			// op3 := op1 <= op2
  OP_NE = 269,			// op3 := op1 <> op2
  // Assignment
  OP_ASSIGN = 270,      // op3 := op1
  // Control Forms
  OP_LABEL = 271,		// The next statement is labeled with op1 
  OP_BR = 272,			// goto op3
  OP_BREQ = 273,		// if(op1 = op2) goto op3
  OP_BRGT = 274,		// if(op1 > op2) goto op3
  OP_BRLT = 275,		// if(op1 < op2) goto op3
  OP_BRGE = 276,		// if(op1 >= op2) goto op3
  OP_BRLE = 277,		// if(op1 <= op2) goto op3
  OP_BRNE = 278,		// if(op1 <> op2) goto op3
  OP_HALT = 279,		// Immediately halt execution
  // Procedure Call Operations
  OP_ARGS = 280,		// The next call requires op1 arguments
  OP_REFOUT = 281,		// Pass op1 by reference
  OP_VALOUT = 282,		// Pass op2 by value
  OP_CALL = 283,		// Call the procedure named op1
  OP_PROCENTRY = 284,	// Mark beginning of the procedure named op1
  OP_ENDPROC = 285,		// Mark the end of the current procedure
  OP_RETURN = 286,		// Return control to the caller
  // Additional Statements
  OP_BOUND = 287,		// if(op3 < op1)or(op3 > op2) then HALT
  OP_ADDR = 288,		// op3 := address of op1
  OP_GLOBAL = 289,		// Declare op1 as a global of size op2
  OP_STRING = 290,		// Associate string op1 with label op2
  OP_COMMENT = 291,		// op1 is a comment
};

#endif /* OPTOKENTYPES_H_ */
