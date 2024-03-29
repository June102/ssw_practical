//
// Created by vovan on 28.09.2019.
//

#include "Syntax.h"

/**
 * TODO: Update grammar for your variant of tasks
 * Given grammar:
 * <Soft>        ::= program <id> ; <block> .
 * <block>       ::= <var part> <state part>
 * <var part>    ::= var <var dec> : <type> [ = <exp> ] ;
 * <var dec>     ::= <id> { , <var dec> }
 * <state part>  ::= <compound>
 * <compound>    ::= begin <state> { ; <state> } end
 * <state>       ::= <assign> | <compound> | <your_other_operations>
 * <assign>      ::= <id> := <exp> ;
 * <exp>         ::= <id> | <constant> | <your_other_operations>
 * <type>        ::= integer
 * <id>          ::= a-z
 * <constant>    ::= 0-9
 */
Syntax::Syntax(std::vector<Lexem>&& t_lex_table) {
	if (t_lex_table.empty())
		//throw std::runtime_error("Syntax: Lexemes table is empty");
		PrintError(LEX_TABLE_EMPTY);
	if (t_lex_table.at(0).GetToken() == eof_tk)
		//throw std::runtime_error("<E> Syntax: Code file is empty");
		PrintError(FILE_EMPTY);
	lex_table = t_lex_table;
	cursor = lex_table.begin();
}


Syntax::~Syntax() {
	pascal_tree->freeTreeNode(pascal_tree);
}


/**
 * XXX: Each block (..Parse) builds its subtree (and returns it),
 *   which is then added at a higher level, at the place of the method call.
 */

 /**
  * @brief Start parse incoming pascal file
  * @param none
  *
  * @return  EXIT_SUCCESS - if file was successful parsed
  * @return -EXIT_FAILURE - if can't parse incoming file
  */
int Syntax::ParseCode() {
	std::cout << "Code contains " << lex_table.size() << " lexemes" << std::endl;
	auto& it = cursor;
	if (programParse(it) != 0)
		return -EXIT_FAILURE;
	while (it != lex_table.end() && it->GetToken() != eof_tk)
		blockParse(it);
	std::cout << "EOF" << std::endl;
	return EXIT_SUCCESS;
}


/**
 * @brief Parse entry point in grammar
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if input program part is correct
 * @return -EXIT_FAILURE - if input program part is incorrect
 * @note Wait input like: program <id_tk> ;
 */
int Syntax::programParse(lex_it& t_iter) {
	if (!checkLexem(t_iter, program_tk)) {
		printError(MUST_BE_PROG, *t_iter);
		return -EXIT_FAILURE;
	}

	auto iter = getNextLex(t_iter);
	if (!checkLexem(iter, id_tk)) {
		if (iter->GetToken() == eof_tk) {
			printError(EOF_ERR, *iter);
			return -EXIT_FAILURE;
		}
		else {
			printError(MUST_BE_ID, *iter);
			return -EXIT_FAILURE;
		}
	}
	auto root_name = iter->GetName(); // save the name of program

	iter = getNextLex(t_iter);
	if (!checkLexem(iter, semi_tk)) {
		if (iter->GetToken() == eof_tk) {
			printError(EOF_ERR, *iter);
			return -EXIT_FAILURE;
		}
		else {
			printError(MUST_BE_SEMI, *iter);
			return -EXIT_FAILURE;
		}
	}

	// First phase is OK, we can start to build the tree
	pascal_tree = pascal_tree->createNode(root_name); // TODO: rewrite, set unifications
										 //   for all *Parse methods

	return EXIT_SUCCESS;
}


/**
 * @brief Parse block part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if block part is matched to grammar
 * @return -EXIT_FAILURE - if block part doesn't matched to grammar
 */
int Syntax::blockParse(lex_it& t_iter) {
	try {
		auto iter = getNextLex(t_iter);
		switch (iter->GetToken()) {
		case var_tk: {
			pascal_tree->left_node = pascal_tree->buildTreeStub(pascal_tree, "var");
			vardpParse(t_iter, pascal_tree->left_node);
			break;
		}
		case begin_tk: {
			pascal_tree->right_node = pascal_tree->buildTreeStub(pascal_tree, "begin");
			compoundParse(t_iter, pascal_tree->right_node);
			break;
		}
		case dot_tk: {
			tree_t* t_tree = pascal_tree;
			while (t_tree->right_node != nullptr)
				t_tree = t_tree->right_node;
			t_tree->ChangeNodeValue("end!");
			std::cout << "Program was parse successfully" << std::endl;
			break;
		}
		default: {
			break;
		}
		}
		pascal_tree->PrintTree(pascal_tree);
		std::cout << std::endl;
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
		return -EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Parse variable declaration part
 * @param[inout] t_iter - iterator of table of lexeme
 * @param[inout] t_tree - subtree of part of variables
 *
 * @return  EXIT_SUCCESS - if variable declaration part is matched to grammar
 * @return -EXIT_FAILURE - if variable declaration part doesn't matched to grammar
 */
int Syntax::vardpParse(Syntax::lex_it& t_iter, tree_t* t_tree) {
	// var_list contains list of variables from current code line
	auto var_list = vardParse(t_iter);
	if (!checkLexem(t_iter, ddt_tk)) {
		printError(MUST_BE_COMMA, *t_iter);
	}

	auto type_iter = getNextLex(t_iter);
	if (!checkLexem(t_iter, type_tk)) {
		printError(MUST_BE_TYPE, *t_iter);
	}

	getNextLex(t_iter);
	if (!checkLexem(t_iter, semi_tk)) {
		printError(MUST_BE_SEMI, *t_iter);
	}

	updateVarTypes(var_list, type_iter->GetName());
	pascal_tree->buildVarTree(var_list, t_tree, id_map);//создаётся дерево идентификатор-тип и далее прикрепляется к основному
	auto forwrd_lex = peekLex(1, t_iter);
	if (checkLexem(forwrd_lex, var_tk) || checkLexem(forwrd_lex, id_tk)) {
		if (checkLexem(forwrd_lex, var_tk))
			getNextLex(t_iter);
		vardpParse(t_iter, t_tree->right_node);
	}
	else {
		pascal_tree->freeTreeNode(t_tree->right_node->right_node);//удаляем последний лишний $
	}
	return EXIT_SUCCESS;
}


/**
 * @brief Parse line of variables
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return list of variables
 * @note For example, we have next code lines:
 *   program exp1;
 *   var
 *     a, b : integer;    // return { a, b }
 *     c, d, e : integer; // return { c, d, e }
 *   ...
 */
std::list<std::string> Syntax::vardParse(lex_it& t_iter) {
	auto iter = getNextLex(t_iter);
	if (!checkLexem(iter, id_tk)) {
		printError(MUST_BE_ID, *iter);
		return std::list<std::string>();
	}

	if (isVarExist(iter->GetName())) printError(DUPL_ID_ERR, *iter);
	else
		id_map.emplace(iter->GetName(), Variable("?", "?"));

	std::list<std::string> var_list;
	var_list.push_back(t_iter->GetName());

	iter = getNextLex(t_iter);
	if (checkLexem(iter, comma_tk))
		var_list.splice(var_list.end(), vardParse(t_iter));

	return var_list;
}


/**
 * @brief Parse compound part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if compound part is matched to grammar
 * @return -EXIT_FAILURE - if compound part doesn't matched to grammar
 */
int Syntax::compoundParse(lex_it& t_iter, tree_t* t_tree) {
	static int compound_count = 0; // XXX: How can this be replaced?
	compound_count++;
	int state_count = 0;
	std::string op = "op";
	op = op + std::to_string(compound_count) + "." + std::to_string(state_count + 1);
	//op += std::to_string(state_count+1);
	t_tree->right_node = pascal_tree->buildTreeStub(t_tree, op);
	while (t_iter->GetToken() != end_tk) {
		if (t_iter->GetToken() == eof_tk) {
			printError(EOF_ERR, *t_iter);
			return -EXIT_FAILURE;
		}	
		state_count++;
		if (state_count > 1)
		{
			op.pop_back();
			op += std::to_string(state_count);
			t_tree->right_node->right_node = pascal_tree->buildTreeStub(t_tree, op);
			t_tree = (t_tree->right_node);
		}
		stateParse(t_iter, t_tree->right_node);
	}
	t_tree->right_node->ChangeNodeValue("end");
	if (compound_count == 1) { // XXX: How can this be replaced?
		if (checkLexem(peekLex(1, t_iter), unknown_tk) ||
			checkLexem(peekLex(1, t_iter), eof_tk)) {
			printError(MUST_BE_DOT, *t_iter);
			return -EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}


/**
 * @brief Parse state part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if state part is matched to grammar
 * @return -EXIT_FAILURE - if state part doesn't matched to grammar
 */
int Syntax::stateParse(lex_it& t_iter, tree_t* t_tree) {
	std::string expr="";
	auto iter = getNextLex(t_iter);
	switch (iter->GetToken()) {
	case id_tk: {
		if (!isVarExist(iter->GetName())) {
			printError(UNKNOWN_ID, *t_iter);
			return -EXIT_FAILURE;
		}
		getNextLex(t_iter);
		if (!checkLexem(t_iter, ass_tk)) {
			printError(MUST_BE_ASS, *t_iter);
			return -EXIT_FAILURE;
		}
		t_tree = t_tree->left_node = pascal_tree->buildTreeStub(t_tree, ":=") ;
		t_tree->left_node = pascal_tree->buildTreeStub(t_tree, iter->GetName());
		if (expressionParse(t_iter, expr) == 0)//постройка дерева арифм выражения
			if (!expr.empty())
			{
				pascal_tree->buildExpTree(t_tree, expr);
				expr = "";
			}
 		if (!checkLexem(t_iter, semi_tk)) { // we exit from expression on the ';'
			printError(MUST_BE_SEMI, *t_iter);
			return -EXIT_FAILURE;
		}
		break;
	}
	case begin_tk: {
		t_tree->left_node = pascal_tree->buildTreeStub(t_tree, "begin") ;
		compoundParse(t_iter, t_tree->left_node);
		getNextLex(t_iter);
		if (!checkLexem(t_iter, semi_tk)) {
			printError(MUST_BE_SEMI, *t_iter);
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
 * @brief Parse expression part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if expression part is matched to grammar
 * @return -EXIT_FAILURE - if expression part doesn't matched to grammar
 */
int Syntax::expressionParse(lex_it& t_iter, std::string &expr) {
	if (t_iter->GetToken() != semi_tk)
	{
		auto iter = getNextLex(t_iter);
		switch (iter->GetToken()) {
		case id_tk: {
			if (!isVarExist(iter->GetName()))
				printError(UNKNOWN_ID, *t_iter);
		}
		case constant_tk: { // like a := 3 ...
			expr += iter->GetName();
			iter = getNextLex(t_iter);
			switch (iter->GetToken()) {
			case add_tk:
			case sub_tk:
			case mul_tk: {
				// TODO: Here you have to check priority of operations
				// lambda check_priority(...) {
				//    if priority is higher {
				//        build_tree(...)
				//    } else {
				//        build_tree(...)
				//    }
				// }
				// expressionParse(...) // look deeper
				expr += iter->GetName();
				if (expressionParse(t_iter, expr) == -1)
					return -EXIT_FAILURE;
				break;
			}
			case div_tk: {
				expr += "/";
				if (expressionParse(t_iter, expr) == -1)
					return -EXIT_FAILURE;
				break;
			}
			default: { // any other lexem, expression is over
				break;
			}
			}
			break;
		}
		case sub_tk: { // like a := -3;
			if (!checkLexem((peekLex(1, t_iter)), constant_tk) && !checkLexem((peekLex(1, t_iter)), id_tk))
			{
				printError(MUST_BE_ID_OR_CONST, *t_iter);
				return -EXIT_FAILURE;
			}
			expr += "-";
			if (expressionParse(t_iter, expr) == -1)
				return -EXIT_FAILURE;
			break;
		}
		case opb_tk: { // like a := ( ... );
			expr += "(";
			if (expressionParse(t_iter, expr) == -1)
				return -EXIT_FAILURE;
			if (!checkLexem(t_iter, cpb_tk))
			{
				printError(MUST_BE_CPB, *t_iter);
				return -EXIT_FAILURE;
			}
			expr += ")";
			iter = getNextLex(t_iter);
			switch (iter->GetToken()) {
			case add_tk:
			case sub_tk:
			case mul_tk: {
				expr += iter->GetName();
				/*if (expressionParse(t_iter, expr) == -1)
					return -EXIT_FAILURE;*/
				break;
			}
			case div_tk: {
				expr += "/";
				/*if (expressionParse(t_iter, expr) == -1)
					return -EXIT_FAILURE;*/
				break;
			}
			default: {
				break;
			}
			}
			if (expressionParse(t_iter, expr) == -1)
				return -EXIT_FAILURE;
			break;
		}
		default: {
			printError(MUST_BE_ID, *t_iter);
			return -EXIT_FAILURE;
		}
		}
	}
	return EXIT_SUCCESS;
}


/**
 * @brief Create node of syntax tree
 * @param[in] t_tree    - parent node
 * @param[in] node_name - name of the created node (value)
 *
 * @return node of tree
 */
tree_t* tree_t::buildTreeStub(tree_t* t_tree, const std::string& node_name) {
	auto* tree = new tree_t;
	tree->value = node_name;
	tree->parent_node = std::addressof(*t_tree); // get address of parent node
	//tree->left_node = nullptr;
	//tree->right_node = nullptr;

	return tree;
}


/**
 * @brief Create like a root node of syntax tree
 * @param[in] node_name - name of the node (value)
 *
 * @return node of tree
 */
tree_t* tree_t::createNode(const std::string& node_name) {
	auto* tree = new tree_t;
	tree->value = node_name;
	//tree->left_node = nullptr;
	//tree->right_node = nullptr;
	//tree->parent_node = nullptr;

	return tree;
}


/**
 * @brief Print information about error
 * @param[in] t_err - error type
 * @param[in] lex   - error lexeme
 *
 * @return none
 */
void Syntax::printError(errors t_err, Lexem lex) {
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

void PrintError(errors t_err)
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
/**
 * @brief Get next lexeme
 * @param[inout] iter - cursor-iterator of lexeme table
 *
 * @return iterator on next lexeme
 */
Syntax::lex_it Syntax::getNextLex(lex_it& iter) {
	try {
		if (iter != lex_table.end())
			iter++;
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
	}

	return iter;
}


/**
 * @brief Peek to forward on the N lexeme
 * @param[in] N      - the number of skipped tokens
 * @param[in] t_iter - copy of cursor-iterator of lexeme table
 *
 * @return copy of iterator on N-th lexeme (token)
 * @note If catch exception, return copy of iterator
 */
Syntax::lex_it Syntax::peekLex(int N, lex_it t_iter) {
	try {
		auto iter = t_iter;
		while (iter != lex_table.end()) {
			if (N == 0) return iter;
			iter++; N--;
		}

		return iter;
	}
	catch (const std::exception & exp) {
		//std::cerr << "<E> Syntax: Can't peek so forward" << std::endl;
		PrintError(PEEK_ERROR);
		return t_iter;
	}
}


/**
 * @brief Check does current lexeme is match to needed token
 * @param[in] t_iter - current lexem
 * @param[in] t_tok  - needed token
 *
 * @return true  - if lexeme is match
 * @return false - if lexeme is unreachable (end) or if doesn't match
 */
bool Syntax::checkLexem(const Syntax::lex_it& t_iter, const tokens& t_tok) {
	if (t_iter == lex_table.end())   return false;
	if (t_iter->GetToken() != t_tok) return false;

	return true;
}


/**
 * @brief Check existence of variable
 * @param[in] t_var_name - variable for check
 *
 * @return true  - if variable is exist
 * @return false - if unknown variable (doesn't exist)
 */
bool Syntax::isVarExist(const std::string& t_var_name) {
	auto map_iter = id_map.find(t_var_name);
	return !(map_iter == id_map.end());
}


/**
 * @brief Update information about type in map of identifiers
 * @param[in] t_var_list  - list of variables
 * @param[in] t_type_name - type of variables
 *
 * @return none
 */
void Syntax::updateVarTypes(const std::list<std::string>& t_var_list,
	const std::string& t_type_name) {
	try {
		for (auto& el : t_var_list)
			id_map.at(el).type = t_type_name;
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
	}
}


/**
 * @brief Build subtree of variable declaration part
 * @param[in]  t_var_list - list of variable
 * @param[out] t_tree     - subtree of variable part
 *
 * @return none
 * @note If we firstly call this method:
 *                              program_name
 *                               /         \
 *             t_tree here ->  var         <block>
 */
void tree_t::buildVarTree(const std::list<std::string>& t_var_list, tree_t* t_tree, std::map<std::string, Variable> id_map) {
	try {
		auto i = 0;
		for (auto& el : t_var_list) {
			tree_t* tmp_tree = createNode(el);
			tmp_tree->right_node = buildTreeStub(tmp_tree, id_map.at(el).type);
			createVarTree(t_tree, tmp_tree, i++);
		}
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
	}
}


/**
 * @brief Insert subtree of <var dec> part to tree <var part>
 * @param[out] t_tree       - current node (look on var/$ root)
 * @param[in]  t_donor_tree - tree with information about identifier
 * @param[in]  lvl          - level of recursion
 *
 * @return none
 * @note How look t_tree:
 *                      program_name
 *                       /      \
 *                     var     <block>
 *                     / \
 *       <t_donor_tree>  <t_tree>
 *                       / \
 *         <t_donor_tree>  $
 *                        etc.
 * How look t_donor_tree:
 *                  a           <id>
 *                   \             \
 *                   integer       <type>
 */
void tree_t::createVarTree(tree_t* t_tree, tree_t* t_donor_tree, int lvl) {
	if (lvl > 0) {
		lvl--;
		createVarTree(t_tree->right_node, t_donor_tree, lvl);
	}
	else {
		t_tree->left_node = t_donor_tree;
		t_tree->right_node = buildTreeStub(t_tree, "$");
	}
}

void tree_t::buildExpTree(tree_t* t_tree, std::string expr)
{
	int expr_lengh = expr.length();
	if (expr_lengh == 2 && expr[0]=='-')
	{
		if (t_tree->left_node == NULL)
			t_tree->left_node = buildTreeStub(t_tree, expr);
		else
			t_tree->right_node = buildTreeStub(t_tree, expr);
	}
	else
	{
		if (expr_lengh == 1)
		{
			if (t_tree->left_node == NULL)
				t_tree->left_node = buildTreeStub(t_tree, expr);
			else
				t_tree->right_node = buildTreeStub(t_tree, expr);
		}
		else
		{
			std::string max = "";
			int op_count = 0;
			int max_i = 0;
			int RorL;
			for (int i = 0; i < expr_lengh; i++)
			
			{
				if (expr[i] > 47 && expr[i] < 58)
					continue;
				else
				{
					//std::cout << expr[i] << std::endl;
					if (expr[i] == '+' || expr[i] == '-')
					{
						if (expr[i] == '-' && (i == 0 || expr[i - 1] == '+' || expr[i - 1] == '*' || expr[i - 1] == '/'))
						{
							continue;
						}
						else
						{
							op_count++;
							max = expr[i];
							max_i = i;
							continue;
						}
					}
					if ((expr[i] == '*' || expr[i] == '/') && max != "+" && max != "-")
					{
						op_count++;
						max = expr[i];
						max_i = i;
						continue;
					}
					if (expr[i] == '(')
					{
						while (expr[i] != ')')
							i++;
					}
				}
			}
			if (t_tree->left_node == NULL)
			{
				t_tree->left_node = buildTreeStub(t_tree, max);
				RorL = 1;
			}
			else
			{
				t_tree->right_node = buildTreeStub(t_tree, max);
				RorL = 0;
			}
			max = "";
			if (expr.front() == '(' && expr[max_i - 1] == ')')
				for (int i = 1; i < max_i-1; i++)
					max += expr[i];
			else
				for (int i = 0; i < max_i; i++)
					max += expr[i];
			if (RorL == 1)
				buildExpTree(t_tree->left_node, max);
			else
				buildExpTree(t_tree->right_node, max);
			max = "";
			if (expr[max_i + 1] == '(' && expr.back() == ')')
				for (int i = max_i + 2; i < expr_lengh-1; i++)
					max += expr[i];
			else
				for (int i = max_i + 1; i < expr_lengh; i++)
					max += expr[i];
			if (RorL == 1)
				buildExpTree(t_tree->left_node, max);
			else
				buildExpTree(t_tree->right_node, max);
		}
	}
}
/**
 * @brief Free allocated (deallocate) memory
 * @param[in] t_tree - node of tree
 *
 * @return none
 */
void tree_t::freeTreeNode(tree_t*& t_tree) {
	try {
		if (t_tree->left_node != nullptr) freeTreeNode(t_tree->left_node);
		if (t_tree->right_node != nullptr) freeTreeNode(t_tree->right_node);
		delete t_tree;
		t_tree = nullptr;
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
	}
}
