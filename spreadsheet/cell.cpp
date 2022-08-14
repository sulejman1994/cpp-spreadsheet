#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using std::make_unique;

DependencyGraph::DependencyGraph(SheetInterface& sheet) 
    : sheet_(sheet)
{

}

bool DependencyGraph::TryChangeCell(Position pos, const std::vector<Position>& new_referenced_cells) {

    std::unordered_set<Position, Position::Hasher> old_referenced_cells = cell_to_referenced_cells_[pos];
    cell_to_referenced_cells_[pos] = { new_referenced_cells.begin(), new_referenced_cells.end() };

    if (!IsCycle(pos)) {
        RecalculateDepentEdges(pos, old_referenced_cells, new_referenced_cells);
        InvalidateCash(pos);
        return true;
    }

    cell_to_referenced_cells_[pos] = old_referenced_cells;
    return false;
}

bool DependencyGraph::IsCycle(Position pos) const {
    bool is_cycle = false;
    std::unordered_map<Position, int, Position::Hasher> colors;
    DfsForCycle(pos, colors, is_cycle);
    return is_cycle;
}

void DependencyGraph::DfsForCycle(Position pos, std::unordered_map<Position, int, Position::Hasher>& colors, bool& is_cycle) const {
    if (!cell_to_referenced_cells_.count(pos)) {
        colors[pos] = 2;
        return;
    }
    colors[pos] = 1;
    for (const auto& cell : cell_to_referenced_cells_.at(pos)) {
        if (!colors.count(cell)) {
            DfsForCycle(cell, colors, is_cycle);
        }
        else if (colors.at(cell) == 1) {
            is_cycle = true;
            return;
        }
        if (is_cycle) {
            return;
        }
    }
    colors[pos] = 2;
}

void DependencyGraph::RecalculateDepentEdges(Position pos, const std::unordered_set<Position, Position::Hasher>& old_referenced_cells,
    const std::vector<Position>& new_referenced_cells) {

    for (const auto& cell : old_referenced_cells) {
        cell_to_depent_cells_[cell].erase(pos);
    }
    for (const auto& cell : new_referenced_cells) {
        cell_to_depent_cells_[cell].insert(pos);
    }
}

void DependencyGraph::InvalidateCash(Position pos) {
    std::unordered_set<Position, Position::Hasher> visited;
    DfsForCashInvalidation(pos, visited);
}

void DependencyGraph::DfsForCashInvalidation(Position pos, std::unordered_set<Position, Position::Hasher>& visited) {
    visited.insert(pos);
    if (!cell_to_depent_cells_.count(pos)) {
        return;
    }
    for (const auto& cell : cell_to_depent_cells_.at(pos)) {
        if (!visited.count(cell)) {
            dynamic_cast<Cell*> (sheet_.GetCell(cell))->ResetCashedValue();
            DfsForCashInvalidation(cell, visited);
        }
    }
}


Impl::Value EmptyImpl::GetValue() const {
    return "";
}

std::string EmptyImpl::GetText() const {
    return "";    
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}

void EmptyImpl::ResetCashedValue() {

}

bool EmptyImpl::IsCashedValue() const {
    return true;
}

TextImpl::TextImpl(std::string text) 
    : text_(text)
{
    
}

Impl::Value TextImpl::GetValue() const {
    if (!text_.empty() && text_[0] == '\'') {
        return text_.substr(1);
    }
    return text_;
}

std::string TextImpl::GetText() const {
    return text_;    
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

void TextImpl::ResetCashedValue() {

}

bool TextImpl::IsCashedValue() const {
    return true;
}

FormulaImpl::FormulaImpl(std::string text, Position pos, SheetInterface* sheet)
    : pos_(pos), formula_(ParseFormula(std::move(text))), sheet_(sheet)
{
    assert(sheet);
}

Impl::Value FormulaImpl::GetValue() const {
    if (value_) {
        return *value_;
    }
    auto value = formula_->Evaluate(*sheet_);
    if (std::holds_alternative<double>(value)) {
        value_ = std::optional(Value(std::get<double>(value))); 
    }
    else {
        value_ = std::optional(Value(std::get<FormulaError>(value)));
    }
    return *value_;
}

std::string FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

void FormulaImpl::ResetCashedValue() {
    value_.reset();
}

bool FormulaImpl::IsCashedValue() const {
    return value_.has_value();
}

Cell::Cell(SheetInterface* sheet)
    : impl_(make_unique<EmptyImpl> ()), sheet_(sheet)
{
    
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {

    if (text.empty()) {
        impl_ = make_unique<EmptyImpl> ();
    }  else if (text[0] != FORMULA_SIGN || text.size() == 1) {
        impl_ = make_unique<TextImpl> (std::move(text));
    } else {
        auto tmp = make_unique<FormulaImpl> (text.substr(1), pos_, sheet_); 
        if ( ! graph_->TryChangeCell(pos_, tmp->GetReferencedCells())) {
            throw CircularDependencyException("circular dependency");
        }
        impl_ = std::move(tmp);
    }
}

void Cell::SetItems(Position pos, SheetInterface* sheet, DependencyGraph* graph) {
    pos_ = pos;
    sheet_ = sheet;
    graph_ = graph;
}

void Cell::ResetCashedValue() {
    impl_->ResetCashedValue();
}

bool Cell::IsCashedValue() const {
    return impl_->IsCashedValue();
}

void Cell::Clear() {
    graph_->TryChangeCell(pos_, {});
    impl_ = make_unique<EmptyImpl> ();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();  
}
std::string Cell::GetText() const {
    return impl_->GetText();
}  

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

  
