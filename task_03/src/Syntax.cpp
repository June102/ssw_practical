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
 * <state>       ::= <assign> | <compound> | <while state> | <for state> | <if state>
 * <while state> ::= while <compare> do <state>
 * <for state>	 ::= for <id> := <constant> (to | downto) <constant> do <state> 
 * <if state>	 ::= if <compare> then <state> { ; else <state> }
 * <assign>      ::= <id> := <exp> ;
 * <exp>         ::= <id> | <constant> | <arith exp> | <compare> | <log exp>
 * <arith exp>   ::= <add> | <sub> | <mul> | <div>
 * <add>	 ::= <exp> + <exp>
 * <sub>	 ::= <exp> - <exp>
 * <div>	 ::= <exp> div <exp>
 * <mul>	 ::= <exp> * <exp>
 * <compare>	 ::= <eq> | <geq> | <seq> | <gr> | <sm> | <neq>
 * <eq>		 ::= <exp> == <exp>
 * <geq>	 ::= <exp> >= <exp>
 * <seq>	 ::= <exp> <= <exp>
 * <gr>		 ::= <exp> > <exp>
 * <sm>		 ::= <exp> < <exp>
 * <neq>	 ::= <exp> <> <exp>
 * <log exp>	 ::= <or> | <and> | <xor> | <nand>
 * <or>		 ::= <exp> or <exp>
 * <and>	 ::= <exp> and <exp>
 * <xor>	 ::= <exp> xor <exp>
 * <nand>	 ::= <exp> nand <exp>
 * <type>        ::= integer
 * <id>          ::= a-z
 * <constant>    ::= 0-9
 */
Syntax::Syntax(std::vector<Lexem>&& t_lex_table) {
	if (t_lex_table.empty())
		throw std::runtime_error("<E> Syntax: Lexemes table is empty");
	if (t_lex_table.at(0).GetToken() == eof_tk)
		throw std::runtime_error("<E> Syntax: Code file is empty");
	lex_table = t_lex_table;
	cursor = lex_table.begin();

	operations.emplace(":=", 0);

	operations.emplace("=", 1);
	operations.emplace("<>", 1);
	operations.emplace("<", 1);
	operations.emplace(">", 1);
	operations.emplace("<=", 1);
	operations.emplace(">=", 1);

	operations.emplace("+", 2);
	operations.emplace("-", 2);

	operations.emplace("or", 2);
	operations.emplace("xor", 2);

	operations.emplace("*", 3);
	//operations.emplace("/", 3);
	operations.emplace("div", 3);

	operations.emplace("and", 3);
	operations.emplace("nand", 3);
}


Syntax::~Syntax() {
	Tree::FreeTree(root_tree);
}


/**
 * Каждый блок (..Parse) строит своё поддерево (и возвращает его),
 *  которое затем добавляется на уровне выше, в месте вызова метода.
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

	std::cout << std::endl;
	std::cout << std::setfill('*') << std::setw(50);
	std::cout << "\r\n";

	//root_tree->PrintTree();
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
	root_tree = Tree::CreateNode(root_name); // TODO: rewrite, set unifications
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
		Tree* ParseTree=nullptr;
		switch (iter->GetToken()) {
		case var_tk: {
			root_tree->AddLeftNode("var");
			vardpParse(t_iter, root_tree->GetLeftNode());
			break;
		}
		case begin_tk: {
			// TO+DO: add check on nullptr from compoundParse
			ParseTree = compoundParse(t_iter);
			if ( ParseTree != nullptr)
			{
				root_tree->AddRightTree(ParseTree);
				break;
			}
		}
		case dot_tk: {
			// TO+DO: If we get error, parse couldn't be successful
			if (Errors==1)
				std::cout << "Program was't parse successfully" << std::endl;
			else
			{
				ParseTree = root_tree;
				while (ParseTree->GetRightNode() != nullptr)
				{
					ParseTree = ParseTree->GetRightNode();
				}
				ParseTree->ChangeValue(ParseTree->GetValue() + "!");
				std::cout << "Program was parse successfully" << std::endl;
				root_tree->PrintTree();
			}
			break;
		}
		default: {
			// XXX: May be like:
			// Here t_iter == eof_tk, if error_flag or error_count == 0
			//   parse was successful
			// else
			//   got error during parse
			break;
		}
		}
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
int Syntax::vardpParse(Syntax::lex_it& t_iter, Tree* t_tree) {
	// var_list contains list of variables from current code line
	auto var_list = vardParse(t_iter);
	if (!checkLexem(t_iter, ddt_tk)) {
		printError(MUST_BE_COMMA, *t_iter);
	}

	getNextLex(t_iter);
	if (checkLexem(t_iter, type_tk))
	{
		auto type_iter = t_iter;
		{
			getNextLex(t_iter);
			if (!checkLexem(t_iter, semi_tk)) {
				printError(MUST_BE_SEMI, *t_iter);
			}

			updateVarTypes(var_list, type_iter->GetName(), 0);
			buildVarTree(var_list, t_tree, -1);
		}
	}
	else
		if (checkLexem(t_iter, array_tk))
		{
			int Leng = ArrayParse(t_iter, var_list);
			if (Leng!=-1)
				buildVarTree(var_list, t_tree, Leng);
		}
		else
		{
			printError(MUST_BE_TYPE_OR_ARRAY, *t_iter);
			return -EXIT_FAILURE;
		}
	auto forwrd_lex = peekLex(1, t_iter);
	if (checkLexem(forwrd_lex, var_tk) || checkLexem(forwrd_lex, id_tk)) {
		if (checkLexem(forwrd_lex, var_tk))
			getNextLex(t_iter);
		vardpParse(t_iter, t_tree->GetRightNode());
	}
	else {
		t_tree->FreeRightNode();
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
		id_map.emplace(iter->GetName(), Variable("?", "?", 0));

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
 * @note Used generating of labels by Pogodin's idea
 */
Tree* Syntax::compoundParse(lex_it& t_iter) {
	static int compound_count = 0; // XXX: How can this be replaced?
	compound_count++;
	int local_lvl = compound_count; // save current compound level
	int sec_prm = 0;

	auto label = [&]() -> std::string {
		return "_*op" + std::to_string(local_lvl) + "." +
			std::to_string(sec_prm);
	};

	auto is_end = [&]() -> bool {
		return (checkLexem(peekLex(1, t_iter), end_tk)
			|| checkLexem(peekLex(1, t_iter), eof_tk));
	};

	Tree* tree = Tree::CreateNode(t_iter->GetName()); // 'begin'
	auto* root_compound_tree = tree; // save the pointer of start of subtree

	while (t_iter->GetToken() != end_tk) {
		if (t_iter->GetToken() == eof_tk) {
			printError(EOF_ERR, *t_iter);
			return nullptr;
		}

		auto* subTree = stateParse(t_iter);
		if (subTree != nullptr) {
			tree->AddRightNode(label());
			tree->GetRightNode()->AddLeftTree(subTree);
			tree = tree->GetRightNode();

			if (!is_end()) sec_prm++;
		}
	}

	if (compound_count == 1) { // XXX: How can this be replaced?
		if (checkLexem(peekLex(1, t_iter), unknown_tk) ||
			checkLexem(peekLex(1, t_iter), eof_tk) ||
			!checkLexem(peekLex(1, t_iter), dot_tk)) {
			printError(MUST_BE_DOT, *t_iter);
			return nullptr;
		}
	}
	else
		tree->AddRightNode(t_iter->GetName());

	return root_compound_tree;
}


/**
 * @brief Parse state part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if state part is matched to grammar
 * @return -EXIT_FAILURE - if state part doesn't matched to grammar
 */
Tree* Syntax::stateParse(lex_it& t_iter) {
	Tree* result_tree = nullptr;
	auto iter = getNextLex(t_iter);
	switch (iter->GetToken()) {
	case id_tk: {
		if (!isVarExist(iter->GetName())) {
			printError(UNKNOWN_ID, *t_iter);
			return nullptr;
		}
		if (isVarIter(iter->GetName()))
		{
			printError(VAR_ITER, *iter);
			return nullptr;
		}
		Tree* tree_exp;
		auto var_iter = iter;
		getNextLex(t_iter);
		std::vector<Lexem> expr;
		if (isVarArray(var_iter, expr))
		{
			for (int i=0; i<expr.size()+2;i++)
				getNextLex(t_iter);
			if (!checkLexem(t_iter, ass_tk)) {
				printError(MUST_BE_ASS, *t_iter);
				return nullptr;
			}
			tree_exp = Tree::CreateNode(t_iter->GetName());
			tree_exp->AddLeftNode("array");
			tree_exp->GetLeftNode()->AddLeftNode(var_iter->GetName());
			tree_exp->GetLeftNode()->AddRightTree(BuildExpTree(tree_exp->GetLeftNode()->GetRightNode(), expr));
		}
		else
		{
			if (!checkLexem(t_iter, ass_tk)) {
				printError(MUST_BE_ASS, *t_iter);
				return nullptr;
			}
			tree_exp = Tree::CreateNode(t_iter->GetName());
			tree_exp->AddLeftNode(var_iter->GetName());
		}
		expr.clear();
		log_count = 0;
		expressionParse(t_iter, expr);
		if (!checkLexem(t_iter, semi_tk)) { // we exit from expression on the ';'
			printError(MUST_BE_SEMI, *t_iter);
			return nullptr;
		}
		if (!CheckVarType(var_iter->GetName(), log_count))
		{
			printError(UNACC_TYPE, *var_iter);
			return nullptr;
		}
		tree_exp->AddRightTree(BuildExpTree(tree_exp->GetRightNode(), expr));
		result_tree = tree_exp;
		break;
	}
	case if_tk:	{
		auto* tree_exp = Tree::CreateNode(iter->GetName());
		std::vector<Lexem> expr;
		log_count = 0;
		if (expressionParse(t_iter, expr) == -1)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		if (log_count == 0)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		tree_exp->AddLeftTree(BuildExpTree(tree_exp->GetLeftNode(), expr));
		if (!checkLexem(t_iter, then_tk))
		{
			printError(MUST_BE_THEN, *t_iter);
			return nullptr;
		}
		auto* tree_state = stateParse(t_iter);
		if (tree_state == nullptr)
		{
			printError(MUST_BE_STATE, *t_iter);
			return nullptr;
		}
		if (checkLexem(peekLex(1, t_iter), else_tk))
		{
			tree_exp->AddRightNode("then");
			tree_exp->GetRightNode()->AddLeftTree(tree_state);
			getNextLex(t_iter);
			tree_state = stateParse(t_iter);
			if (tree_state == nullptr)
			{
				printError(MUST_BE_STATE, *t_iter);
				return nullptr;
			}
			tree_exp->GetRightNode()->AddRightTree(tree_state);
		}
		else
			tree_exp->AddRightTree(tree_state);
		result_tree = tree_exp;
		break;
	}
	case for_tk: {
		auto* tree_exp = Tree::CreateNode(iter->GetName());
		auto iter = getNextLex(t_iter);
		auto Var_Iter = iter;
		if (!checkLexem(iter, id_tk))
		{
			printError(MUST_BE_ID, *t_iter);
			return nullptr;
		}
		SetForIter(iter->GetName(), 1);
		if (!isVarExist(iter->GetName()))
		{
			printError(UNKNOWN_ID, *iter);
			return nullptr;
		}
		if (getVarType(iter->GetName()) != "integer")
		{
			printError(UNACC_TYPE, *iter);
			return nullptr;
		}
		if (!checkLexem(getNextLex(t_iter), ass_tk))
		{
			printError(MUST_BE_ASS, *t_iter);
			return nullptr;
		}
		auto* tree_state = Tree::CreateNode(t_iter->GetName());
		tree_state->AddLeftNode(iter->GetName());
		std::vector<Lexem> expr;
		log_count = 0;
		if (expressionParse(t_iter, expr) == -1)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		if (log_count != 0)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		tree_state->AddRightTree(BuildExpTree(tree_state->GetRightNode(), expr));
		switch (t_iter->GetToken()) {
		case to_tk: {
			expr.clear();
			log_count = 0;
			if (expressionParse(t_iter, expr) == -1)
			{
				printError(UNACC_EXP, *t_iter);
				return nullptr;
			}
			if (log_count != 0)
			{
				printError(UNACC_EXP, *t_iter);
				return nullptr;
			}
			tree_exp->AddLeftNode("<=");
			tree_exp->GetLeftNode()->AddLeftTree(tree_state);
			tree_exp->GetLeftNode()->AddRightTree(BuildExpTree(tree_exp->GetLeftNode()->GetRightNode(), expr));
			break;
		}
		case downto_tk: {
			expr.clear();
			log_count = 0;
			if (expressionParse(t_iter, expr) == -1)
			{
				printError(UNACC_EXP, *t_iter);
				return nullptr;
			}
			if (log_count != 0)
			{
				printError(UNACC_EXP, *t_iter);
				return nullptr;
			}

			tree_exp->AddLeftNode(">=");
			tree_exp->GetLeftNode()->AddLeftTree(tree_state);
			tree_exp->GetLeftNode()->AddRightTree(BuildExpTree(tree_exp->GetLeftNode()->GetRightNode(), expr));
			break;
		}
		default: {
			printError(MUST_BE_TO, *t_iter);
			return nullptr;
		}
		}
		if (!checkLexem(t_iter, do_tk))
		{
			printError(MUST_BE_DO, *t_iter);
			return nullptr;
		}
		tree_state = stateParse(t_iter);
		if (tree_state == nullptr)
		{
			printError(MUST_BE_STATE, *t_iter);
			return nullptr;
		}
		tree_exp->AddRightTree(tree_state);
		result_tree = tree_exp;
		SetForIter(Var_Iter->GetName(), 0);
	}
		break;
	case while_tk: {
		auto* tree_exp = Tree::CreateNode(iter->GetName());
		std::vector<Lexem> expr;
		log_count = 0;
		if (expressionParse(t_iter, expr) == -1)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		if (log_count == 0)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		tree_exp->AddLeftTree(BuildExpTree(tree_exp->GetLeftNode(), expr));
		if (!checkLexem(t_iter, do_tk))
		{
			printError(MUST_BE_DO, *t_iter);
			return nullptr;
		}
		auto* tree_state = stateParse(t_iter);
		if (tree_state == nullptr)
		{
			printError(MUST_BE_STATE, *t_iter);
			return nullptr;
		}
		tree_exp->AddRightTree(tree_state);
		result_tree = tree_exp;
		break;
	}
	case repeat_tk: {
		auto* rep_tree = Tree::CreateNode(iter->GetName());
		auto* tree_state = stateParse(t_iter);
		if (tree_state == nullptr)
		{
			printError(MUST_BE_STATE, *t_iter);
			return nullptr;
		}
		rep_tree->AddLeftTree(tree_state);
		iter = getNextLex(t_iter);
		if (!checkLexem(iter, until_tk))
		{
			printError(MUST_BE_UNTIL, *t_iter);
			return nullptr;
		}
		std::vector<Lexem> expr;
		log_count = 0;
		if (expressionParse(t_iter, expr) == -1)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		if (log_count == 0)
		{
			printError(UNACC_EXP, *t_iter);
			return nullptr;
		}
		rep_tree->AddRightTree(BuildExpTree(rep_tree->GetRightNode(), expr));
		result_tree = rep_tree;
		break;
	}
		break;
	case begin_tk: {
		auto* tree_comp = compoundParse(t_iter);
		getNextLex(t_iter);
		if (!checkLexem(t_iter, semi_tk)) {
			printError(MUST_BE_SEMI, *t_iter);
			return nullptr;
		}
		if (tree_comp != nullptr)
			result_tree = tree_comp;
		break;
	}
				 // TO+DO: Add if/while/for statements
	default: {
		break;
	}
	}

	return result_tree;
}


/**
 * @brief Parse expression part
 * @param[inout] t_iter - iterator of table of lexeme
 *
 * @return  EXIT_SUCCESS - if expression part is matched to grammar
 * @return -EXIT_FAILURE - if expression part doesn't matched to grammar
 */
int Syntax::expressionParse(lex_it& t_iter, std::vector<Lexem>& expr) {
	if (!checkLexem(peekLex(1, t_iter), semi_tk))
	{
		auto iter = getNextLex(t_iter);
		switch (iter->GetToken()) {
		case id_tk: {
			if (!isVarExist(iter->GetName()))
				printError(UNKNOWN_ID, *t_iter);
			if (!checkLexem(peekLex(1, iter), semi_tk))
				if (!checkExpr(iter))
				{
					printError(UNACC_EXP, *iter);
					return -EXIT_FAILURE;
				}
			if (isVarArray(t_iter))
			{
				while (!checkLexem(iter, csb_tk))
				{
					expr.emplace_back(*iter);
					iter = getNextLex(t_iter);
				}
			}
		}
		case constant_tk: { // like a := 3 ...
			expr.emplace_back(*iter);
			iter = getNextLex(t_iter);
			switch (iter->GetToken()) {
			case eqv_tk:
			case neq_tk:
			case geq_tk:
			case leq_tk:
			case lw_tk:
			case ge_tk:
				log_count++;
			case add_tk:
			case sub_tk:
			case mul_tk:
			case div_tk:
			case or_tk:
			case xor_tk:
			case and_tk: {
				expr.emplace_back(*iter);
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
			expr.emplace_back(*iter);
			if (expressionParse(t_iter, expr) == -1)
				return -EXIT_FAILURE;
			break;
		}
		case opb_tk: { // like a := ( ... );
			expr.emplace_back(*iter);
			if (expressionParse(t_iter, expr) == -1)
				return -EXIT_FAILURE;
			if (!checkLexem(t_iter, cpb_tk))
			{
				printError(MUST_BE_CPB, *t_iter);
				return -EXIT_FAILURE;
			}
			expr.emplace_back(*t_iter);
			iter = getNextLex(t_iter);
			switch (iter->GetToken()) {
			case eqv_tk:
			case neq_tk:
			case geq_tk:
			case leq_tk:
			case lw_tk:
			case ge_tk:
				log_count++;
			case add_tk:
			case sub_tk:
			case mul_tk:
			case div_tk:
			case or_tk:
			case xor_tk:
			case and_tk: {
				expr.emplace_back(*iter);
				if (expressionParse(t_iter, expr) == -1)
					return -EXIT_FAILURE;
				break;
			}
			default: {
				break;
			}
			}
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

int Syntax::ArrayParse(Syntax::lex_it& t_iter, std::list<std::string> var_list)
{
	int Leng;
	getNextLex(t_iter);
	if (!checkLexem(t_iter, osb_tk))
	{
		printError(MUST_BE_OSB, *t_iter);
		return -EXIT_FAILURE;
	}
	getNextLex(t_iter);
	if (checkLexem(t_iter, constant_tk))
	{
		Leng = atoi(t_iter->GetName().c_str());
	}
	else
	{
		if (checkLexem(t_iter, sub_tk))
		{
			if (!checkLexem(peekLex(1, t_iter), constant_tk))
			{
				printError(MUST_BE_CONST, *t_iter);
				return -EXIT_FAILURE;
			}
			getNextLex(t_iter);
			Leng = atoi(t_iter->GetName().c_str()) * -1;
		}
		else
		{
			printError(MUST_BE_CONST, *t_iter);
			return -EXIT_FAILURE;
		}
	}
	getNextLex(t_iter);
	if (!checkLexem(t_iter, dot_tk))
	{
		printError(MUST_BE_DOT, *t_iter);
		return -EXIT_FAILURE;
	}
	else
	{
		getNextLex(t_iter);
		if (!checkLexem(t_iter, dot_tk))
		{
			printError(MUST_BE_DOT, *t_iter);
			return -EXIT_FAILURE;
		}
	}
	getNextLex(t_iter);
	if (checkLexem(t_iter, constant_tk))
	{
		Leng = atoi(t_iter->GetName().c_str()) - Leng + 1;
	}
	else
	{
		if (checkLexem(t_iter, sub_tk))
		{
			if (!checkLexem(peekLex(1, t_iter), constant_tk))
			{
				printError(MUST_BE_CONST, *t_iter);
				return -EXIT_FAILURE;
			}
			getNextLex(t_iter);
			Leng = (atoi(t_iter->GetName().c_str()) * -1) - Leng + 1;
		}
		else
		{
			printError(MUST_BE_CONST, *t_iter);
			return -EXIT_FAILURE;
		}
	}
	getNextLex(t_iter);
	if (!checkLexem(t_iter, csb_tk))
	{
		printError(MUST_BE_CSB, *t_iter);
		return -EXIT_FAILURE;
	}
	getNextLex(t_iter);
	if (!checkLexem(t_iter, of_tk))
	{
		printError(MUST_BE_OF, *t_iter);
		return -EXIT_FAILURE;
	}
	getNextLex(t_iter);
	if (!checkLexem(t_iter, type_tk))
	{
		printError(MUST_BE_TYPE, *t_iter);
		return -EXIT_FAILURE;
	}
	auto type_iter = t_iter;
	getNextLex(t_iter);
	if (!checkLexem(t_iter, semi_tk))
	{
		printError(MUST_BE_SEMI, *t_iter);
	}
	updateVarTypes(var_list, type_iter->GetName(), Leng);
	return Leng;
}

Tree* Syntax::BuildExpTree(Tree* t_tree, std::vector<Lexem> expr)
{
	if (expr.at(0).GetToken() == sub_tk)
	{
		Lexem Zero("0", constant_tk, -1);
		expr.insert(expr.begin(), Zero);
	}
	int expr_size = expr.size();
	if (expr.at(0).GetToken() == opb_tk)
	{
		int n = 1;
		while (expr.at(n).GetToken() != cpb_tk)
			n++;
		if (n == expr_size - 1)
		{
			std::vector <Lexem> ExpPart;
			for (int i = 1; i < expr_size - 1; i++)
			{
				ExpPart.emplace_back(expr.at(i));
			}
			t_tree = BuildExpTree(t_tree, ExpPart);
			return t_tree;
		}
	}
	if (expr.at(0).GetToken() == id_tk)
	{
		if (isVarArray(expr.at(0).GetName()))
		{
			int n = 2;
			while (expr.at(n).GetToken() != csb_tk)
				n++;
			if (n == expr_size - 1)
			{
				std::vector <Lexem> ExpPart;
				t_tree = Tree::CreateNode("array");
				t_tree->AddLeftNode(expr.at(0).GetName());
				for (int i = 2; i < expr_size - 1; i++)
				{
					ExpPart.emplace_back(expr.at(i));
				}
				t_tree->AddRightTree(BuildExpTree(t_tree->GetRightNode(), ExpPart));
				return t_tree;
			}
		}
	}
	if (expr_size == 1)
	{
		t_tree = Tree::CreateNode(expr.at(0).GetName());
		return t_tree;
	}
	std::vector <Lexem> ExpPart;
	int min_pr = 5;
	int op_count = 0;
	int min_i = 0;
	int Curr_pr;
	for (int i = 0; i < expr_size; i++)
	{
		if (expr.at(i).GetToken() == constant_tk || expr.at(i).GetToken() == id_tk)
			continue;
		else
		{
			if (expr.at(i).GetToken() == sub_tk && (i == 0 || (expr.at(i - 1).GetToken() != constant_tk && expr.at(i - 1).GetToken() != id_tk)))
						continue;
			if (expr.at(i).GetToken() == opb_tk)
			{
				while (expr.at(i).GetToken() != cpb_tk)
					i++;
				i++;
			}
			if (expr.at(i).GetToken() == osb_tk)
			{
				while (expr.at(i).GetToken() != csb_tk)
					i++;
				i++;
			}
			if (i != expr_size)
			{
				Curr_pr = operations.at(expr.at(i).GetName());
				if (Curr_pr <= min_pr)
				{
					min_pr = Curr_pr;
					min_i = i;
				}
			}
		}
	}
	t_tree = Tree::CreateNode(expr.at(min_i).GetName());
	if (expr.front().GetToken() == opb_tk && expr.at(min_i - 1).GetToken() == cpb_tk)
		for (int i = 1; i < min_i - 1; i++)
			ExpPart.emplace_back(expr.at(i));
	else
		for (int i = 0; i < min_i; i++)
			ExpPart.emplace_back(expr.at(i));
	t_tree->AddLeftTree(BuildExpTree(t_tree->GetLeftNode(), ExpPart));
	ExpPart.clear();
	if (expr.at(min_i+1).GetToken() == opb_tk && expr.back().GetToken() == cpb_tk)
		for (int i = min_i+2; i < expr_size -1; i++)
			ExpPart.emplace_back(expr.at(i));
	else
		for (int i = min_i+1; i < expr_size; i++)
			ExpPart.emplace_back(expr.at(i));
	t_tree->AddRightTree(BuildExpTree(t_tree->GetRightNode(), ExpPart));
	return t_tree;
}

/**
 * @brief Print information about error
 * @param[in] t_err - error type
 * @param[in] lex   - error lexeme
 *
 * @return none
 */
void Syntax::printError(errors t_err, Lexem lex) {
	Errors = 1;
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
		std::cerr << "<E> Syntax: Must be '.'" << " on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_OSB: {
		std::cerr << "<E> Syntax: Must be '[' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_CONST: {
		std::cerr << "<E> Syntax: Must be constant instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_CSB: {
		std::cerr << "<E> Syntax: Must be ']' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_OF: {
		std::cerr << "<E> Syntax: Must be 'of' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_TYPE_OR_ARRAY: {
		std::cerr << "<E> Syntax: Must be type or 'array' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case NO_INDEX_OF_ARRAY: {
		std::cerr << "<E> Syntax: Variable '" << lex.GetName() << "' on " 
			<< lex.GetLine() << " line" << " must have index" << std::endl;
		break;
	}
	case UNACC_EXP: {
		std::cerr << "<E> Syntax: Unacceptable expression" << " on " 
			<< lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_STATE: {
		std::cerr << "<E> Syntax: Must be some statement" << " on "
			<< lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_DO: {
		std::cerr << "<E> Syntax: Must be 'do' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_TO: {
		std::cerr << "<E> Syntax: Must be 'to' or 'downto' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_CPB: {
		std::cerr << "<E> Syntax: Must be ')' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_UNTIL: {
		std::cerr << "<E> Syntax: Must be 'until' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case UNACC_TYPE: {
		std::cerr << "<E> Syntax: Unecceptable type of " << lex.GetName() 
			<< " identifier on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case MUST_BE_THEN: {
		std::cerr << "<E> Syntax: Must be 'then' instead '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line" << std::endl;
		break;
	}
	case VAR_ITER: {
		std::cerr << "<E> Syntax: Iterator '" << lex.GetName()
			<< "' on " << lex.GetLine() << " line, can't be changed inside the sycle" << std::endl;
		break;
	}
					// TO+DO: Add remaining error types 
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
		std::cerr << "<E> Syntax: Can't peek so forward" << std::endl;
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

std::string Syntax::getVarType(const std::string& t_var_name)
{
	auto map_iter = id_map.find(t_var_name);
	return map_iter->second.type;
}

bool Syntax::CheckVarType(const std::string& t_var_name, int log_count)
{
	if (log_count > 0)
	{
		if (getVarType(t_var_name) != "boolean")
			return false;
	}
	else
	{
		if (getVarType(t_var_name) == "boolean")
			return false;
	}
	return true;
}

bool Syntax::checkExpr(lex_it& t_iter)
{
	auto iter = t_iter;
	auto First_var = iter;
	auto Second_var = peekLex(2, iter);
	auto Operation = peekLex(1, iter);
	if (First_var->GetToken() == constant_tk && CheckVarType(Second_var->GetName(), 1))
	{
		return false;
	}
	if (Second_var->GetToken() == constant_tk && CheckVarType(First_var->GetName(), 1))
	{
		return false;
	}
	if (CheckVarType(First_var->GetName(), 1) && CheckVarType(Second_var->GetName(), 1))
		if (Operation->GetToken() == or_tk || Operation->GetToken() == xor_tk || Operation->GetToken() == and_tk)
		{
			log_count++;
			return true;
		}
		else
			return false;
	else
		if (!CheckVarType(First_var->GetName(), 0) || !CheckVarType(Second_var->GetName(), 0))
			return false;
	return true;
}

bool Syntax::isVarArray(lex_it& t_iter, std::vector<Lexem> expr)
{
	auto map_iter = id_map.find(t_iter->GetName());
	if (map_iter->second.array_l != 0)
	{
		auto exp_iter = peekLex(1, t_iter);
		if (checkLexem(peekLex(1, t_iter), osb_tk) && (expressionParse(exp_iter, expr)!= -1) && checkLexem(peekLex(expr.size()+2, t_iter), csb_tk))
		{
			return true;
		}
		else
		{
			printError(NO_INDEX_OF_ARRAY, *t_iter);
			return false;
		}

	}
	return false;
}

bool Syntax::isVarArray(std::string Var_Name)
{
	auto map_iter = id_map.find(Var_Name);
	if (map_iter->second.array_l == 0)
		return false;
	else
		return true;
}
bool Syntax::isVarArray(lex_it& t_iter)
{
	std::vector<Lexem> expr;
	auto map_iter = id_map.find(t_iter->GetName());
	if (map_iter->second.array_l != 0)
	{
		auto exp_iter = peekLex(1, t_iter);
		if (checkLexem(peekLex(1, t_iter), osb_tk) && (expressionParse(exp_iter, expr) != -1) && checkLexem(peekLex(expr.size() + 2, t_iter), csb_tk))
		{
			return true;
		}
		else
		{
			printError(NO_INDEX_OF_ARRAY, *t_iter);
			return false;
		}

	}
	return false;
}

/**
 * @brief Update information about type in map of identifiers
 * @param[in] t_var_list  - list of variables
 * @param[in] t_type_name - type of variables
 *
 * @return none
 */
void Syntax::updateVarTypes(const std::list<std::string>& t_var_list,
	const std::string& t_type_name, int t_array_l) {
	try {
		for (auto& el : t_var_list)
		{
			id_map.at(el).type = t_type_name;
			id_map.at(el).array_l = t_array_l;
		}
	}
	catch (const std::exception & exp) {
		std::cerr << "<E> Syntax: Catch exception in " << __func__ << ": "
			<< exp.what() << std::endl;
	}
}

void Syntax::SetForIter(std::string var_name, int T_F)
{
	id_map.find(var_name)->second.iterator = T_F;
}

bool Syntax::isVarIter(std::string var_name)
{
	auto map_iter = id_map.find(var_name);
	return (map_iter->second.iterator == 1);
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
void Syntax::buildVarTree(const std::list<std::string>& t_var_list, Tree* t_tree, int Length) {
	try {
		auto i = 0;
		for (auto& el : t_var_list) {
			auto* tmp_tree = Tree::CreateNode(el);
			tmp_tree->AddRightNode(id_map.at(el).type);
			if (Length != -1)
			{
				tmp_tree->AddLeftNode("array");
				tmp_tree->GetLeftNode()->AddRightNode(std::to_string(Length));
			}
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
 *
 * How look t_donor_tree:
 *                  a           <id>
 *                   \             \
 *                   integer       <type>
 */
void Syntax::createVarTree(Tree* t_tree, Tree* t_donor_tree, int lvl) {
	if (lvl > 0) {
		lvl--;
		createVarTree(t_tree->GetRightNode(), t_donor_tree, lvl);
	}
	else {
		t_tree->AddLeftTree(t_donor_tree);
		t_tree->AddRightNode("$");
	}
}
