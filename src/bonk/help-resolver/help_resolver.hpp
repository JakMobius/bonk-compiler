#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include "bonk/compiler.hpp"
#include "bonk/middleend/metadata/metadata.hpp"
#include "bonk/middleend/middleend.hpp"

namespace bonk {

class HelpResolver {
  public:
    explicit HelpResolver(Compiler& compiler);

    bool compile_file(const std::filesystem::path& file_path);

    std::unique_ptr<SourceMetadata>
    get_recent_metadata_for_source(const std::filesystem::path& path);

    static std::filesystem::path get_output_path(const std::filesystem::path& path);

  private:
    Compiler& compiler;

    std::optional<std::string_view> get_source(bonk::Buffer& buffer,
                                               const std::filesystem::path& path);
    std::optional<bonk::AST> get_ast(const std::filesystem::path& file_path);
    std::optional<bonk::AST> get_transformed_ast(MiddleEnd& middle_end,
                                                 const std::filesystem::path& file_path);

    void recompile_file(MiddleEnd& middle_end, const std::filesystem::path& path,
                        TreeNodeProgram* ast);

    void file_not_found(TreeNode* node, const std::filesystem::path& path);
};

} // namespace bonk