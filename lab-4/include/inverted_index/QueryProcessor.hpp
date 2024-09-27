#ifndef INVERTED_INDEX_QUERY_PROCESSOR_HPP
#define INVERTED_INDEX_QUERY_PROCESSOR_HPP

#include "InvertedIndex.hpp"
#include <string>
#include <vector>

namespace inverted_index {

/**
 * @brief Processes Boolean queries (AND, OR, NOT) against an InvertedIndex.
 */
class QueryProcessor {
  public:
    /**
     * @brief Constructs a QueryProcessor with a reference to an InvertedIndex.
     *
     * @param index Reference to the InvertedIndex to query against.
     */
    QueryProcessor(const InvertedIndex &index);

    /**
     * @brief Parses and executes a Boolean query.
     *
     * @param query The Boolean query string (e.g., "term1 AND term2 OR NOT term3").
     * @return A vector of document IDs that satisfy the query.
     *
     * @throws std::invalid_argument If the query is malformed.
     */
    std::vector<int> executeQuery(const std::string &query);

  private:
    const InvertedIndex &index_;

    /**
     * @brief Tokenizes the input query string into terms and operators.
     *
     * @param query The raw query string.
     * @return A vector of tokens.
     */
    std::vector<std::string> tokenize(const std::string &query);

    /**
     * @brief Converts infix expression tokens to Reverse Polish Notation using the Shunting Yard algorithm.
     *
     * @param tokens The tokenized query in infix notation.
     * @return A vector of tokens in RPN.
     *
     * @throws std::invalid_argument If the query has mismatched parentheses or invalid tokens.
     */
    std::vector<std::string> toRPN(const std::vector<std::string> &tokens);

    /**
     * @brief Evaluates the query expressed in Reverse Polish Notation.
     *
     * @param rpn The query in RPN.
     * @return A vector of document IDs that satisfy the query.
     *
     * @throws std::invalid_argument If the RPN expression is malformed.
     */
    std::vector<int> evaluateRPN(const std::vector<std::string> &rpn);

    /**
     * @brief Performs set intersection between two sorted vectors of document IDs.
     *
     * @param list1 First sorted vector of document IDs.
     * @param list2 Second sorted vector of document IDs.
     * @return A sorted vector containing the intersection of list1 and list2.
     */
    std::vector<int> intersect(const std::vector<int> &list1, const std::vector<int> &list2);

    /**
     * @brief Performs set union between two sorted vectors of document IDs.
     *
     * @param list1 First sorted vector of document IDs.
     * @param list2 Second sorted vector of document IDs.
     * @return A sorted vector containing the union of list1 and list2.
     */
    std::vector<int> unionLists(const std::vector<int> &list1, const std::vector<int> &list2);

    /**
     * @brief Performs set difference (list1 - list2) between two sorted vectors of document IDs.
     *
     * @param list1 First sorted vector of document IDs.
     * @param list2 Second sorted vector of document IDs.
     * @return A sorted vector containing the difference of list1 and list2.
     */
    std::vector<int> difference(const std::vector<int> &list1, const std::vector<int> &list2);
};

} // namespace inverted_index

#endif
