//
// Created by vovan on 28.09.2019.
//

#ifndef LECS_PARS_SYNTAX_H
#define LECS_PARS_SYNTAX_H

#include <algorithm>
#include <iostream>
#include <chrono>
#include <list>
#include <map>
#include <vector>
#include "Lexem.h"
#include "Variable.h"
#include "Tree.h"


class Syntax {
public:
	explicit Syntax(std::vector<Lexem>&& t_lex_table);
	int ParseCode();
	~Syntax();
private:
	using lex_it = std::vector<Lexem>::iterator; // alias of vector iterator
	lex_it                          cursor;
	std::vector<Lexem>              lex_table;   // out table of lexemes
	std::map<std::string, Variable> id_map;      // our table of identifiers
	Tree* root_tree;
	int Errors = 0;
	int log_count = 0;

	std::map<std::string, int> operations;

	lex_it getNextLex(lex_it& iter);
	lex_it peekLex(int N, lex_it t_iter);


	int   expressionParse(lex_it& t_iter, std::vector<Lexem>& expr);
	Tree* stateParse(lex_it& t_iter);
	Tree* compoundParse(lex_it& t_iter);
	int   vardpParse(lex_it& t_iter, Tree* t_tree);
	std::list<std::string> vardParse(lex_it& t_iter);
	int   blockParse(lex_it& t_iter);
	int   programParse(lex_it& t_iter);

	void printError(errors t_err, Lexem lex);
	bool checkLexem(const lex_it& t_iter, const tokens& t_tok);
	bool checkExpr(lex_it& t_iter);
	bool isVarExist(const std::string& t_var_name);
	std::string getVarType(const std::string& t_var_name);
	bool CheckVarType(const std::string& t_var_name, int log_count);
	bool isVarArray(lex_it& t_iter, std::vector<Lexem> expr);
	bool isVarArray(lex_it& t_iter);
	bool isVarArray(std::string Var_Name);
	void updateVarTypes(const std::list<std::string>& t_var_list,
		const std::string& t_type_name, int t_array_l);
	void SetForIter(std::string var_name, int T_F);
	bool isVarIter(std::string var_name);
	void buildVarTree(const std::list<std::string>& t_var_list, Tree* t_tree, int Length);

	void createVarTree(Tree* t_tree, Tree* t_donor_tree, int lvl);
	Tree* createLowestOpTree(Tree* t_parent_tree, std::string value);
	Tree* BuildExpTree(Tree* t_tree, std::vector<Lexem> expr);
	int ArrayParse(Syntax::lex_it& t_iter, std::list<std::string> var_list);
};


#endif //LECS_PARS_SYNTAX_H
