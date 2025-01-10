// Verilog.cpp
#include "Verilog.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>
#include <algorithm>
#include <utility>

// Helper function to trim whitespace from both ends of a string
static inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Function to split a string by a delimiter and return as a vector
static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delimiter)) {
        tokens.push_back(trim(item));
    }
    return tokens;
}

// Function to collect all variables used in an expression
static std::unordered_set<std::string> extract_variables(const std::string& expression) {
    std::unordered_set<std::string> vars;
    std::regex var_regex(R"(\b[A-Za-z_][A-Za-z0-9_]*\b)");
    auto words_begin = std::sregex_iterator(expression.begin(), expression.end(), var_regex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string var = match.str();
        // Exclude 'assign' keyword if present
        if (var != "assign") { // 'assign' is a keyword, not a variable
            vars.insert(var);
        }
    }
    return vars;
}

std::string convert_verilog_to_blif(const std::string& verilog_file) {
    std::ifstream infile(verilog_file);
    if (!infile.is_open()) {
        return "";
    }

    std::string line;
    std::string module_name;
    std::vector<std::string> all_inputs;   // All declared inputs
    std::vector<std::string> outputs;
    std::unordered_set<std::string> wires;
    std::vector<std::pair<std::string, std::string>> assign_statements; // Preserve order

    // Regular expressions for parsing
    std::regex module_regex(R"(^\s*module\s+(\w+)\s*\(([^)]*)\)\s*;)");
    std::regex input_regex(R"(^\s*input\s+(.+);)");
    std::regex output_regex(R"(^\s*output\s+(.+);)");
    std::regex wire_regex(R"(^\s*wire\s+(.+);)");
    std::regex assign_regex(R"(^\s*assign\s+(\w+)\s*=\s*(.+?)\s*;)");

    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line.find("//") == 0) {
            continue; // Skip empty lines and comments
        }

        std::smatch match;
        if (std::regex_match(line, match, module_regex)) {
            module_name = match[1];
            std::string ports = match[2];
            std::vector<std::string> port_list = split(ports, ',');
            // Ports will be further classified as inputs/outputs later if needed
        }
        else if (std::regex_match(line, match, input_regex)) {
            std::string input_list = match[1];
            std::vector<std::string> tokens = split(input_list, ',');
            for (const auto& token : tokens) {
                all_inputs.push_back(token);
            }
        }
        else if (std::regex_match(line, match, output_regex)) {
            std::string output_list = match[1];
            std::vector<std::string> tokens = split(output_list, ',');
            for (const auto& token : tokens) {
                outputs.push_back(token);
            }
        }
        else if (std::regex_match(line, match, wire_regex)) {
            std::string wire_list = match[1];
            std::vector<std::string> tokens = split(wire_list, ',');
            for (const auto& token : tokens) {
                wires.insert(token);
            }
        }
        else if (std::regex_match(line, match, assign_regex)) {
            std::string lhs = match[1];
            std::string rhs = match[2];
            assign_statements.emplace_back(lhs, rhs);
        }
    }

    infile.close();

    // Collect all variables used in assign statements
    std::unordered_set<std::string> used_vars;
    for (size_t i = 0; i < assign_statements.size(); ++i) {
        const std::pair<std::string, std::string>& pair = assign_statements[i];
        std::unordered_set<std::string> vars = extract_variables(pair.second);
        used_vars.insert(vars.begin(), vars.end());
    }

    // Determine which inputs are actually used
    std::vector<std::string> used_inputs;
    for (size_t i = 0; i < all_inputs.size(); ++i) {
        const std::string& in = all_inputs[i];
        if (used_vars.find(in) != used_vars.end()) {
            used_inputs.push_back(in);
        }
    }

    // Prepare BLIF content
    std::stringstream blif;
    blif << ".model " << module_name << "\n";

    // Inputs
    blif << ".inputs";
    for (size_t i = 0; i < used_inputs.size(); ++i) {
        blif << " " << used_inputs[i];
    }
    blif << "\n";

    // Outputs
    blif << ".outputs";
    for (size_t i = 0; i < outputs.size(); ++i) {
        blif << " " << outputs[i];
    }
    blif << "\n";

    // Process assign statements to generate .names
    // Simple handling: assuming expressions are combinations of AND, OR, NOT
    // More complex expressions may require more sophisticated parsing

    for (size_t i = 0; i < assign_statements.size(); ++i) {
        const std::pair<std::string, std::string>& pair = assign_statements[i];
        std::string lhs = pair.first;
        std::string rhs = pair.second;

        // Remove all whitespace
        std::string expression = rhs;
        expression.erase(std::remove_if(expression.begin(), expression.end(), ::isspace), expression.end());

        std::vector<std::string> inputs_for_names;
        std::string op = ""; // Operation type: AND, OR, NOT, BUF

        // Determine the operation
        if (expression.find("!") != std::string::npos) {
            op = "NOT";
        }
        else if (expression.find('&') != std::string::npos) {
            op = "AND";
        }
        else if (expression.find('|') != std::string::npos) {
            op = "OR";
        }
        else {
            op = "BUF"; // Buffer (direct connection)
        }

        if (op == "NOT") {
            // Handle NOT operation
            size_t not_pos = expression.find('!');
            if (not_pos != std::string::npos && not_pos + 1 < expression.size()) {
                std::string input_var = expression.substr(not_pos + 1, expression.size() - not_pos - 1);
                // Handle multi-character variable names
                // For simplicity, assume variable names are single characters as in the sample
                // To handle multi-character names, more sophisticated parsing is needed
                // Here, we take the rest of the string as the variable name
                input_var = trim(input_var);
                inputs_for_names.push_back(input_var);

                // In BLIF, represent NOT as a truth table
                blif << ".names " << input_var << " " << lhs << "\n";
                blif << "0 1\n";
                blif << ".\n";
                continue;
            }
        }
        else if (op == "AND" || op == "OR") {
            // Split the expression by the operator
            char delimiter = (op == "AND") ? '&' : '|';
            std::vector<std::string> operands = split(expression, delimiter);
            inputs_for_names.insert(inputs_for_names.end(), operands.begin(), operands.end());
        }
        else if (op == "BUF") {
            // Buffer operation
            inputs_for_names.push_back(expression);
        }

        // Generate .names entry
        blif << ".names";
        for (size_t j = 0; j < inputs_for_names.size(); ++j) {
            blif << " " << inputs_for_names[j];
        }
        blif << " " << lhs << "\n";

        // Generate truth table based on the operation
        if (op == "AND") {
            // AND gate: output is 1 only when all inputs are 1
            std::string line_entry = "";
            for (size_t j = 0; j < inputs_for_names.size(); ++j) {
                line_entry += "1";
            }
            line_entry += " 1\n";
            blif << line_entry;
        }
        else if (op == "OR") {
            // OR gate: output is 1 when any input is 1
            for (size_t j = 0; j < inputs_for_names.size(); ++j) {
                std::string entry = "";
                for (size_t k = 0; k < inputs_for_names.size(); ++k) {
                    if (k == j) {
                        entry += "1";
                    }
                    else {
                        entry += "-";
                    }
                }
                entry += " 1\n";
                blif << entry;
            }
        }
        else if (op == "BUF") {
            // Buffer (direct connection)
            blif << "1 1\n";
        }
        // Add more operations if needed
        // blif << ".\n";
    }

    blif << ".end\n";

    // Write BLIF content to a file
    // Generate BLIF file path by replacing .v with .blif
    size_t dot_pos = verilog_file.find_last_of('.');
    std::string blif_file;
    if (dot_pos != std::string::npos) {
        blif_file = verilog_file.substr(0, dot_pos) + ".blif";
    }
    else {
        blif_file = verilog_file + ".blif";
    }

    std::ofstream outfile(blif_file);
    if (!outfile.is_open()) {
        return "";
    }

    outfile << blif.str();
    outfile.close();

    return blif_file;
}
