#include "inverted_index/QueryProcessor.hpp"
#include "log/Logger.hpp"
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace inverted_index {

QueryProcessor::QueryProcessor(const InvertedIndex& index)
    : index_(index) {}

std::vector<int> QueryProcessor::executeQuery(const std::string& query) {
    LOG_DEBUG("Executing query: '{}'", query);
    auto tokens = tokenize(query);
    auto rpn = toRPN(tokens);
    auto result = evaluateRPN(rpn);
    LOG_DEBUG("Query result has {} documents.", result.size());
    return result;
}

std::vector<std::string> QueryProcessor::tokenize(const std::string& query) {
    LOG_DEBUG("Tokenizing query.");
    std::vector<std::string> tokens;
    std::istringstream stream(query);
    std::string token;
    while (stream >> token) {
        // Convert to uppercase for operators to handle case-insensitive operators
        std::string upper_token;
        std::transform(token.begin(), token.end(), std::back_inserter(upper_token), ::toupper);
        if (upper_token == "AND" || upper_token == "OR" || upper_token == "NOT" ||
            upper_token == "(" || upper_token == ")") {
            tokens.emplace_back(upper_token);
        } else {
            // Assume the token is a term; normalize it (e.g., lowercase)
            std::string normalized_term;
            std::transform(token.begin(), token.end(), std::back_inserter(normalized_term), ::tolower);
            tokens.emplace_back(normalized_term);
        }
    }
    LOG_DEBUG("Tokenized query into {} tokens.", tokens.size());
    return tokens;
}

std::vector<std::string> QueryProcessor::toRPN(const std::vector<std::string>& tokens) {
    LOG_DEBUG("Converting tokens to Reverse Polish Notation.");
    std::vector<std::string> output;
    std::stack<std::string> op_stack;

    // Define operator precedence
    auto precedence = [](const std::string& op) -> int {
        if (op == "NOT") return 3;
        if (op == "AND") return 2;
        if (op == "OR") return 1;
        return 0;
    };

    for (const auto& token : tokens) {
        if (token == "AND" || token == "OR" || token == "NOT") {
            while (!op_stack.empty() && 
                   precedence(op_stack.top()) >= precedence(token)) {
                output.emplace_back(op_stack.top());
                op_stack.pop();
            }
            op_stack.emplace(token);
        }
        else if (token == "(") {
            op_stack.emplace(token);
        }
        else if (token == ")") {
            while (!op_stack.empty() && op_stack.top() != "(") {
                output.emplace_back(op_stack.top());
                op_stack.pop();
            }
            if (op_stack.empty()) {
                throw std::invalid_argument("Mismatched parentheses in query.");
            }
            op_stack.pop(); // Pop the '('
        }
        else {
            // Token is a term
            output.emplace_back(token);
        }
    }

    while (!op_stack.empty()) {
        if (op_stack.top() == "(" || op_stack.top() == ")") {
            throw std::invalid_argument("Mismatched parentheses in query.");
        }
        output.emplace_back(op_stack.top());
        op_stack.pop();
    }

    LOG_DEBUG("Converted to RPN with {} tokens.", output.size());
    return output;
}

std::vector<int> QueryProcessor::evaluateRPN(const std::vector<std::string>& rpn) {
    LOG_DEBUG("Evaluating RPN.");
    std::stack<std::vector<int>> eval_stack;
    int total_docs = index_.getTotalDocuments();
    std::vector<int> all_docs(total_docs);
    for (int i = 1; i <= total_docs; ++i) {
        all_docs[i - 1] = i;
    }

    for (const auto& token : rpn) {
        if (token == "AND" || token == "OR" || token == "NOT") {
            if (token == "NOT") {
                // Unary operator
                if (eval_stack.empty()) {
                    throw std::invalid_argument("Invalid query: NOT operator with no operand.");
                }
                auto operand = eval_stack.top();
                eval_stack.pop();
                // Compute NOT operand
                std::vector<int> result;
                size_t i = 0, j = 0;
                while (i < all_docs.size() && j < operand.size()) {
                    if (all_docs[i] < operand[j]) {
                        result.emplace_back(all_docs[i]);
                        ++i;
                    }
                    else if (all_docs[i] > operand[j]) {
                        ++j;
                    }
                    else {
                        // Document is in operand; skip it
                        ++i;
                        ++j;
                    }
                }
                while (i < all_docs.size()) {
                    result.emplace_back(all_docs[i]);
                    ++i;
                }
                eval_stack.emplace(result);
            }
            else {
                // Binary operators: AND, OR
                if (eval_stack.size() < 2) {
                    throw std::invalid_argument("Invalid query: Binary operator with insufficient operands.");
                }
                auto right = eval_stack.top();
                eval_stack.pop();
                auto left = eval_stack.top();
                eval_stack.pop();
                std::vector<int> result;
                if (token == "AND") {
                    result = intersect(left, right);
                }
                else if (token == "OR") {
                    result = unionLists(left, right);
                }
                eval_stack.emplace(result);
            }
        }
        else {
            // Token is a term
            auto postings = index_.getPostings(token);
            std::sort(postings.begin(), postings.end());
            eval_stack.emplace(postings);
        }
    }

    if (eval_stack.size() != 1) {
        throw std::invalid_argument("Invalid query: Excess operands.");
    }

    return eval_stack.top();
}

std::vector<int> QueryProcessor::intersect(const std::vector<int>& list1, const std::vector<int>& list2) {
    LOG_DEBUG("Performing intersection of two posting lists.");
    std::vector<int> result;
    size_t i = 0, j = 0;
    while (i < list1.size() && j < list2.size()) {
        if (list1[i] == list2[j]) {
            result.emplace_back(list1[i]);
            ++i;
            ++j;
        }
        else if (list1[i] < list2[j]) {
            ++i;
        }
        else {
            ++j;
        }
    }
    LOG_DEBUG("Intersection resulted in {} documents.", result.size());
    return result;
}

std::vector<int> QueryProcessor::unionLists(const std::vector<int>& list1, const std::vector<int>& list2) {
    LOG_DEBUG("Performing union of two posting lists.");
    std::vector<int> result;
    size_t i = 0, j = 0;
    while (i < list1.size() && j < list2.size()) {
        if (list1[i] == list2[j]) {
            result.emplace_back(list1[i]);
            ++i;
            ++j;
        }
        else if (list1[i] < list2[j]) {
            result.emplace_back(list1[i]);
            ++i;
        }
        else {
            result.emplace_back(list2[j]);
            ++j;
        }
    }
    // Append remaining elements
    while (i < list1.size()) {
        result.emplace_back(list1[i]);
        ++i;
    }
    while (j < list2.size()) {
        result.emplace_back(list2[j]);
        ++j;
    }
    LOG_DEBUG("Union resulted in {} documents.", result.size());
    return result;
}

std::vector<int> QueryProcessor::difference(const std::vector<int>& list1, const std::vector<int>& list2) {
    LOG_DEBUG("Performing difference (list1 - list2) of two posting lists.");
    std::vector<int> result;
    size_t i = 0, j = 0;
    while (i < list1.size() && j < list2.size()) {
        if (list1[i] == list2[j]) {
            ++i;
            ++j;
        }
        else if (list1[i] < list2[j]) {
            result.emplace_back(list1[i]);
            ++i;
        }
        else {
            ++j;
        }
    }
    // Append remaining elements from list1
    while (i < list1.size()) {
        result.emplace_back(list1[i]);
        ++i;
    }
    LOG_DEBUG("Difference resulted in {} documents.", result.size());
    return result;
}

} // namespace inverted_index

