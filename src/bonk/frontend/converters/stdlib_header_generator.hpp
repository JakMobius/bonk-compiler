#pragma once

#include "bonk/frontend/frontend.hpp"

namespace bonk {

class StdLibHeaderGenerator;

struct StdlibFunction {
    StdLibHeaderGenerator& generator;
    std::unique_ptr<TreeNodeBlockDefinition> function;

    StdlibFunction& parameter(std::string_view name, TrivialTypeKind type);
    StdlibFunction& return_type(TrivialTypeKind return_type);
    void attach(TreeNodeProgram* program);
};

class StdLibHeaderGenerator {
    friend class StdlibFunction;

    FrontEnd& front_end;
    AST* current_ast = nullptr;

  public:

    explicit StdLibHeaderGenerator(FrontEnd& front_end) : front_end(front_end) {
    }

    bonk::AST generate();

    StdlibFunction generate_stdlib_function(std::string_view name);

};

} // namespace bonk