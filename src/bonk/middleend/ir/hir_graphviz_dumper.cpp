
#include "hir_graphviz_dumper.hpp"
#include "hir_printer.hpp"

void bonk::HIRGraphvizDumper::dump(bonk::IRProgram& program) {
    stream.get_stream() << "digraph G {\n"
                           "    node [shape=box, fontname=\"Courier New\", fontsize=10, "
                           "style=\"rounded,filled\", fillcolor=\"#F5F5F5\", color=\"#666666\", "
                           "penwidth=1.5, cellpadding=4, align=\"left\"];\n";

    for (auto& procedure : program.procedures) {
        dump(*procedure);
    }
    stream.get_stream() << "}\n";
}

void bonk::HIRGraphvizDumper::dump(bonk::IRProcedure& procedure) {
    for (int i = 0; i < procedure.base_blocks.size(); i++) {
        auto& block = procedure.base_blocks[i];
        stream.get_stream() << "    block" << (i + procedure_base_index)
                            << " [label=<<table border=\"0\" cellspacing=\"0\" cellborder=\"0\">\n";
        stream.get_stream() << "        <tr><td align=\"center\">Block " << i << "</td></tr>\n";

        for (auto& instruction : block->instructions) {
            stream.get_stream() << "        <tr><td align=\"left\">";
            dump_instruction_text(procedure.program, instruction);
            stream.get_stream() << "</td></tr>\n";
        }

        stream.get_stream() << "    </table>>];\n";

        for (auto& next : block->successors) {

            // TODO: There should be a better way to do this
            int next_index = 0;
            while (next_index < procedure.base_blocks.size() &&
                   procedure.base_blocks[next_index].get() != next)
                next_index++;

            stream.get_stream() << "    block" << (i + procedure_base_index) << " -> block" << (next_index + procedure_base_index) << ";\n";
        }
    }
    procedure_base_index += procedure.base_blocks.size();
}

void bonk::HIRGraphvizDumper::dump_instruction_text(bonk::IRProgram& program,
                                                    bonk::IRInstruction* instruction) {
    buffer.str("");
    buffer.clear();
    printer.print(program, *((HIRInstruction*)instruction));
    for (char c : buffer.str()) {
        switch(c) {
                case '<': stream.get_stream() << "&lt;"; break;
                case '>': stream.get_stream() << "&gt;"; break;
                case '&': stream.get_stream() << "&amp;"; break;
                case '\n': break;
                default: stream.get_stream() << c;
        }
    }
}