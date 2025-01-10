// Verilog.h
#ifndef VERILOG_H
#define VERILOG_H

#include <string>

// Converts a Verilog file to a BLIF file.
// Returns the path to the generated BLIF file.
// Returns an empty string if conversion fails.
std::string convert_verilog_to_blif(const std::string& verilog_file);

#endif // VERILOG_H
