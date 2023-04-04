#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_clone_visitor.hpp"
#include "bonk/tree/ast_visitor.hpp"
#include "bonk/tree/binary_ast_deserializer.hpp"
#include "bonk/tree/binary_ast_serializer.hpp"
#include "bonk/tree/template_visitor.hpp"

namespace bonk {

struct TypeReferenceMetadata {
    std::vector<std::string> file_names;
    std::vector<unsigned long> identifier_files;

    void encode(const bonk::OutputStream& stream);
    void decode(const bonk::BufferInputStream& stream);
};

class SourceMetadata {
  public:
    static std::filesystem::path get_meta_path(const std::filesystem::path& path);

    SourceMetadata(MiddleEnd& middle_end, const std::filesystem::path& source_path);

    bool is_up_to_date_for(const SourceMetadata& other);
    void rebuild_metadata_ast(TreeNodeProgram* ast);
    bool metadata_is_newer_than_source();

    TreeNode* get_meta_ast();
    AST to_ast()&&;

    void fill_external_symbol_table(MiddleEnd& middle_end);

  protected:
    void read_metadata();
    void write_metadata_if_needed();

  private:
    bool metadata_rebuilt = false;

    std::filesystem::file_time_type check_date;
    std::filesystem::file_time_type changed_date;
    std::string_view meta_file_contents;

    bonk::AST meta_ast;
    std::filesystem::path source_path;
    std::filesystem::path meta_path = "";

    TypeReferenceMetadata type_reference_metadata;

    MiddleEnd& middle_end;
};

} // namespace bonk