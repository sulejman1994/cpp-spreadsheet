#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet() 
    : graph_(*this)
{

}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    ValidatePosition(pos);
    if (table_.count(pos) && table_.at(pos).GetText() == text) {
        return;
    }
    table_[pos].SetItems(pos, this, &graph_);
    table_[pos].Set(text);
    SetEmptyNewReferencedCells(table_.at(pos).GetReferencedCells());
    if (pos.row >= size_.rows) {
        size_.rows = pos.row + 1;
    }
    if (pos.col >= size_.cols) {
        size_.cols = pos.col + 1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    ValidatePosition(pos);
    if (table_.count(pos)) {
        return &table_.at(pos);
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    ValidatePosition(pos);
    if (table_.count(pos)) {
        return &table_.at(pos);
    }
    return nullptr; 
}

void Sheet::ClearCell(Position pos) {
    ValidatePosition(pos);
    if (!table_.count(pos)) {
        return;
    }
    table_[pos].Clear();
    table_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    RecalculateSize();
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    RecalculateSize();
    for (int i = 0; i < size_.rows; ++i) {
        for (int k = 0; k < size_.cols; ++k) {
            if (table_.count({i, k})) {
                output << table_.at({i, k}).GetValue();
            }
            if (k != size_.cols - 1) {
                output << "\t";
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    RecalculateSize();
    for (int i = 0; i < size_.rows; ++i) {
        for (int k = 0; k < size_.cols; ++k) {
            if (table_.count({i, k})) {
                output << table_.at({i, k}).GetText();
            }
            if (k != size_.cols - 1) {
                output << "\t";
            } 
        }
        output << "\n";
    }
}

void Sheet::ValidatePosition(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position " + pos.ToString());
    }
}
  
void Sheet::RecalculateSize() const {
    if (table_.empty()) {
        size_ = Size{0, 0};
        return;
    }
    int max_col = -1;
    int max_row = -1;
    for (const auto& [pos, _] : table_) {
        if (table_.at(pos).GetText().empty()) {
            continue;
        }
        max_row = std::max(pos.row, max_row);
        max_col = std::max(pos.col, max_col);
    } 
    size_ = Size{max_row + 1, max_col + 1};
}

void Sheet::SetEmptyNewReferencedCells(const std::vector<Position>& referenced_cells) {
    for (const auto& cell : referenced_cells) {
        if (!table_.count(cell)) {
            table_[cell].Set("");
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
} 


