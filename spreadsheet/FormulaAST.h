#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>
#include <set>

namespace ASTImpl {
class Expr;
}
  
class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};


class FormulaAST {
public:
  
    FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr, const std::set<Position>& cells = {});

    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(const SheetInterface& sheet) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::vector<Position> GetReferencedCells() const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::vector<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);
