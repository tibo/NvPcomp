
%{
#include <stdio.h>
#include <limits.h>
#include <parse.hh>
#include <NvPcomp_logger.h>
#include <NvPcompScanner.h>
#include <ast.h>
#include <astNode.h>
#include <ast_include.h>
#include <defines.h>

#define YY_USER_ACTION yylloc->columns (yyleng);

#define YY_USER_INIT yylval->commentFound = false; \
					 typedefMode = false; \
					 reenterTypedefMode = false; \
					 openBraces = 0;

/* For logging */
#define RETURN(x) \
			LOG(SCANNERLog,logLEVEL1) << "Token: " << #x << " on line: " << yylineno; \
			prev_token = x; \
			str_prev_token = std::string(#x); \
			yylval->sval = strdup(yytext); \
			yylval->tval = (int)x; \
			yylval->astval = new leaf_astNode(strdup(yytext), *yylloc, acTree, (int)x); \
			return(x)

using namespace NvPcomp;

typedef NvPcomp::BParser::token token;

int 	CallerLevel;
bool 	typedefMode;
bool	reenterTypedefMode;		/* We were in typedefmode but had to come out of it due to an open brace */
int		openBraces;				/* Keep track of the open braces so we can reenter typedefMode if needed */

%}

%option nodefault yyclass="FlexScanner" noyywrap nounput c++ yylineno

/************************************************************************/
/* Regular Definitions													*/
/************************************************************************/
DELIM	[ \f]
WS		{DELIM}+
LETTER	[A-Za-z_]
DIGIT	[0-9]
NUMBER	(-)?{DIGIT}+
FLOAT	{DIGIT}+"."{DIGIT}*

%x comment
/************************************************************************/
/* Rules																*/
/* TODO:																*/
/*																		*/
/*%token	CHARACTER_CONSTANT_TK 	ENUMERATION_CONSTANT_TK */
/*%token 	TYPEDEF_NAME_TK   */
/************************************************************************/
%%

{WS}		{ yylloc->step(); }
[\n]+		{ yylloc->lines(yyleng);  yylloc->step();}
"\t"		{ yylloc->columns(7); yylloc->step(); }

"/*"					{	CallerLevel = YY_START;
							BEGIN(comment); 
						}
<comment>[^*\n]*        {}
<comment>"*"+[^*/\n]*   {}
<comment>\n             {yylloc->lines(yyleng);}
<comment>"*"+"/"        {				handleComment();
							BEGIN(CallerLevel); }
"!!$"		{table->dump();}

auto		{ RETURN(token::AUTO_TK); }
break		{ RETURN(token::BREAK_TK); }
case		{ RETURN(token::CASE_TK); }
char		{ RETURN(token::CHAR_TK); }
const		{ RETURN(token::CONST_TK); }
continue	{ RETURN(token::CONTINUE_TK); }
default		{ RETURN(token::DEFAULT_TK); }
do			{ RETURN(token::DO_TK); }
double		{ RETURN(token::DOUBLE_TK); }
else		{ RETURN(token::ELSE_TK); }
enum		{ RETURN(token::ENUM_TK); }
extern		{ RETURN(token::EXTERN_TK); }
float		{ RETURN(token::FLOAT_TK); }
for			{ RETURN(token::FOR_TK); }
goto		{ RETURN(token::GOTO_TK); }
if			{ RETURN(token::IF_TK); }
int			{ RETURN(token::INT_TK); }
long		{ RETURN(token::LONG_TK); }
register	{ RETURN(token::REGISTER_TK); }
return		{ RETURN(token::RETURN_TK); }
short		{ RETURN(token::SHORT_TK); }
signed		{ RETURN(token::SIGNED_TK); }
sizeof		{ RETURN(token::SIZEOF_TK); }
static		{ RETURN(token::STATIC_TK); }
struct		{ RETURN(token::STRUCT_TK); }
switch		{ RETURN(token::SWITCH_TK); }
typedef		{ RETURN(token::TYPEDEF_TK); }
union		{ RETURN(token::UNION_TK); }
unsigned	{ RETURN(token::UNSIGNED_TK); }
void		{ RETURN(token::VOID_TK); }
volatile	{ RETURN(token::VOLATILE_TK); }
while		{ RETURN(token::WHILE_TK); }

{LETTER}({LETTER}|{DIGIT})* { return(check_id()); }

{NUMBER}	{return check_integer();}
{FLOAT}		{return check_float();}

L?\"(\\.|[^\\"])*\"		{ RETURN(token::STRING_LITERAL_TK); }

"{"			{ newOpenBrace(); RETURN(token::OPEN_BRACE_TK); }
"}"			{ newClosedBrace(); RETURN(token::CLOSE_BRACE_TK); }
"("			{ RETURN(token::OPEN_PAREN_TK); }
")"			{ RETURN(token::CLOSE_PAREN_TK); }
"["			{ RETURN(token::OPEN_BRACK_TK); }
"]"			{ RETURN(token::CLOSE_BRACK_TK); }
";"			{ 	if(!typedefMode) {
					RETURN(token::SEMICOLON_TK); 
				} else {
					declarationError();
				}
			}
":"			{ RETURN(token::COLON_TK); }
","			{ RETURN(token::COMMA_TK); }
"."			{ RETURN(token::PERIOD_TK); }
"?"			{ RETURN(token::QUESTION_TK); }
"~"			{ RETURN(token::BIT_NOT_TK); }
"&"			{ RETURN(token::BIT_AND_TK); }
"|"			{ RETURN(token::BIT_OR_TK); }
"^"			{ RETURN(token::BIT_XOR_TK); }
"*"			{ RETURN(token::STAR_TK); }
"/"			{ RETURN(token::DIV_TK); }
"+"			{ RETURN(token::PLUS_TK); }
"-"			{ RETURN(token::MINUS_TK); }
"%"			{ RETURN(token::MOD_TK); }
"<"			{ RETURN(token::LT_TK); }
">"			{ RETURN(token::GT_TK); }
"->"		{ RETURN(token::PTR_OP_TK ); }
"++"		{ RETURN(token::INC_OP_TK); }
"--"		{ RETURN(token::DEC_OP_TK); }
"="			{ RETURN(token::EQUAL_TK); }
"<<"		{ RETURN(token::LEFT_OP_TK); }
">>"		{ RETURN(token::RIGHT_OP_TK); }
"<="		{ RETURN(token::LE_OP_TK); }
">="		{ RETURN(token::GE_OP_TK); }
"=="		{ RETURN(token::EQ_OP_TK); }
"!="		{ RETURN(token::NE_OP_TK); }
"!"			{ RETURN(token::NOT_TK); }
"&&"		{ RETURN(token::AND_OP_TK); }
"||"		{ RETURN(token::OR_OP_TK); }
"+="		{ RETURN(token::ADD_ASSIGN_TK); }
"-="		{ RETURN(token::SUB_ASSIGN_TK); }
"%="		{ RETURN(token::MOD_ASSIGN_TK); }
"*="		{ RETURN(token::MUL_ASSIGN_TK); }
"/="		{ RETURN(token::DIV_ASSIGN_TK); }
"<<="		{ RETURN(token::LEFT_ASSIGN_TK); }
">>="		{ RETURN(token::RIGHT_ASSIGN_TK); }
"&="		{ RETURN(token::AND_ASSIGN_TK); }
"^="		{ RETURN(token::XOR_ASSIGN_TK); }
"|="		{ RETURN(token::OR_ASSIGN_TK); }
"..."		{ RETURN(token::ELIPSIS_TK); }
.			{ id_error();}

%%

/************************************************************************/
/* User Code															*/
/************************************************************************/

NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::insertTypedef(std::string key) {

	symNode *tempNode = new symNode(*yylloc, key, "token::TYPEDEF_NAME_TK");
	
	/******************************************************************/
	/* Just here until the parser adds it. */
	tempNode->addType((int)token::TYPEDEF_NAME_TK);
	/******************************************************************/
	 
	InsertResult result = table->insert(key, tempNode);
	
	if(result == INSERT_SUCCESS_W_SHADOW) {
		LOG(INFOLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
		LOG(INFOLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-Variable redefined " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column;
		RETURN(token::ERROR_TK);		
	} else if(result == INSERT_FAIL_IN_CURRENT_LEVEL) {
		LOG(INFOLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
		LOG(INFOLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-failed to insert " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column;
		RETURN(token::ERROR_TK);
	} else {
		typedefMode = false;
		RETURN(token::TYPEDEF_NAME_TK);
	}
}

NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::insertID(std::string key) {
	/* Insert the id into the symbol table. */
	symNode *tempNode = new symNode(*yylloc, key, "");	
	/******************************************************************/
	/* Just here until the parser adds it. */
	//tempNode->addType((int)prev_token);
	/******************************************************************/	
	InsertResult result = table->insert(key, tempNode);
        std::cout << "inserting " << key << std::endl;

	if(result == INSERT_SUCCESS_W_SHADOW) {
		LOG(INFOLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
		LOG(INFOLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^Warning: Variable redefined " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column;
		RETURN(token::IDENTIFIER_TK);
	} else if(result == INSERT_FAIL_IN_CURRENT_LEVEL) {
		LOG(INFOLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
		LOG(INFOLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-failed to insert " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column;
		RETURN(token::ERROR_TK);
	} else {
		RETURN(token::IDENTIFIER_TK);
	}	
}

/* Very basic IDENTIFIER decision making */
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::check_id() {
	std::string key = std::string(yytext);
	symNode *tempNode;
	
	if(table->search(key, tempNode, false) != -1) {
		
		if(typedefMode) {
			/* The node is already in the table but should not be. */
			return(typedefError());			
		} else {
			if(tempNode->hasType((int)token::TYPEDEF_NAME_TK)) {
				RETURN(token::TYPEDEF_NAME_TK);
			} else {
				RETURN(token::IDENTIFIER_TK);
			}
		}	
	} else {
		if(typedefMode) {
			return(insertTypedef(key));
		} else {
			return(insertID(key));				
		}
	}	
}

/* Check integer bounds */
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::check_integer() {
	char *end_ptr;
	long long_var;
	
	//std::string temp = std::string(yytext);
	
	errno = 0;
	long_var = strtol(yytext, &end_ptr, 0);
	
	if(ERANGE == errno) {
		LOG(WARNINGLog, logLEVEL1) << "WARNING: Value: " << yytext << " is out of range.";
	} else if(long_var > ARCH_INT_MAX) {	/*Needs to be redefined for the target machine.*/
		LOG(WARNINGLog, logLEVEL1) << "WARNING: Value: " << yytext << " is too large for the target architecture.";
	} else if(long_var < ARCH_INT_MIN) {
		LOG(WARNINGLog, logLEVEL1) << "WARNING: Value: " << yytext << " is too small for the target architecture.";
	}

	yylval->ival = (int) long_var;
	
	LOG(SCANNERLog,logLEVEL1) << "\tConstant Integer: " << long_var << " on line: " << yylineno;
	
	RETURN(token::INTEGER_CONSTANT_TK);	
}

// Check Float Bounds.
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::check_float() {
	double doub_var; 
	errno = 0; 
	doub_var = strtod(yytext,NULL); 
			
	if(ERANGE == errno) {
		LOG(WARNINGLog, logLEVEL1) << "WARNING: Value: " << yytext <<" is out of range.";
	}
	
	yylval->dval = doub_var;	
	LOG(SCANNERLog,logLEVEL1) << "\tConstant float: " << doub_var << " on line: " << yylineno;
	
	RETURN(token::FLOATING_CONSTANT_TK);	

}

// Unrecognized Character Error
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::id_error() {
	LOG(ERRORLog, logLEVEL1) << buffer.bufferGetLine(yylineno, yylineno);
	LOG(ERRORLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-Unrecognized character: " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column << std::endl;
	RETURN(token::ERROR_TK);
}

// error defining new type.
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::typedefError() {
	LOG(ERRORLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
	LOG(ERRORLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-Redefined identifier";
	LOG(ERRORLog, logLEVEL1) << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column << std::endl;
	RETURN(token::ERROR_TK);
}

// error defining new type.
NvPcomp::BParser::token::yytokentype NvPcomp::FlexScanner::declarationError() {
	LOG(ERRORLog, logLEVEL1) << buffer.bufferGetLineNoCR(yylineno, yylineno);
	LOG(ERRORLog, logLEVEL1) << std::string(yylloc->begin.column - 1, ' ') << "^-Declaration does not declare anything!";
	LOG(ERRORLog, logLEVEL1) << "Found " << yytext << " on line: " << yylloc->begin.line << " at position: " << yylloc->begin.column << ", expecting identifier." << std::endl;
	RETURN(token::ERROR_TK);
}

// Deal with comments
void NvPcomp::FlexScanner::handleComment() {

	if(yylval->commentFound) {
		/* 	
		 * 	We already have a comment that hasn't been added.
		 * 	so add it to the end of the last comment's location
		 */
		yylval->comment_loc.end.line = yylloc->end.line;
		yylval->comment_loc.end.column = yylloc->end.column;
	} else {
		/* This is a new comment that needs to be added. */
		yylval->commentFound = true;
		yylval->comment_loc = *yylloc;
	}

}

void NvPcomp::FlexScanner::newOpenBrace() {
	if(typedefMode) {
		typedefMode = false;
		reenterTypedefMode = true;
		LOG(SCANNERLog,logLEVEL4) << "Scanner: entering reenterTypedefMode";
	}
	
	++openBraces;
	LOG(SCANNERLog,logLEVEL4) << "Scanner: new open brace current Count: " << openBraces;
	
}

void NvPcomp::FlexScanner::newClosedBrace() {
	--openBraces;
	if(openBraces < 1 && reenterTypedefMode) {
		typedefMode = true;
		reenterTypedefMode = false;
		LOG(SCANNERLog,logLEVEL4) << "Scanner: exciting reenterTypeDef and entering typedefMode: ";
	}
	
	LOG(SCANNERLog,logLEVEL4) << "Scanner: new closed brace current Count: " << openBraces;
	
}

