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

class Syntax;

class tree_t {
private:
	tree_t* left_node;
	tree_t* right_node;
	tree_t* parent_node;
	std::string value;
public:
	tree_t();
	void PrintTree(tree_t* tree);
	tree_t* buildTreeStub(tree_t* t_tree, const std::string& node_name);
	tree_t* createNode(const std::string& node_name);
	void ChangeNodeValue(const std::string& node_name);
	void buildVarTree(const std::list<std::string>& t_var_list, tree_t* t_tree, std::map<std::string, Variable> id_map);

	void createVarTree(tree_t* t_tree, tree_t* t_donor_tree, int lvl);
	void freeTreeNode(tree_t*& t_tree);
	void buildExpTree(tree_t* t_tree, std::string expr);
	friend Syntax;
};
void tree_t::ChangeNodeValue(const std::string& node_name)
{
	value = node_name;
}
tree_t::tree_t()
{
	left_node = nullptr;
	right_node = nullptr;
	parent_node = nullptr;
}
void tree_t::PrintTree(tree_t* tree)
{
	if (tree == NULL)
	{
		std::cout << ".";
		return;
	}
	std::cout << tree->value;
	PrintTree(tree->left_node);
	PrintTree(tree->right_node);
}
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
	tree_t* tree_ptr;
	tree_t* pascal_tree; // our syntax tree
	lex_it getNextLex(lex_it& iter);
	lex_it peekLex(int N, lex_it t_iter);

	int   expressionParse(lex_it& t_iter, std::string &expr);
	int   stateParse(lex_it& t_iter, tree_t* t_tree);
	int   compoundParse(lex_it& t_iter, tree_t* t_tree);
	int   vardpParse(lex_it& t_iter, tree_t* t_tree);
	std::list<std::string> vardParse(lex_it& t_iter);
	int   blockParse(lex_it& t_iter);
	int   programParse(lex_it& t_iter);

	void printError(errors t_err, Lexem lex);
	bool checkLexem(const lex_it& t_iter, const tokens& t_tok);
	bool isVarExist(const std::string& t_var_name);
	void updateVarTypes(const std::list<std::string>& t_var_list,
		const std::string& t_type_name);
};


#endif //LECS_PARS_SYNTAX_H
