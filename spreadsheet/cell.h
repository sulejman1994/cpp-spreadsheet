#pragma once

#include "common.h"
#include "formula.h"
#include <optional>

class DependencyGraph {
public:
    explicit DependencyGraph(SheetInterface& sheet);
    bool TryChangeCell(Position pos, const std::vector<Position>& new_referenced_cells);
private:
    std::unordered_map<Position, std::unordered_set<Position, Position::Hasher>, Position::Hasher> cell_to_referenced_cells_;
    std::unordered_map<Position, std::unordered_set<Position, Position::Hasher>, Position::Hasher> cell_to_depent_cells_;
    SheetInterface& sheet_;

    bool IsCycle(Position pos) const;
    void DfsForCycle(Position pos, std::unordered_map<Position, int, Position::Hasher>& colors, bool& is_cycle) const;
    
    void RecalculateDepentEdges(Position pos, const std::unordered_set<Position, Position::Hasher>& old_referenced_cells,
        const std::vector<Position>& new_referenced_cells);

    void InvalidateCash(Position pos);
    void DfsForCashInvalidation(Position pos, std::unordered_set<Position, Position::Hasher>& visited);
};


class Impl {
public:
    using Value = std::variant<std::string, double, FormulaError>;
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual void ResetCashedValue() = 0;
    virtual bool IsCashedValue() const = 0;
};

class EmptyImpl : public Impl {
public:
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void ResetCashedValue() override;
    bool IsCashedValue() const override;
};  

class TextImpl : public Impl {
public:
    explicit TextImpl(std::string text);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void ResetCashedValue() override;
    bool IsCashedValue() const override;
private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string text, Position pos, SheetInterface* sheet);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void ResetCashedValue() override;
    bool IsCashedValue() const override;
private:
    Position pos_ = Position::NONE;
    std::unique_ptr<FormulaInterface> formula_ = nullptr;
    SheetInterface* sheet_ = nullptr;
    mutable std::optional<Value> value_; // храним кешированное значение
};


class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface* sheet = nullptr);
    ~Cell();

    void Set(std::string text); // в этом методе в случае формулы ищем циклическую зависимость с помощью графа зависимостей 
    // и если нашли, бросаем CircularDependencyException
    void SetItems(Position pos, SheetInterface* sheet, DependencyGraph* graph);

    void ResetCashedValue();
    bool IsCashedValue() const;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    Position GetPosition() const;

private:
    Position pos_ = Position::NONE;
    std::unique_ptr<Impl> impl_ = nullptr;
    SheetInterface* sheet_ = nullptr;
    DependencyGraph* graph_ = nullptr;
};
    

   
