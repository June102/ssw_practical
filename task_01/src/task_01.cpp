//
// Created by vldmr on 13.09.19.
//
#include "task_01.h"


/**
 * Given grammar:
 * <Soft>        ::= program <id> ; <block> .
 * <block>       ::= <var part> <state part>
 * <var part>    ::= var <var dec> : <type> ;
 * <var dec>     ::= <id> { , <var dec> }
 * <state part>  ::= <compound>
 * <compound>    ::= begin <state> { ; <state> } end
 * <state>       ::= <assign> | <compound> | <your_other_operations>
 * <assign>      ::= <id> := <exp> ;
 * <exp>         ::= <id> | <constant>
 * <type>        ::= integer
 * <id>          ::= a-z
 * <constant>    ::= 0-9
 */
enum tokens {
	unknown_tk = -1,    // we get unknown token
	program_tk = 0,     // 'program'
	var_tk,             // 'var'
	begin_tk,           // 'begin'
	end_tk,             // 'end'
	type_tk,            // 'type'
	id_tk = 8,    // any [aA-zZ][0-9]
	constant_tk = 9,    // 0-9
	dot_tk,             // '.'
	comma_tk,           // ','
	ddt_tk,             // ':'
	semi_tk,            // ';'
	eqv_tk,             // '='
	ass_tk,             // ':='
	add_tk,				// '+'
	sub_tk,				// '-'
	mul_tk,				// '*'
	div_tk				// 'div'
	/**
	 * TODO: Here is your +/-/etc tokens
	 */
};

//typedef std::pair<std::string, tokens> lexem;

struct lexem
{
	lexem LexIn(std::string lex, tokens tk, int N);
	std::string info;
	tokens token;
	int NLine;
}Lexem;
lexem lexem::LexIn(std::string lex, tokens tk, int N)
{
	info = lex;
	token = tk;
	NLine = N;
	return Lexem;
}

typedef struct synt {
	char  GetCurrentCurs();     // get current character in stream
	lexem GetLex();             // get next lexem

	char  PeekChar(int n);      // peek on N symbols in stream
	lexem PeekLex(int n);       // peek N lexem;
	bool  FileEOF();
	void  FileOpen(std::string file);
	bool  FileIsOpen();
	void  FileClose();
	bool  FileFail();
	int   GetCurrentLine();
private:
	int LineN=1;

	std::ifstream code;         // input stream of Pascal code

	char cursor{ -1 };         // cursor of stream

	char getChar();            // get next character in stream
} synt_t;

struct TListEl//список
{
	std::string Info;
	std::string Type;
	TListEl* Link;
};
TListEl* AddInf(std::string info, std::string Type, TListEl* pList)//добавление в список
{
	TListEl* pElem = new TListEl;
	pElem->Info = info;
	pElem->Type = Type;
	pElem->Link = pList;

	return pElem;
}
int SList(TListEl* pList, std::string ident)//поиск в списке
{
	TListEl* pElem = pList;
	while (pElem != NULL)
	{
		if (pElem->Info == ident)
			return 1;
		pElem = pElem->Link;
	}
	return 0;
}
struct variable {//структура для работы с переменными
	void VarSave(std::string ident, std::string Type);//сохранение переменной
	int VarSearch(std::string ident);//поиск переменной
	void DelList();
private:
	TListEl* VarAr=NULL;
} VarList;

void variable::VarSave(std::string ident, std::string Type)
{
	VarAr=AddInf(ident, Type, VarAr);
}
int variable::VarSearch(std::string ident)
{
	return SList(VarAr, ident);
}
void variable::DelList()
{
	delete VarAr;
}
void ErrMesage(lexem Lex, std::string Type)//вывод инф. об ошибках
{
		std::cout << "Error found in line " << Lex.NLine << " in lex: \"" << Lex.info << "\"\nType of error: " << Type << std::endl;
}
void ErrMesage(std::string Type)
{
	std::cout << "\nType of error: " << Type << std::endl;
}
void ErrMesage(int StrN, char ch, std::string Type)
{
	std::cout << "Error found in line " << StrN << " in lex: \"" << ch << "\"\nType of error: " << Type << std::endl;
}
struct LexTable
{
	void LexInTable(lexem lex);
	lexem GetLexT();
	lexem PeekLexT(int i);
	bool EmptyT();
private:
	std::vector <lexem> Lexems;
};
void LexTable::LexInTable(lexem lex)
{
	Lexems.insert(Lexems.begin(), lex);
}
lexem LexTable::GetLexT()
{
	lexem Lex = Lexems.back();
	Lexems.pop_back();
	return Lex;
}
lexem LexTable::PeekLexT(int i)
{
	lexem Lex = Lexems.at(Lexems.size() - i);
	return Lex;
}
bool LexTable::EmptyT()
{
	return Lexems.empty();
}

/**
 * @brief Get next character (symbol)
 * @param none
 *
 * @return current character
 * @note If stream is broken (eof or something), throw exception
 */
char synt_t::getChar() {
	if (code.fail()) {
		ErrMesage("Can't read from the file");
		//std::cerr << "<E> Can't read from the file" << std::endl;
		throw std::runtime_error("File doesn't available");
	}

	if (!code.eof()) {
		code >> std::noskipws >> cursor;
	}
	else {
		ErrMesage("File is EOF early");
		//std::cerr << "<E> File is EOF early" << std::endl;
		throw std::runtime_error("File is EOF early");
	}

	return cursor;
}


/**
 * @brief Peek to forward in stream on @n symbols
 * @param[in] n - the number of symbols for peek
 *
 * @return symbol from stream
 * @return -1 - can't peek forward
 * @note Peek only forward
 */
char synt_t::PeekChar(int n) {
	try {
		char ch = -1;
		int curr_pos = code.tellg(); // get current position in stream

		code.seekg(curr_pos + n, code.beg); // set needed position in stream
		code >> std::noskipws >> ch;    // get character from stream with ' '
		code.seekg(curr_pos, code.beg); // return previous position in stream

		return ch;
	}
	catch (std::exception & exp) {
		std::cerr << "<E> Catch exception in " << __func__ << ": " << exp.what()
			<< std::endl;
		return -1;
	}
}


/**
 * @brief Get current value of cursor
 * @param none
 *
 * @return value of cursor
 */
char synt_t::GetCurrentCurs() {
	return cursor;
}


/**
 * @brief Get next lexem
 * @param none
 *
 * @return lexem value (token)
 */
lexem synt_t::GetLex() {
	try {
		auto ch = GetCurrentCurs();
		while (ch == -1 || ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
			if (ch == '\n')
				LineN++;
			ch = getChar();
		}

		auto isId = [](char ch) {
			return std::isalpha(static_cast<unsigned char>(ch)) ||
				std::isdigit(static_cast<unsigned char>(ch));
		};


		std::string lex;
		if (std::isdigit(static_cast<unsigned char>(ch))) { // Constants (Numbers)
			while (std::isdigit(static_cast<unsigned char>(ch))) {
				lex += ch;
				ch = getChar();
			}

			return Lexem.LexIn(lex, constant_tk, LineN);// std::make_pair(lex, constant_tk);
		}
		else if (std::isalpha(static_cast<unsigned char>(ch))) { // Identificators
			while (isId(ch)) {
				lex += ch;
				ch = getChar();
			}

			if (lex == "program") { return Lexem.LexIn(lex, program_tk, LineN); } //std::make_pair(lex, program_tk); }
			else if (lex == "var") { return Lexem.LexIn(lex, var_tk, LineN); } //std::make_pair(lex, var_tk); }
			else if (lex == "begin") { return Lexem.LexIn(lex, begin_tk, LineN); } //std::make_pair(lex, begin_tk); }
			else if (lex == "integer") { return Lexem.LexIn(lex, type_tk, LineN); } //std::make_pair(lex, type_tk); }
			else if (lex == "end") { return Lexem.LexIn(lex, end_tk, LineN); } //std::make_pair(lex, end_tk); }
			else { // it is ID
				return Lexem.LexIn(lex, id_tk, LineN); //std::make_pair(lex, id_tk);
			}
		}
		else if (std::ispunct(static_cast<unsigned char>(ch))) { // Other symbols
			tokens tok;
			switch (ch) {
			case ',': tok = comma_tk; break;
			case '.': tok = dot_tk;   break;
			case ':': tok = ddt_tk;   break;
			case ';': tok = semi_tk;  break;
			case '=': tok = eqv_tk;   break;
			case '+': tok = add_tk;   break;
			case '-': tok = sub_tk;   break;
			case '*': tok = mul_tk;   break;
			case '/': tok = div_tk;   break;
			default: {
				ErrMesage(GetCurrentLine(), ch, "Unknown token");
				//std::cerr << "<E> Unknown token " << ch << std::endl; break;
			}
			}
			lex += ch;

			if (tok == ddt_tk) {
				ch = getChar();
				if (ch == '=') {
					lex += ch;
					tok = ass_tk;
				}
			}

			getChar(); // some kind of k o s t y l; here we look on \n
			return Lexem.LexIn(lex, tok, LineN); //std::make_pair(lex, tok);
		}
		else {
			ErrMesage(GetCurrentLine(), ch, "Unknown token");
			//std::cerr << "<E> Unknown token " << ch << std::endl;
		}

		return Lexem.LexIn("", unknown_tk, LineN); //std::make_pair("", unknown_tk);
	}
	catch (const std::exception & exp) {
		return Lexem.LexIn("", unknown_tk, LineN); //std::make_pair("", unknown_tk);
	}
}


/**
 * @brief Peek to forward on the N lexem
 * @param[in] n - the number of skipped tokens
 *
 * @return N-th lexem (token)
 * @note Return cursor of stream to previous position
 */
lexem synt_t::PeekLex(int n) {
	int curr_pos = code.tellg(); // get current position in stream
	auto curr_curs = GetCurrentCurs();
	try {
		lexem res;
		for (int i = 0; i < n; i++) {
			res = GetLex();
		}
		code.seekg(curr_pos, code.beg);
		cursor = curr_curs;

		return res;
	}
	catch (const std::exception & exp) {
		ErrMesage("You try to peek too much forward, get back");
		//std::cerr << "<E> You try to peek too much forward, get back" << std::endl;
		code.seekg(curr_pos, code.beg);
		cursor = curr_curs;

		return Lexem.LexIn("", unknown_tk, LineN); //std::make_pair("", unknown_tk);
	}
}

bool synt_t::FileEOF()
{
	return code.eof();
}
void  synt_t::FileOpen(std::string file)
{
	code.open(file);
}
bool  synt_t::FileIsOpen()
{
	return code.is_open();
}
void  synt_t::FileClose()
{
	code.close();
}
bool  synt_t::FileFail()
{
	return code.fail();
}
int   synt_t::GetCurrentLine()
{
	return LineN;
}

/********** prototypes of functions ***************/
void  BuildLexTable(synt_t& parser, LexTable& Table);
void  buildTreeStub(lexem lex);
int   expressionParse(lexem lex, LexTable& Table);
int   stateParse(lexem& lex, LexTable& Table);
int   compoundParse(lexem lex, LexTable& Table);
lexem vardParse(lexem lex, LexTable& Table);
int   blockParse(lexem lex, LexTable& Table);
int   programParse(synt_t& parser);
/*************************************************/
void  BuildLexTable(synt_t& parser, LexTable& Table)
{
	while (!parser.FileEOF() && !parser.FileFail())
		Table.LexInTable(parser.GetLex());
}
/**
 * @brief Stub for building tree
 * @param[in] lex - current lexem
 *
 * @return none
 * @note: in future here you will be building tree node
 */
void buildTreeStub(lexem lex) {
	/*std::cout << "<D> stub, get lexem " << lex.first << " (" << lex.second << ")"
			  << std::endl;*/
}


/**
 * @brief Parse expression part
 * @param[in]    lex    - current lexem
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return  EXIT_SUCCESS - if expression part is matched to grammar
 * @return -EXIT_FAILURE - if expression part doesn't matched to grammar
 */
int expressionParse(lexem lex, LexTable& Table) {
	lex = Table.GetLexT();
	switch (lex.token) {
	case id_tk:
	case constant_tk: {
		/**
		 * TODO: Here you have to check existence of operations (+/-/'*'/'/'/etc)+
		 */
		lex = Table.PeekLexT(1);
		
		if (lex.token == add_tk || lex.token == sub_tk || lex.token == mul_tk || lex.token == div_tk) {
			lex = Table.GetLexT();
			expressionParse(lex, Table);
		}
		 /**
		  * Also check priority of operations
		  */
		buildTreeStub(lex); // Here is your Tree realization
		break;
	}
	default: {
		ErrMesage(lex, "Must be identificator or constant");
		//std::cerr << "<E> Must be identificator or constant or '-' or '('"<< std::endl;
		return -EXIT_FAILURE;
	}
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Parse state part
 * @param[in]    lex    - current lexem
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return  EXIT_SUCCESS - if state part is matched to grammar
 * @return -EXIT_FAILURE - if state part doesn't matched to grammar
 */
int stateParse(lexem& lex, LexTable& Table) {
	lex = Table.GetLexT();
	switch (lex.token) {
	case id_tk: { // a := b (assign part)
		/**
		 * TODO: Here you have to check existence of variable+
		 */
		if (VarList.VarSearch(lex.info) != 1) {
			ErrMesage(lex, "undeclared identificator");
			//std::cerr << "<E> undeclared identificator" << std::endl;
			return -EXIT_FAILURE;
		}

		lex = Table.GetLexT();
		if (lex.token != ass_tk) {
			ErrMesage(lex, ":= is absent");
			//std::cerr << "<E> := is absent" << std::endl;
			return -EXIT_FAILURE;
		}

		expressionParse(lex, Table);

		lex = Table.GetLexT();
		if (lex.token != semi_tk) {
			ErrMesage(lex, "; is absent");
			//std::cerr << "<E> ; is absent" << std::endl;
			return -EXIT_FAILURE;
		}
		break;
	}
	case begin_tk: { // compound statements
		compoundParse(lex, Table);
		lex = Table.GetLexT();
		if (lex.token != semi_tk) {
			ErrMesage(lex, "';' is absent");
			//std::cerr << "<E> ';' is absent" << std::endl;
			return -EXIT_FAILURE;
		}
		break;
	}
	default: {

		break;
	}
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Parse compound part
 * @param[in]    lex    - current lexem
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return  EXIT_SUCCESS - if compound part is matched to grammar
 * @return -EXIT_FAILURE - if compound part doesn't matched to grammar
 */
int compoundParse(lexem lex, LexTable& Table) {
	static int compound_count = 0;
	compound_count++;
	while (lex.token != end_tk) {
		buildTreeStub(lex); // Here is your Tree realization
		if (Table.EmptyT()) {
			ErrMesage(lex, "Each begin must be closed by 'end'");
			//std::cerr << "<E> Each begin must be closed by 'end'" << std::endl;
			return -EXIT_FAILURE;
		}
		stateParse(lex, Table);
	}

	if (compound_count == 1) {
		if (Table.PeekLexT(1).token == unknown_tk) {
			ErrMesage(lex, "'.' is absent");
			//std::cerr << "<E> '.' is absent" << std::endl;
			return -EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Parse variable declaration part
 * @param[in]    lex    - current lexem
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return lexem - last parsed lexem (will actually return ':')
 */
lexem vardParse(lexem lex, LexTable& Table) {
	lex = Table.GetLexT();
	lexem VarType;
	int i=0;
	if (lex.token != id_tk) {
		ErrMesage(lex, "Here must be identificator");
		//std::cerr << "<E> Here must be identificator" << std::endl;
		return lex;
	}
	if (VarList.VarSearch(lex.info) != 1)
	{
		do
		{
			i++;
			VarType = Table.PeekLexT(i + 1);
			if (VarType.token == type_tk)
				VarList.VarSave(lex.info, VarType.info);
		} while (VarType.token != semi_tk && VarType.token != type_tk);
	}
	else
		ErrMesage(lex, "This_identificator_already_used");
		//std::cerr << "<E> This identificator already used" << std::endl;
	/**
	 *  TODO: Here save list of identificators+
	 */

	lex = Table.GetLexT();
	if (lex.token == comma_tk)
		return vardParse(lex, Table); // Раскручивая стек обратно,
									   // будет возвращено последнее значение

	return lex;
}


/**
 * @brief Parse block part
 * @param[in]    lex    - current lexem
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return  EXIT_SUCCESS - if block part is matched to grammar
 * @return -EXIT_FAILURE - if block part doesn't matched to grammar
 */
int blockParse(lexem lex, LexTable& Table) {
	lex = Table.GetLexT();
	switch (lex.token) { // var / begin
	case var_tk: {   // var a, b: integer;
		lex = vardParse(lex, Table);
		if (lex.token != ddt_tk)
			ErrMesage(lex, ": is absent");
			//std::cerr << "<E> : is absent" << std::endl;

		lex = Table.GetLexT();
		if (lex.token != type_tk)
			ErrMesage(lex, "Identificator must have type");
			//std::cerr << "<E> Identificator must have type" << std::endl;

		lex = Table.GetLexT();
		if (lex.token != semi_tk) {
			ErrMesage(lex, "; is absent");
			//std::cerr << "<E> ; is absent" << std::endl;
			return -EXIT_FAILURE;
		}

		buildTreeStub(lex); // Here is your Tree realization

		break;
	}
	case begin_tk: {
		compoundParse(lex, Table);
		break;
	}
	case dot_tk: {
		std::cout << "Program was parse successfully" << std::endl;
		VarList.DelList();
		return 1;
	}
	default: {
		ErrMesage(lex, "Unknown lexem");
		//std::cerr << "<E> Unknown lexem" << std::endl;
		return -EXIT_FAILURE;
	}
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Entry point in grammar
 * @param[inout] parser - our lexical and syntax analyzer
 *
 * @return  EXIT_SUCCESS - if input program part is correct
 * @return -EXIT_FAILURE - if input program part is incorrect
 * @note Wait input like: program <id_tk> ;
 */
int programParse(LexTable& Table) {
	auto lex = Table.GetLexT();
	if (lex.token == program_tk) {
		lex = Table.GetLexT();
		if (lex.token != id_tk) {
			ErrMesage(lex, "Name of program is absent");
			//std::cerr << "<E> Name of program is absent" << std::endl;
			if (lex.token != semi_tk) {
				ErrMesage(lex, "; is absent");
				//std::cerr << "<E> ; is absent" << std::endl;
				return -EXIT_FAILURE;
			}
		}

		lex = Table.GetLexT();
		if (lex.token != semi_tk) {
			ErrMesage(lex, "; is absent");
			//std::cerr << "<E> ; is absent" << std::endl;
			return -EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	else {
		ErrMesage(lex, "Undefined type of program");
		//std::cerr << "<E> Undefined type of program" << std::endl;
		return -EXIT_FAILURE;
	}
}


/**
 * @brief Start parse incoming pascal file
 * @param[in] file_path - path to a pascal file (code.p)
 *
 * @return  EXIT_SUCCESS - if file was successful parsed
 * @return -EXIT_FAILURE - if can't parse incoming file
 */
int Parse2(const std::string& file_path) {
	try {
		synt_t example_synt;
		LexTable TableOfLex;
		example_synt.FileOpen(file_path);
		if (!example_synt.FileIsOpen()) {
			ErrMesage("Can't open file");
			//std::cerr << "<E> Can't open file" << std::endl;
			return -EXIT_FAILURE;
		}
		BuildLexTable(example_synt, TableOfLex);
		if (programParse(TableOfLex) != 0) {
			example_synt.FileClose();
			VarList.DelList();
			return -EXIT_FAILURE;
		}

		lexem lex;
		while (!TableOfLex.EmptyT()) {
			if (blockParse(lex, TableOfLex) == 1)
				break;
		}

		example_synt.FileClose();
		return EXIT_SUCCESS;
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Catch exception in " << __func__ << ": " << exp.what()
			<< std::endl;
		return -EXIT_FAILURE;
	}
}


/**
 * What you have to do:
 *  - rewrite synt structure (set more encapsulated);
 *  - add struct for work with variables;
 *  - add variable check (check of existen);
 *  - add + and - operations (with variadic number of values);
 *  - add function of error information (must pass the number line of code
 *      with error, type of error);
 *  - rewrite work with table of lexemes (get lexemes directly from table);
 *  - implement all TODO tips (start with 'TODO: Here you have to ...')
 */
