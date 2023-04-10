
#include "external_type_replacer.hpp"

void bonk::ExternalTypeReplacer::replace_types(bonk::TreeNode* node) {
    auto it = middle_end.type_table.type_cache.find(node);
    if(it == middle_end.type_table.type_cache.end()) return;

    auto type = it->second;

    if(type->kind != TypeKind::external) return;

    auto external_type = (ExternalType*)type;
    external_type->get_resolved();

    auto resolved = std::move(external_type->resolved);
    auto resolved_raw = resolved.get();

    middle_end.type_table.save_type(std::move(resolved));
    it->second = resolved_raw;
}

bool bonk::ExternalTypeReplacer::replace(bonk::AST& ast) {
    ast.root->accept(this);
    return true;
}
