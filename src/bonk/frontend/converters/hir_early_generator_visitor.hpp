#pragma once

#include <unordered_map>
#include "bonk/frontend/ast/ast_visitor.hpp"
#include "bonk/frontend/frontend.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/ir/instruction_pool.hpp"

namespace bonk {

class HIREarlyGeneratorVisitor;
struct HIRValue;

struct HIRLoopContext {
    TreeNode* loop_block = nullptr;
    int loop_start_label = -1;
    int loop_end_label = -1;
};

struct HIRValueRaw {
    IRRegister register_id;

    // Type field is moved to these structures on purpose
    // to make HIRValueRaw and HIRValueReference structures
    // self-contained and not depend on HIRValue structure
    Type* type = nullptr;
};

struct HIRValueReference {
    IRRegister register_id;
    HIRValueRaw reference_container;

    // See note above
    Type* type = nullptr;
};

struct HIRValue {
    HIREarlyGeneratorVisitor& visitor;
    std::variant<std::monostate, HIRValueReference, HIRValueRaw> value;

    HIRValue(HIREarlyGeneratorVisitor& visitor) : visitor(visitor) {}

    void set_reference(IRRegister register_id, HIRValueRaw reference_container, Type* type);

    void set_value(IRRegister register_id, Type* type);

    void release();

    bool is_reference() const;

    Type* get_type();

    void increase_reference_counter();
    void decrease_reference_count();

    ~HIRValue() {
        decrease_reference_count();
    }
};

struct AliveScope {
    TreeNode* block = nullptr;
    std::vector<std::unique_ptr<HIRValue>> alive_values {};

    AliveScope() = default;

    AliveScope(const AliveScope&) = delete;
    AliveScope& operator=(const AliveScope&) = delete;

    AliveScope(AliveScope&&) = default;
    AliveScope& operator=(AliveScope&&) = default;
};

class HIREarlyGeneratorVisitor : ASTVisitor {
    friend struct HIRValue;

    FrontEnd& front_end;
    HIRProgram* current_program;
    bool errors_occurred = false;

    TreeNodeHiveDefinition* current_hive_definition = nullptr;
    TreeNodeBlockDefinition* current_block_definition = nullptr;
    HIRProcedure* current_procedure = nullptr;
    HIRBaseBlock* current_base_block = nullptr;

    std::optional<HIRLoopContext> current_loop_context{};
    std::vector<AliveScope> alive_scopes;
    std::unique_ptr<HIRValue> return_value;

  public:
    HIREarlyGeneratorVisitor(FrontEnd& front_end) : front_end(front_end) {
    }

    std::unique_ptr<HIRProgram> generate(TreeNode* ast);

    HIROperationType convert_operation_to_hir(OperatorType type);
    HIRDataType convert_type_to_hir(Type* type);

    void visit(TreeNodeProgram* node) override;
    void visit(TreeNodeHelp* node) override;
    void visit(TreeNodeIdentifier* node) override;
    void visit(TreeNodeBlockDefinition* node) override;
    void visit(TreeNodeVariableDefinition* node) override;
    void visit(TreeNodeParameterList* node) override;
    void visit(TreeNodeParameterListItem* node) override;
    void visit(TreeNodeCodeBlock* node) override;
    void visit(TreeNodeArrayConstant* node) override;
    void visit(TreeNodeNumberConstant* node) override;
    void visit(TreeNodeStringConstant* node) override;
    void visit(TreeNodeBinaryOperation* node) override;
    void visit(TreeNodeUnaryOperation* node) override;
    void visit(TreeNodeHiveAccess* node) override;
    void visit(TreeNodeBonkStatement* node) override;
    void visit(TreeNodeLoopStatement* node) override;
    void visit(TreeNodeHiveDefinition* node) override;
    void visit(TreeNodeCall* node) override;
    void visit(TreeNodeCast* node) override;
    void visit(TreeNodeBrekStatement* node) override;
    void visit(TreeNodeNull* node) override;

    void compile_lazy_logic(TreeNodeBinaryOperation* node);

    std::unique_ptr<HIRValue> load_value(HIRValue* value);

    std::unique_ptr<HIRValue> eval(TreeNode* node);

    void kill_alive_variables(TreeNode* until_scope = nullptr);
    std::unique_ptr<HIRValue> assign(HIRValue* left, HIRValue* right);

    void write_file(TreeNode* operation);
    void write_location(TreeNode* operation);
};

} // namespace bonk