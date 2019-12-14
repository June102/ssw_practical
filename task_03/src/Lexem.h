//
// Created by vovan on 28.09.2019.
//

#ifndef LECS_PARS_LEXEM_H
#define LECS_PARS_LEXEM_H

#include <string>


enum tokens {
	unknown_tk = -1,    // we get unknown token
	program_tk = 0,     // 'program'
	var_tk,             // 'var'
	begin_tk,           // 'begin'
	end_tk,             // 'end'
	type_tk,            // 'type'
	id_tk = 8,    // any [aA-zZ][0-9]
	constant_tk = 9,    // 0-9
	if_tk,				// 'if'
	then_tk,			// 'then'
	else_tk,			// 'else'
	for_tk,				// 'for'
	while_tk,			// 'while'
	do_tk,				// 'do'
	repeat_tk,			// 'repeat'
	until_tk,			// 'until'
	or_tk,				// 'or'
	and_tk,				// 'and'
	xor_tk,				// 'xor'
	nand_tk,			// 'nand'
	of_tk,				// 'of'
	to_tk,				// 'to'
	downto_tk,			// 'downto'
	array_tk,			// 'array'
	dot_tk,             // '.'
	comma_tk,           // ','
	ddt_tk,             // ':'
	semi_tk,            // ';'
	eqv_tk,             // '='
	ass_tk,             // ':='
	add_tk,             // '+'
	sub_tk,             // '-'
	mul_tk,             // '*'
	div_tk,             // '/'
	opb_tk,             // '('
	cpb_tk,             // ')'
	osb_tk,				// '['
	csb_tk,				// ']'
	neq_tk,				// '<>'
	ge_tk,				// '>'
	lw_tk,				// '<'
	geq_tk,				// '>='
	leq_tk,				// '<='

	eof_tk              // end of file
};


enum errors {
	UNKNOWN_LEXEM,   // Unknown lexeme
	EOF_ERR,         // End of file error
	MUST_BE_ID,      // Error: Must be identifier
	MUST_BE_SEMI,    // Error: Must be ';'
	MUST_BE_PROG,    // Error: Must be 'program'
	MUST_BE_COMMA,   // Error: Must be ','
	MUST_BE_TYPE,    // Error: Must be type of identifier
	MUST_BE_DOT,     // Error: Must be '.'
	MUST_BE_ASS,     // Error: Must be ':='
	DUPL_ID_ERR,     // Error: Duplicate declaration on identifier
	UNKNOWN_ID,      // Error: Undefined identifier

	MUST_BE_OSB,	 // Error: Must be '['
	MUST_BE_CONST,	 // Error: Must be constant
	MUST_BE_CSB,	 // Error: Must be ']'
	MUST_BE_OF,		 // Error: Must be 'of'
	MUST_BE_TYPE_OR_ARRAY, // Error: Must be type or 'array'
	NO_INDEX_OF_ARRAY,// Error: Variable must be array
	UNACC_EXP,		 // Error: Unacceptable expression
	MUST_BE_ID_OR_CONST,   // Error: Must be identifier or constant
	MUST_BE_STATE,	 // Error: Must be some statement
	MUST_BE_DO,		 // Error: Must be 'do'
	MUST_BE_TO,		 // Error: Must be 'to' or 'downto'
	MUST_BE_CPB,	 // Error: Must be ')'
	MUST_BE_UNTIL,	 // Error: Must be 'until'
	UNACC_TYPE,		 // Error: Unacceptble type of identifier
	MUST_BE_THEN,	 // Error: Must be 'then'
	VAR_ITER,		 // Error: Var is Iterator
};


class Lexem {
public:
    Lexem() : name(""), token(unknown_tk), line(0) {};
    Lexem(std::string &&t_name, tokens t_tok, int t_line) : name(t_name),
                                              token(t_tok), line(t_line) {};

    std::string GetName();
    tokens      GetToken();
    int         GetLine();
private:
    std::string name;
    tokens      token;
    int         line;
};


#endif //LECS_PARS_LEXEM_H
