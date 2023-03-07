
#include "parser.hpp"

namespace bonk {

std::unique_ptr<TreeNode> Parser::parse_assignment() {

    Lexeme* lvalue = next_lexeme();
    if (lvalue->type == LexemeType::l_eof)
        return nullptr;

    eat_lexeme();
    Lexeme* assignment = next_lexeme();

    if (assignment->is(OperatorType::o_assign)) {
        // This is not an assignment. Exit silently.
        spit_lexeme();
        return nullptr;
    }

    if (lvalue->type != LexemeType::l_identifier) {
        linked_compiler.error().at(lvalue->start_position) << "expression is not assignable";
        return nullptr;
    }

    auto identifier = std::make_unique<TreeNodeIdentifier>();
    identifier->variable_name = std::get<IdentifierLexeme>(lvalue->data).identifier;
    identifier->source_position = lvalue->start_position;

    eat_lexeme();

    auto rvalue = parse_expression();
    if (!rvalue) {
        if (linked_compiler.state)
            return nullptr;
        linked_compiler.error().at(next_lexeme()->start_position) << "expected value";
        return nullptr;
    }

    auto expression = std::make_unique<TreeNodeOperator>();
    expression->oper_type = OperatorType::o_assign;

    expression->left = std::move(identifier);
    expression->right = std::move(rvalue);

    return expression;
}

std::unique_ptr<TreeNode> Parser::parse_code_line() {
    auto expression = parse_expression_leveled(true);
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    return nullptr;
}

std::unique_ptr<TreeNodeList> Parser::parse_block() {

    auto result = std::make_unique<TreeNodeList>();

    do {
        auto block = parse_code_line();
        if (linked_compiler.state)
            return nullptr;

        if (!block)
            break;

        result->list.push_back(std::move(block));
    } while (true);

    return result;
}

std::unique_ptr<TreeNodeBlockDefinition> Parser::parse_block_definition() {

    auto old_position = position;
    auto next = next_lexeme();

    if (!next->is(OperatorType::o_blok)) {
        return nullptr;
    }

    eat_lexeme();
    next = next_lexeme();

    if (next->is_identifier()) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected block name";
        position = old_position;
        return nullptr;
    }

    auto block_identifier = std::make_unique<TreeNodeIdentifier>();
    block_identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
    block_identifier->source_position = next->start_position;

    eat_lexeme();
    next = next_lexeme();

    if (next->is(BraceType('{'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '{'";
        position = old_position;
        return nullptr;
    }

    eat_lexeme();

    auto block = parse_block();
    if (!block) {
        position = old_position;
        return nullptr;
    }

    next = next_lexeme();

    if (next->is(BraceType('}'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '}'";
        position = old_position;
        return nullptr;
    }

    eat_lexeme();

    auto block_definition = std::make_unique<TreeNodeBlockDefinition>();

    block_definition->body = std::move(block);
    block_definition->block_name = std::move(block_identifier);

    return block_definition;
}

std::unique_ptr<TreeNode> Parser::parse_comparison() {

    auto expression = parse_math_expression();
    if (!expression)
        return nullptr;

    auto result = std::move(expression);

    do {
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_operator)
            break;
        OperatorType operator_type = std::get<OperatorLexeme>(next->data).type;
        if (!is_comparison_operator(operator_type))
            break;

        eat_lexeme();

        auto next_term = parse_math_term();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = operator_type;
        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

bool Parser::is_comparison_operator(OperatorType oper) {
    switch (oper) {
    case OperatorType::o_equal:
    case OperatorType::o_less:
    case OperatorType::o_greater:
    case OperatorType::o_less_equal:
    case OperatorType::o_greater_equal:
    case OperatorType::o_not_equal:
        return true;
    default:
        return false;
    }
}

std::unique_ptr<TreeNode> Parser::parse_expression() {
    return parse_expression_leveled(false);
}

std::unique_ptr<TreeNode> Parser::parse_expression_leveled(bool top_level) {

    auto expression = parse_logic_expression();
    if (linked_compiler.state)
        return nullptr;

    if (!expression) {
        expression = parse_unary_operator();
        if (linked_compiler.state)
            return nullptr;
    }

    if (top_level) {

        if (!expression) {
            expression = parse_var_definition();
            if (linked_compiler.state)
                return nullptr;
        }

        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_semicolon) {
            if (expression) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected semicolon";
            }
        } else {
            while (next->type == LexemeType::l_semicolon) {
                eat_lexeme();
                next = next_lexeme();
            }
            if (!expression) {
                return parse_expression();
            }
        }
    }

    if (!expression) {
        expression = parse_sub_block();
        if (linked_compiler.state)
            return nullptr;
    }

    return expression;
}

bool Parser::parse_global(TreeNodeList* target) {

    while (true) {
        unsigned long old_position = position;

        if (!parse_help(target)) {
            if (old_position == position)
                break;
            continue;
        }
    }

    if (!parse_program(target)) {
        if (!linked_compiler.state)
            linked_compiler.error() << "empty program";
        return false;
    }

    Lexeme* next = next_lexeme();

    if (next->type != LexemeType::l_eof) {
        if (!linked_compiler.state)
            linked_compiler.error().at(next_lexeme()->start_position) << "expected end of file";
        return false;
    }

    return true;
}

bool Parser::parse_help(TreeNodeList* target) {
    Lexeme* next = next_lexeme();
    if (!next->is(OperatorType::o_help)) {
        return false;
    }

    eat_lexeme();

    next = next_lexeme();
    if (next->type != LexemeType::l_identifier) {
        linked_compiler.error().at(next_lexeme()->start_position)
            << "expected library or file name to import";
        return false;
    }
    eat_lexeme();

    auto library_name = std::get<IdentifierLexeme>(next->data).identifier;

    std::string full_path = "/usr/local/lib/bonkScript/help/";
    full_path += library_name;
    full_path += ".bs";

    // Read the file
    std::ifstream ifstream(full_path);
    if (!ifstream.is_open()) {
        std::string library_name_str{library_name};
        linked_compiler.error().at(next_lexeme()->start_position)
            << "failed to open '" << library_name_str << "': " << strerror(errno);
        return false;
    }
    std::string source;
    ifstream.seekg(0, std::ios::end);
    source.reserve(ifstream.tellg());
    ifstream.seekg(0, std::ios::beg);
    source.assign((std::istreambuf_iterator<char>(ifstream)), std::istreambuf_iterator<char>());

    //    if (linked_compiler.lexical_analyzer.file_already_compiled(full_path)) {
    //        return true;
    //    }

    std::vector<Lexeme> lexemes =
        linked_compiler.lexical_analyzer.parse_file(full_path.c_str(), source.c_str());

    if (linked_compiler.state) {
        return true;
    }

    Parser nested_parser{linked_compiler};

    if (!nested_parser.append_file(&lexemes, target)) {
        return false;
    }

    return true;
}

std::unique_ptr<TreeNode> Parser::parse_logic_expression() {

    auto result = parse_logic_term();

    do {
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_operator)
            break;
        auto oper_type = std::get<OperatorLexeme>(next->data).type;

        eat_lexeme();

        auto next_term = parse_expression();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = oper_type;

        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

std::unique_ptr<TreeNode> Parser::parse_logic_term() {

    auto expression = parse_unary_operator();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    expression = parse_assignment();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    expression = parse_comparison();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    return expression;
}

std::unique_ptr<TreeNode> Parser::parse_math_expression() {
    unsigned long old_position = position;
    auto term = parse_math_term();
    if (!term && old_position != position)
        return nullptr;

    auto result = std::move(term);

    do {
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_operator)
            break;
        OperatorType operator_type = std::get<OperatorLexeme>(next->data).type;
        if (operator_type != OperatorType::o_plus && operator_type != OperatorType::o_minus)
            break;

        eat_lexeme();

        auto next_term = parse_math_term();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = operator_type;
        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

std::unique_ptr<TreeNode> Parser::parse_math_factor() {
    auto result = parse_reference();
    if (result)
        return result;

    if (next_lexeme()->type != LexemeType::l_number) {
        return nullptr;
    }

    auto result_number = std::make_unique<TreeNodeNumber>();
    auto lexeme = std::get<NumberLexeme>(next_lexeme()->data);
    result_number->float_value = lexeme.double_value;
    result_number->integer_value = lexeme.integer_value;
    eat_lexeme();
    return result_number;
}

std::unique_ptr<TreeNode> Parser::parse_math_term() {
    auto result = parse_math_factor();

    do {
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_operator)
            break;
        OperatorType operator_type = std::get<OperatorLexeme>(next->data).type;
        if (operator_type != OperatorType::o_multiply && operator_type != OperatorType::o_divide)
            break;

        eat_lexeme();

        auto next_term = parse_math_factor();
        if (!next_term) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected expression";
            }
            return nullptr;
        }

        auto sum = std::make_unique<TreeNodeOperator>();
        sum->oper_type = operator_type;
        sum->left = std::move(result);
        sum->right = std::move(next_term);
        result = std::move(sum);

    } while (true);

    return result;
}

std::unique_ptr<TreeNode> Parser::parse_global_definition() {
    auto expression = parse_block_definition();
    if (expression)
        return expression;
    if (linked_compiler.state)
        return nullptr;

    auto variable_definition = parse_var_definition();
    if (variable_definition) {
        if (variable_definition->is_contextual) {
            linked_compiler.error().at(variable_definition->source_position) << "global variables may not be contextual";
            return nullptr;
        }
        Lexeme* next = next_lexeme();
        if (next->type != LexemeType::l_semicolon) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected semicolon";
            return nullptr;
        }
        eat_lexeme();
        return variable_definition;
    }

    return nullptr;
}

bool Parser::parse_program(TreeNodeList* target) {

    auto block = parse_global_definition();
    if (!block) {
        return false;
    }

    while (block) {
        target->list.push_back(std::move(block));

        block = parse_global_definition();
    }

    return true;
}

std::unique_ptr<TreeNode> Parser::parse_reference() {
    std::unique_ptr<TreeNode> variable = nullptr;

    Lexeme* next = next_lexeme();

    bool is_call = false;

    if (next->is(OperatorType::o_call)) {
        is_call = true;

        eat_lexeme();
        next = next_lexeme();
    }

    if (next->is(BraceType('('))) {
        eat_lexeme();

        if (is_call) {
            linked_compiler.error().at(next_lexeme()->start_position) << "cannot call an expression";
            return nullptr;
        }

        variable = parse_expression();
        next = next_lexeme();

        if (next->is(BraceType(')'))) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected ')'";
            }
            return nullptr;
        }
        if (variable == nullptr) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "empty parenthesis in expression context";
            }

            eat_lexeme();
            return nullptr;
        }
        eat_lexeme();
    } else if (next->is_identifier()) {
        auto identifier = std::make_unique<TreeNodeIdentifier>();
        identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
        identifier->source_position = next->start_position;
        eat_lexeme();

        if (is_call) {
            auto arguments = parse_arguments();

            auto call = std::make_unique<TreeNodeCall>();
            call->source_position = identifier->source_position;
            call->call_function = std::move(identifier);
            call->call_parameters = std::move(arguments);

            return call;
        }

        variable = std::move(identifier);
    }

    if (variable == nullptr) {
        if (is_call) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected block name to call";
        }
        return nullptr;
    }

    return variable;
}

std::unique_ptr<TreeNodeList> Parser::parse_arguments() {

    Lexeme* next = next_lexeme();
    if (next->is(BraceType('['))) {
        return nullptr;
    }

    eat_lexeme();

    auto argument_list = std::make_unique<TreeNodeList>();

    next = next_lexeme();

    while (true) {

        if (!next->is_identifier()) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter name";
            return nullptr;
        }

        auto parameter_name = std::make_unique<TreeNodeIdentifier>();
        parameter_name->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
        parameter_name->source_position = next->start_position;

        eat_lexeme();
        next = next_lexeme();

        if (!next->is(OperatorType::o_assign)) {
            linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter value";
            return nullptr;
        }

        eat_lexeme();

        auto parameter_value = parse_expression();
        if (!parameter_value) {
            if (!linked_compiler.state) {
                linked_compiler.error().at(next_lexeme()->start_position) << "expected parameter value";
            }
            return nullptr;
        }

        auto argument = std::make_unique<TreeNodeCallParameter>();
        argument->parameter_name = std::move(parameter_name);
        argument->parameter_value = std::move(parameter_value);

        argument_list->list.push_back(std::move(argument));

        next = next_lexeme();
        if (next->type != LexemeType::l_comma) {
            break;
        }

        eat_lexeme();
        next = next_lexeme();
    }

    if (!next->is(BraceType(']'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected ']'";
        return nullptr;
    }

    eat_lexeme();

    return argument_list;
}

std::unique_ptr<TreeNode> Parser::parse_sub_block() {
    std::unique_ptr<TreeNode> expression = parse_loop();
    if (expression)
        return expression;
    return nullptr;
}

std::unique_ptr<TreeNodeCycle> Parser::parse_loop() {
    Lexeme* next = next_lexeme();

    if (!next->is(OperatorType::o_loop)) {
        return nullptr;
    }

    const ParserPosition& cycle_position = next->start_position;
    eat_lexeme();

    auto block = parse_nested_block();
    if (!block) {
        if (!linked_compiler.state)
            linked_compiler.error().at(next_lexeme()->start_position) << "expected cycle body";
        return nullptr;
    }

    auto cycle = std::make_unique<TreeNodeCycle>();
    cycle->source_position = cycle_position;
    cycle->body = std::move(block);
    return cycle;
}

std::unique_ptr<TreeNodeList> Parser::parse_nested_block() {
    Lexeme* next = next_lexeme();

    if(!next->is(BraceType('{'))) {
        return nullptr;
    }

    eat_lexeme();

    auto block = parse_block();
    if (!block) {
        if (linked_compiler.state)
            return nullptr;
        linked_compiler.error().at(next_lexeme()->start_position) << "expected block";
        return nullptr;
    }
    block->source_position = next->start_position;

    next = next_lexeme();

    if (!next->is(BraceType('}'))) {
        linked_compiler.error().at(next_lexeme()->start_position) << "expected '}'";
        return nullptr;
    }

    eat_lexeme();

    return block;
}

std::unique_ptr<TreeNode> Parser::parse_unary_operator() {
    Lexeme* lexeme = next_lexeme();

    if (lexeme->type != LexemeType::l_operator) {
        return nullptr;
    }
    auto oper = std::get<OperatorLexeme>(lexeme->data).type;

    switch(oper) {
    case OperatorType::o_brek: {
        eat_lexeme();
        auto no_arg_operator = std::make_unique<TreeNodeOperator>();
        no_arg_operator->oper_type = OperatorType::o_brek;
        no_arg_operator->source_position = lexeme->start_position;
        return no_arg_operator;
    }
    case OperatorType::o_bonk:
    case OperatorType::o_call: {
        eat_lexeme();
        auto expression = parse_expression();
        if (!expression) {
            if (linked_compiler.state)
                return nullptr;
            linked_compiler.error().at(next_lexeme()->start_position) << "missing operand";
        }

        auto unary_operator = std::make_unique<TreeNodeOperator>();
        unary_operator->oper_type = oper;
        unary_operator->source_position = lexeme->start_position;
        unary_operator->right = std::move(expression);

        return unary_operator;
    }
    default:
        return nullptr;
    }
}

std::unique_ptr<TreeNodeVariableDefinition> Parser::parse_var_definition() {

    Lexeme* next = next_lexeme();
    bool is_contextual = false;

    if (!next->is(OperatorType::o_bowl))
        return nullptr;

    eat_lexeme();
    next = next_lexeme();

    if (next->is_identifier()) {
        linked_compiler.error().at(next->start_position) << "expected variable name";
        return nullptr;
    }

    eat_lexeme();

    auto identifier = std::make_unique<TreeNodeIdentifier>();

    identifier->variable_name = std::get<IdentifierLexeme>(next->data).identifier;
    identifier->source_position = next->start_position;

    auto definition = std::make_unique<TreeNodeVariableDefinition>();
    definition->variable_name = std::move(identifier);
    definition->is_contextual = is_contextual;

    definition->source_position = next->start_position;

    next = next_lexeme();

    if (!next->is(OperatorType::o_assign))
        return definition;
    eat_lexeme();

    definition->variable_value = parse_expression();

    if (definition->variable_value == nullptr) {
        if (!linked_compiler.state) {
            linked_compiler.error().at(next_lexeme()->start_position)
                << "expected initial variable value";
        }
        return nullptr;
    }

    return definition;
}


}