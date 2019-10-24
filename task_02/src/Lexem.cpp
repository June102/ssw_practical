//
// Created by vovan on 28.09.2019.
//

#include "Lexem.h"


/**
 * @brief Return name of lexeme
 * @param none
 *
 * @return name of lexeme
 */
std::string Lexem::GetName() {
    return name;
}


/**
 * @brief Return type of lexeme
 * @param none
 *
 * @return type (token) of lexeme
 */
tokens Lexem::GetToken() {
    return token;
}


/**
 * @brief Return line (from pascal file) of lexeme
 * @param none
 *
 * @return line of lexeme
 */
int Lexem::GetLine() {
    return line;
}

/**
 * @brief Print information about error
 * @param[in] t_err - error type
 * @param[in] lex   - error lexeme
 *
 * @return none
 */
void Error_Msg::printError(errors t_err, Lexem lex) {
	switch (t_err) {
	case UNKNOWN_LEXEM: {
		std::cerr << "<E> Lexer: Get unknown lexem '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case EOF_ERR: {
		std::cerr << "<E> Syntax: Premature end of file" << std::endl;
		break;
	}
	case MUST_BE_ID: {
		std::cerr << "<E> Syntax: Must be identifier instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_SEMI: {
		std::cerr << "<E> Syntax: Must be ';' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_PROG: {
		std::cerr << "<E> Syntax: Program must start from lexem 'program' ("
			<< lex.GetLine() << ")" << std::endl;
		break;
	}
	case MUST_BE_COMMA: {
		std::cerr << "<E> Syntax: Must be ',' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case DUPL_ID_ERR: {
		std::cerr << "<E> Syntax: Duplicate identifier '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case UNKNOWN_ID: {
		std::cerr << "<E> Syntax: Undefined variable '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_DOT: {
		std::cerr << "<E> Syntax: Program must be end by '.'" << std::endl;
		break;
	}
	case MUST_BE_TYPE: {
		std::cerr << "<E> Syntax: Must be type of identifier instead '" << lex.GetName() 
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_ASS: {
		std::cerr << "<E> Syntax: Must be ':=' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_ID_OR_CONST: {
		std::cerr << "<E> Syntax: Must be id or constant instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_CPB: {
		std::cerr << "<E> Syntax: Must be close round braket instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}

	
					// TODO: Add remaining error types+
	default: {
		std::cerr << "<E> Syntax: Undefined type of error" << std::endl;
		break;
	}
	}
}

void Error_Msg::PrintError(errors t_err)
{
	switch (t_err) {
	case CANT_OPEN_FILE: {
		std::cerr << "<E> Can't open file" << std::endl;
		break;
	}
	case UNKNOWN_TK: {
		std::cerr << "<E> Unknown token" << std::endl;
		break;
	}
	case FILE_FAIL: {
		std::cerr << "<E>  Can't read from the file | File doesn't available" << std::endl;
		break;
	}
	case LEX_TABLE_EMPTY: {
		std::cerr << "<E> Syntax: Lexemes table is empty" << std::endl;
		break;
	}
	case FILE_EMPTY: {
		std::cerr << "<E> Syntax: Code file is empty" << std::endl;
		break;
	}
	case PEEK_ERROR: {
		std::cerr << "<E> Syntax: Can't peek so forward" << std::endl;
		break;
	}
	default: {
		std::cerr << "<E> Syntax: Undefined type of error" << std::endl;
		break;
	}
	}
}
