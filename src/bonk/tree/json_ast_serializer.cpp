
#include "json_ast_serializer.hpp"

void bonk::JSONASTFieldCallback::operator()(bonk::OperatorType& value, std::string_view name) {
    serializer.field(name).block_string_field() << bonk::BONK_OPERATOR_NAMES[(int)value];
}
void bonk::JSONASTFieldCallback::operator()(bonk::NumberConstantContents& value,
                                              std::string_view name) {
    serializer.field(name).block_start_block();
    serializer.field("double_value").block_number_field(value.double_value);
    serializer.field("integer_value").block_number_field(value.integer_value);
    serializer.field("kind").block_string_field()
        << (value.kind == bonk::NumberConstantKind::rather_double ? "rather_double"
                                                                  : "rather_integer");
    serializer.close_block();
}
void bonk::JSONASTFieldCallback::operator()(bonk::TrivialTypeKind& value, std::string_view name) {
    serializer.field(name).block_string_field() << bonk::BONK_TRIVIAL_TYPE_KIND_NAMES[(int)value];
}
void bonk::JSONASTFieldCallback::operator()(bonk::ParserPosition& value, std::string_view name) {
    serializer.field(name).block_string_field() << value;
}
void bonk::JSONASTFieldCallback::operator()(std::string_view value, std::string_view name) {
    serializer.field(name).block_string_field() << value;
}
void bonk::JSONASTFieldCallback::operator()(TreeNodeType& value, std::string_view name) {
    auto field = serializer.field(name).block_string_field();
    switch (value) {

    case TreeNodeType::n_unset:
        field << "unset";
        break;
    case TreeNodeType::n_program:
        field << "program";
        break;
    case TreeNodeType::n_help_statement:
        field << "help_statement";
        break;
    case TreeNodeType::n_block_definition:
        field << "block_definition";
        break;
    case TreeNodeType::n_hive_definition:
        field << "hive_definition";
        break;
    case TreeNodeType::n_variable_definition:
        field << "variable_definition";
        break;
    case TreeNodeType::n_parameter_list_definition:
        field << "parameter_list_definition";
        break;
    case TreeNodeType::n_parameter_list_item:
        field << "parameter_list_item";
        break;
    case TreeNodeType::n_identifier:
        field << "identifier";
        break;
    case TreeNodeType::n_code_block:
        field << "code_block";
        break;
    case TreeNodeType::n_array_constant:
        field << "array_constant";
        break;
    case TreeNodeType::n_number_constant:
        field << "number_constant";
        break;
    case TreeNodeType::n_string_constant:
        field << "string_constant";
        break;
    case TreeNodeType::n_bonk_statement:
        field << "bonk_statement";
        break;
    case TreeNodeType::n_brek_statement:
        field << "brek_statement";
        break;
    case TreeNodeType::n_hive_access:
        field << "hive_access";
        break;
    case TreeNodeType::n_loop_statement:
        field << "loop_statement";
        break;
    case TreeNodeType::n_primitive_type:
        field << "primitive_type";
        break;
    case TreeNodeType::n_binary_operation:
        field << "binary_operation";
        break;
    case TreeNodeType::n_unary_operation:
        field << "unary_operation";
        break;
    case TreeNodeType::n_many_type:
        field << "many_type";
        break;
    case TreeNodeType::n_call:
        field << "call";
        break;
    case TreeNodeType::n_cast:
        field << "cast";
        break;
    case TreeNodeType::n_null:
        field << "null";
        break;
    }
}
