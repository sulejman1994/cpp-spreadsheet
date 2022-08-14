#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>
#include <functional>


class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;
     
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;
      
    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    static void ValidatePosition(Position pos);

private:
    std::unordered_map<Position, Cell, Position::Hasher> table_;
    DependencyGraph graph_;
    mutable Size size_ = {0, 0}; 
    
    void SetEmptyNewReferencedCells(const std::vector<Position>& referenced_cells);
    void RecalculateSize() const;

};

std::ostream& operator<<(std::ostream& out, const CellInterface::Value& value);

