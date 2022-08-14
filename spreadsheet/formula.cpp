#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
} 

std::ostream& operator<<(std::ostream& output, FormulaError::Category fe_category) {
    return output << FormulaError(fe_category);
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression);
    Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override; 
    std::vector<Position> GetReferencedCells() const override;

private:
    FormulaAST ast_;
};

Formula::Formula(std::string expression)
    : ast_(ParseFormulaAST(std::move(expression)))
{
    
}

FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const {
    try {
        return ast_.Execute(sheet);
    }
    catch (const FormulaError& fe) {
        return fe;
    }
    catch (const std::exception& ex) {
        return FormulaError(FormulaError::Category::Value);
    }
}
        
           
std::string Formula::GetExpression() const {
    std::ostringstream out;
    ast_.PrintFormula(out);
    return out.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    return ast_.GetReferencedCells();
}
    
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
