#pragma once

#include <array>
#include <sstream>
#include <gtest/gtest.h>
#include "bonk/backend/qbe/qbe_backend.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/parsing/parser.hpp"

void ensure_path(std::filesystem::path& path);
bool compile_bonk_source(const char* source, std::filesystem::path output_file);
bool compile_qbe(std::filesystem::path input_file, std::filesystem::path output_file);
bool compile_gcc(std::filesystem::path input_file, std::filesystem::path output_file);
bool compile_c_source(const char* source, std::filesystem::path output_file);
bool link_executable(const std::vector<std::filesystem::path>& object_files,
                     std::filesystem::path output_file);
std::string run_executable(std::filesystem::path executable_file);
bool run_bonk_with_counterpart(const char* bonk_source, const char* c_source,
                               const char* executable_name);