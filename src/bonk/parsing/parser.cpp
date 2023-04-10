
#include "parser.hpp"
#include <functional>

namespace bonk {

Parser::Parser(Compiler& compiler) : compiler(compiler) {
}

std::unique_ptr<TreeNodeProgram> Parser::parse_file(std::vector<Lexeme>* lexemes) {
    errors_occurred = false;
    input = lexemes;
    auto result = parse_program();
    if(errors_occurred) {
        return nullptr;
    }
    return result;
}

void Parser::spit_lexeme() {
    position--;
}

Lexeme* Parser::next_lexeme() {
    return &(*input)[(int)position];
}

void Parser::eat_lexeme() {
    Lexeme* c = next_lexeme();
    assert(c->type != LexemeType::l_eof);
    position++;
}

std::unique_ptr<TreeNodeProgram> Parser::parse_program() {
    // Program : HelpStatement* Definition*

    std::unique_ptr<TreeNodeProgram> program = std::make_unique<TreeNodeProgram>();
    program->source_position = next_lexeme()->start_position;

    while (next_lexeme()->is(OperatorType::o_help)) {
        program->help_statements.push_back(parse_help_statement());
    }

    while (next_lexeme()->type != LexemeType::l_eof) {
        auto definition = parse_definition();
        if(definition) {
            program->body.push_back(std::move(definition));
        } else {
            // Recover
            eat_lexeme();
        }
    }

    return program;
}

std::unique_ptr<TreeNodeHelp> Parser::parse_help_statement() {
    // HelpStatement : help StringConstant

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    Lexeme* identifier = next_lexeme();
    eat_lexeme();
    if (!identifier->is(LexemeType::l_string)) {
        error().at(identifier->start_position) << "Expected string literal after help statement";
        return nullptr;
    }

    std::unique_ptr<TreeNodeHelp> help = std::make_unique<TreeNodeHelp>();
    help->source_position = start_position;

    help->string = std::make_unique<TreeNodeStringConstant>();
    help->string->source_position = start_position;
    help->string->string_value = std::get<StringLexeme>(identifier->data).string;

    return help;
}

std::unique_ptr<TreeNode> Parser::parse_definition() {
    // Definition : BlokDefinition | VariableDefinition | HiveDefinition

    if (next_lexeme()->is(OperatorType::o_blok)) {
        return parse_blok_definition();
    } else if (next_lexeme()->is(OperatorType::o_bowl)) {
        return parse_variable_definition();
    } else if (next_lexeme()->is(OperatorType::o_hive)) {
        return parse_hive_definition();
    } else {
        error().at(next_lexeme()->start_position) << "Expected definition";
        return nullptr;
    }
}

std::unique_ptr<TreeNodeBlockDefinition> Parser::parse_blok_definition() {
    // BlokDefinition : blok Identifier ParameterListDefinition? BlokReturnType? CodeBlock

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    Lexeme* identifier = next_lexeme();
    eat_lexeme();
    if (!identifier->is(LexemeType::l_identifier)) {
        error().at(identifier->start_position) << "Expected blok name";
        return nullptr;
    }

    std::unique_ptr<TreeNodeBlockDefinition> blok = std::make_unique<TreeNodeBlockDefinition>();
    blok->source_position = start_position;

    blok->block_name = std::make_unique<TreeNodeIdentifier>();
    blok->block_name->source_position = identifier->start_position;
    blok->block_name->identifier_text = std::get<IdentifierLexeme>(identifier->data).identifier;

    if (next_lexeme()->is(BraceType('['))) {
        blok->block_parameters = parse_parameter_list_definition();
    }

    if (next_lexeme()->is(LexemeType::l_colon)) {
        eat_lexeme();
        blok->return_type = parse_type();
    }

    if (next_lexeme()->is(BraceType('{'))) {
        blok->body = parse_code_block();
    } else if (next_lexeme()->is(LexemeType::l_semicolon)) {
        eat_lexeme();
        return blok;
    } else {
        error().at(next_lexeme()->start_position) << "Expected code block or a semicolon";
        return nullptr;
    }

    return blok;
}

std::unique_ptr<TreeNodeVariableDefinition> Parser::parse_variable_definition() {
    // VariableDefinition : bowl Identifier (: Type)? (= Expression)?

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    Lexeme* identifier = next_lexeme();
    eat_lexeme();
    if (!identifier->is(LexemeType::l_identifier)) {
        error().at(identifier->start_position) << "Expected variable name";
        return nullptr;
    }

    std::unique_ptr<TreeNodeVariableDefinition> variable =
        std::make_unique<TreeNodeVariableDefinition>();
    variable->source_position = start_position;

    variable->variable_name = std::make_unique<TreeNodeIdentifier>();
    variable->variable_name->source_position = identifier->start_position;
    variable->variable_name->identifier_text =
        std::get<IdentifierLexeme>(identifier->data).identifier;

    if (next_lexeme()->is(LexemeType::l_colon)) {
        eat_lexeme();
        variable->variable_type = parse_type();
    }

    if (next_lexeme()->is(OperatorType::o_assign)) {
        eat_lexeme();
        variable->variable_value = parse_expression();
    }

    return variable;
}

std::unique_ptr<TreeNodeParameterListDefinition> Parser::parse_parameter_list_definition() {
    // ParameterListDefinition: [(VariableDefinition (, VariableDefinition)*)?]

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    std::unique_ptr<TreeNodeParameterListDefinition> parameter_list =
        std::make_unique<TreeNodeParameterListDefinition>();
    parameter_list->source_position = start_position;

    while (next_lexeme()->is(OperatorType::o_bowl)) {
        parameter_list->parameters.push_back(parse_variable_definition());
        if (next_lexeme()->is(LexemeType::l_comma)) {
            eat_lexeme();
        } else {
            break;
        }
    }

    if (!next_lexeme()->is(BraceType(']'))) {
        error().at(next_lexeme()->start_position) << "Expected closing brace for parameter list";
        return nullptr;
    }

    eat_lexeme();

    return parameter_list;
}

std::unique_ptr<TreeNodeCodeBlock> Parser::parse_code_block() {
    // CodeBlock: { (CodeBlock | LoopStatement | Statement;)* }

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    std::unique_ptr<TreeNodeCodeBlock> code_block = std::make_unique<TreeNodeCodeBlock>();
    code_block->source_position = start_position;

    while (!next_lexeme()->is(BraceType('}'))) {

        if (next_lexeme()->is(BraceType('{'))) {
            code_block->body.push_back(parse_code_block());
        } else if (next_lexeme()->is(OperatorType::o_loop)) {
            code_block->body.push_back(parse_loop_statement());
        } else {
            auto statement = parse_statement();
            if(!statement) {
                // Recover: Eat lexeme and continue parsing
                eat_lexeme();
                continue;
            }
            code_block->body.push_back(std::move(statement));
            if (next_lexeme()->is(LexemeType::l_semicolon)) {
                eat_lexeme();
            } else {
                error().at(next_lexeme()->start_position) << "Expected semicolon after statement";
                // Recover: assume semicolon is missing, continue parsing
            }
        }
    }

    eat_lexeme();

    return code_block;
}

std::unique_ptr<TreeNode> Parser::parse_statement() {
    // Statement: { Expression | BonkStatement | BrekStatement | VariableDeclaration }

    if (next_lexeme()->is(OperatorType::o_bowl)) {
        return parse_variable_definition();
    } else if (next_lexeme()->is(OperatorType::o_bonk)) {
        return parse_bonk_statement();
    } else if (next_lexeme()->is(OperatorType::o_brek)) {
        return parse_brek_statement();
    } else {
        return parse_expression();
    }
}

std::unique_ptr<TreeNodeBonkStatement> Parser::parse_bonk_statement() {
    // BonkStatement: bonk Expression?

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    std::unique_ptr<TreeNodeBonkStatement> bonk_statement =
        std::make_unique<TreeNodeBonkStatement>();
    bonk_statement->source_position = start_position;

    if (next_lexeme()->is(LexemeType::l_semicolon)) {
        return bonk_statement;
    }

    bonk_statement->expression = parse_expression();

    return bonk_statement;
}

std::unique_ptr<TreeNodeBrekStatement> Parser::parse_brek_statement() {
    // BrekStatement: brek

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    std::unique_ptr<TreeNodeBrekStatement> brek_statement =
        std::make_unique<TreeNodeBrekStatement>();
    brek_statement->source_position = start_position;

    return brek_statement;
}

std::unique_ptr<TreeNodeArrayConstant> Parser::parse_array_constant() {
    // ArrayConstant: [Expression*]

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();
    std::unique_ptr<TreeNodeArrayConstant> array_constant =
        std::make_unique<TreeNodeArrayConstant>();

    array_constant->source_position = start_position;

    while (!next_lexeme()->is(BraceType(']'))) {
        array_constant->elements.push_back(parse_expression());
        if (next_lexeme()->is(LexemeType::l_comma)) {
            eat_lexeme();
        } else {
            break;
        }
    }

    if (!next_lexeme()->is(BraceType(']'))) {
        error().at(next_lexeme()->start_position) << "Expected closing brace for array constant";
        return nullptr;
    }

    eat_lexeme();

    return array_constant;
}

std::unique_ptr<TreeNodeParameterList> Parser::parse_parameter_list() {
    // ParameterList: [(ParameterListItem (, ParameterListItem)*)?]

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();

    std::unique_ptr<TreeNodeParameterList> parameter_list =
        std::make_unique<TreeNodeParameterList>();
    parameter_list->source_position = start_position;

    if (next_lexeme()->is(BraceType(']'))) {
        eat_lexeme();
        return parameter_list;
    }

    parameter_list->parameters.push_back(parse_parameter_list_item());

    while (next_lexeme()->is(LexemeType::l_comma)) {
        eat_lexeme();
        parameter_list->parameters.push_back(parse_parameter_list_item());
    }

    if (!next_lexeme()->is(BraceType(']'))) {
        error().at(next_lexeme()->start_position) << "Expected closing brace for parameter list";
        return nullptr;
    }

    eat_lexeme();

    return parameter_list;
}

std::unique_ptr<TreeNodeParameterListItem> Parser::parse_parameter_list_item() {
    // ParameterListItem: Identifier = Expression

    auto start_position = next_lexeme()->start_position;

    if (!next_lexeme()->is(LexemeType::l_identifier)) {
        error().at(next_lexeme()->start_position) << "Expected identifier for parameter list item";
        return nullptr;
    }

    std::unique_ptr<TreeNodeIdentifier> identifier = std::make_unique<TreeNodeIdentifier>();
    identifier->source_position = next_lexeme()->start_position;
    identifier->identifier_text = std::get<IdentifierLexeme>(next_lexeme()->data).identifier;

    eat_lexeme();

    if (!next_lexeme()->is(OperatorType::o_assign)) {
        error().at(next_lexeme()->start_position)
            << "Expected '=' after identifier for parameter list item";
        return nullptr;
    }

    eat_lexeme();

    std::unique_ptr<TreeNodeParameterListItem> parameter_list_item =
        std::make_unique<TreeNodeParameterListItem>();
    parameter_list_item->source_position = start_position;
    parameter_list_item->parameter_name = std::move(identifier);
    parameter_list_item->parameter_value = parse_expression();

    return parameter_list_item;
}

std::unique_ptr<TreeNodeLoopStatement> Parser::parse_loop_statement() {
    // LoopStatement: loop ParameterListDefinition? CodeBlock

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();

    std::unique_ptr<TreeNodeLoopStatement> loop_statement =
        std::make_unique<TreeNodeLoopStatement>();
    loop_statement->source_position = start_position;

    if (next_lexeme()->is(BraceType('['))) {
        loop_statement->loop_parameters = parse_parameter_list_definition();
    }

    loop_statement->body = parse_code_block();

    return loop_statement;
}

std::unique_ptr<TreeNode> Parser::parse_type() {
    // Type: many Type | TrivialType | Identifier

    auto start_position = next_lexeme()->start_position;

    if (next_lexeme()->is(KeywordType::k_many)) {
        eat_lexeme();
        std::unique_ptr<TreeNodeManyType> many_type = std::make_unique<TreeNodeManyType>();
        many_type->source_position = start_position;
        many_type->parameter = parse_type();
        return many_type;
    }

    if (next_lexeme()->is(LexemeType::l_identifier)) {
        std::unique_ptr<TreeNodeIdentifier> identifier = std::make_unique<TreeNodeIdentifier>();
        identifier->source_position = next_lexeme()->start_position;
        identifier->identifier_text = std::get<IdentifierLexeme>(next_lexeme()->data).identifier;
        eat_lexeme();
        return identifier;
    }

    TrivialTypeKind primitive_type{};

    if (!next_lexeme()->is(LexemeType::l_keyword)) {
        error().at(next_lexeme()->start_position)
            << "Expected primitive type or identifier for type";
        return nullptr;
    }

    KeywordType keyword_type = std::get<KeywordLexeme>(next_lexeme()->data).type;

    if (keyword_type == KeywordType::k_null) {
        eat_lexeme();
        auto result = std::make_unique<TreeNodeNull>();
        result->source_position = start_position;
        return result;
    }

    switch (keyword_type) {
    case KeywordType::k_flot:
        primitive_type = TrivialTypeKind::t_flot;
        break;
    case KeywordType::k_dabl:
        primitive_type = TrivialTypeKind::t_dabl;
        break;
    case KeywordType::k_shrt:
        primitive_type = TrivialTypeKind::t_shrt;
        break;
    case KeywordType::k_buul:
        primitive_type = TrivialTypeKind::t_buul;
        break;
    case KeywordType::k_nubr:
        primitive_type = TrivialTypeKind::t_nubr;
        break;
    case KeywordType::k_long:
        primitive_type = TrivialTypeKind::t_long;
        break;
    case KeywordType::k_strg:
        primitive_type = TrivialTypeKind::t_strg;
        break;
    case KeywordType::k_nothing:
        primitive_type = TrivialTypeKind::t_nothing;
        break;
    default:
        error().at(next_lexeme()->start_position)
            << "Expected primitive type or identifier for type";
        return nullptr;
    }

    eat_lexeme();

    std::unique_ptr<TreeNodePrimitiveType> primitive_type_node =
        std::make_unique<TreeNodePrimitiveType>();
    primitive_type_node->source_position = start_position;
    primitive_type_node->primitive_type = primitive_type;
    return primitive_type_node;
}

std::unique_ptr<TreeNodeHiveDefinition> Parser::parse_hive_definition() {
    // HiveDefinition: hive Identifier { (BlokDefinition | VariableDefinition;)* }

    auto start_position = next_lexeme()->start_position;
    eat_lexeme();

    std::unique_ptr<TreeNodeHiveDefinition> hive_definition =
        std::make_unique<TreeNodeHiveDefinition>();
    hive_definition->source_position = start_position;

    if (!next_lexeme()->is(LexemeType::l_identifier)) {
        error().at(next_lexeme()->start_position) << "Expected identifier for hive definition";
        return nullptr;
    }

    hive_definition->hive_name = std::make_unique<TreeNodeIdentifier>();
    hive_definition->hive_name->source_position = next_lexeme()->start_position;
    hive_definition->hive_name->identifier_text =
        std::get<IdentifierLexeme>(next_lexeme()->data).identifier;

    eat_lexeme();

    if (!next_lexeme()->is(BraceType('{'))) {
        error().at(next_lexeme()->start_position) << "Expected opening brace for hive definition";
        return nullptr;
    }

    eat_lexeme();

    while (!next_lexeme()->is(BraceType('}'))) {
        if (next_lexeme()->is(OperatorType::o_blok)) {
            hive_definition->body.push_back(parse_blok_definition());
        } else if (next_lexeme()->is(OperatorType::o_bowl)) {
            hive_definition->body.push_back(parse_variable_definition());
            if (!next_lexeme()->is(LexemeType::l_semicolon)) {
                error().at(next_lexeme()->start_position)
                    << "Expected semicolon after variable definition";
                return nullptr;
            }
            eat_lexeme();
        } else {
            error().at(next_lexeme()->start_position)
                << "Expected blok or variable definition in hive definition";
            return nullptr;
        }
    }

    eat_lexeme();

    return hive_definition;
}

std::unique_ptr<TreeNode> Parser::parse_expression() {
    // Expression: ExpressionAssignment

    return parse_expression_assignment();
}

std::unique_ptr<TreeNode> Parser::parse_expression_assignment() {
    // ExpressionAssignment: OperatorExpression<ExpressionOr,         (= | += | -= | *= | /=)>

    return parse_operator_expression([this]() { return parse_expression_or(); },
                                     {OperatorType::o_assign, OperatorType::o_plus_assign,
                                      OperatorType::o_minus_assign, OperatorType::o_multiply_assign,
                                      OperatorType::o_divide_assign},
                                     true);
}

std::unique_ptr<TreeNode> Parser::parse_expression_or() {
    // ExpressionOr:         OperatorExpression<ExpressionAnd,        or>

    return parse_operator_expression([this]() { return parse_expression_and(); },
                                     {OperatorType::o_or});
}

std::unique_ptr<TreeNode> Parser::parse_expression_and() {
    // ExpressionAnd:        OperatorExpression<ExpressionEquality,   and>

    return parse_operator_expression([this]() { return parse_expression_equality(); },
                                     {OperatorType::o_and});
}

std::unique_ptr<TreeNode> Parser::parse_expression_equality() {
    // ExpressionEquality:   OperatorExpression<ExpressionRelational, (= | !=)>

    return parse_operator_expression([this]() { return parse_expression_relational(); },
                                     {OperatorType::o_equal, OperatorType::o_not_equal});
}

std::unique_ptr<TreeNode> Parser::parse_expression_relational() {
    // ExpressionRelational: OperatorExpression<ExpressionAdd,        (< | > | <= | >=)>

    return parse_operator_expression([this]() { return parse_expression_add(); },
                                     {OperatorType::o_less, OperatorType::o_greater,
                                      OperatorType::o_less_equal, OperatorType::o_greater_equal});
}

std::unique_ptr<TreeNode> Parser::parse_expression_add() {
    // ExpressionAdd:        OperatorExpression<ExpressionMul,        (+ | -)>

    return parse_operator_expression([this]() { return parse_expression_multiply(); },
                                     {OperatorType::o_plus, OperatorType::o_minus});
}

std::unique_ptr<TreeNode> Parser::parse_expression_multiply() {
    // ExpressionMul:        OperatorExpression<ExpressionUnary,      (* | /)>

    return parse_operator_expression([this]() { return parse_expression_unary(); },
                                     {OperatorType::o_multiply, OperatorType::o_divide});
}

std::unique_ptr<TreeNode> Parser::parse_expression_unary() {
    // ExpressionUnary: ExpressionPrimary | ExpressionCall | UnaryOperator ExpressionUnary

    auto start_position = next_lexeme()->start_position;

    if (next_lexeme()->is(OperatorType::o_call)) {
        return parse_expression_call();
    }

    auto unary_operators = {OperatorType::o_plus, OperatorType::o_minus, OperatorType::o_not};

    for (auto each_operator : unary_operators) {
        if (next_lexeme()->is(each_operator)) {
            eat_lexeme();
            std::unique_ptr<TreeNodeUnaryOperation> unary_operator =
                std::make_unique<TreeNodeUnaryOperation>();
            unary_operator->source_position = start_position;
            unary_operator->operand = parse_expression_unary();
            unary_operator->operator_type = each_operator;
            return unary_operator;
        }
    }

    return parse_expression_primary();
}

std::unique_ptr<TreeNode> Parser::parse_expression_primary() {
    // ExpressionPrimary: HiveAccess | Identifier | NumberConstant | StringConstant | ArrayConstant
    // | (Expression) | CodeBlock | NullKeyword

    auto start_position = next_lexeme()->start_position;

    if (next_lexeme()->is(LexemeType::l_identifier)) {
        std::unique_ptr<TreeNodeIdentifier> identifier = std::make_unique<TreeNodeIdentifier>();
        identifier->source_position = start_position;
        identifier->identifier_text = std::get<IdentifierLexeme>(next_lexeme()->data).identifier;
        eat_lexeme();
        if (next_lexeme()->is(OperatorType::o_of)) {
            eat_lexeme();
            std::unique_ptr<TreeNodeHiveAccess> hive_access =
                std::make_unique<TreeNodeHiveAccess>();
            hive_access->source_position = start_position;
            hive_access->field = std::move(identifier);
            hive_access->hive = parse_expression_primary();
            return hive_access;
        }
        return identifier;
    }

    if (next_lexeme()->is(LexemeType::l_number)) {
        std::unique_ptr<TreeNodeNumberConstant> number_constant =
            std::make_unique<TreeNodeNumberConstant>();
        number_constant->source_position = start_position;
        auto number_lexeme = std::get<NumberLexeme>(next_lexeme()->data);
        number_constant->contents = number_lexeme.contents;
        eat_lexeme();
        return number_constant;
    }

    if (next_lexeme()->is(LexemeType::l_string)) {
        std::unique_ptr<TreeNodeStringConstant> string_constant =
            std::make_unique<TreeNodeStringConstant>();
        string_constant->source_position = start_position;
        string_constant->string_value = std::get<StringLexeme>(next_lexeme()->data).string;
        eat_lexeme();
        return string_constant;
    }

    if (next_lexeme()->is(KeywordType::k_null)) {
        auto null_constant = std::make_unique<TreeNodeNull>();
        null_constant->source_position = start_position;
        eat_lexeme();
        return null_constant;
    }

    if (next_lexeme()->is(BraceType('['))) {
        return parse_array_constant();
    }

    if (next_lexeme()->is(BraceType('('))) {
        eat_lexeme();
        std::unique_ptr<TreeNode> expression = parse_expression();
        if (!next_lexeme()->is(BraceType(')'))) {
            error().at(next_lexeme()->start_position) << "Expected closing brace after expression";
            return nullptr;
        }
        eat_lexeme();
        return expression;
    }

    if (next_lexeme()->is(BraceType('{'))) {
        return parse_code_block();
    }

    error().at(next_lexeme()->start_position) << "Expected expression";

    return nullptr;
}

std::unique_ptr<TreeNode> Parser::parse_expression_call() {
    // ExpressionCall: @ Expression ArgumentList?

    auto start_position = next_lexeme()->start_position;

    eat_lexeme();

    std::unique_ptr<TreeNodeCall> call = std::make_unique<TreeNodeCall>();
    call->source_position = start_position;
    call->callee = parse_expression_primary();
    if (next_lexeme()->is(BraceType('['))) {
        call->arguments = parse_parameter_list();
    }

    return call;
}

MessageStreamProxy Parser::warning() const { return compiler.warning(); }
MessageStreamProxy Parser::error() {
    errors_occurred = true;
    return compiler.error();
}
MessageStreamProxy Parser::fatal_error() {
    errors_occurred = true;
    return compiler.fatal_error();
}

} // namespace bonk