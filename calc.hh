//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>
#include <optional>
#include <string_view>

namespace Calc {

struct BadExpression : public std::runtime_error
{
    BadExpression(std::string const& bad_expression);
    std::string expression;
};

struct UnknownVariable : public std::runtime_error
{
    UnknownVariable(std::string const& unknown_variable);
    std::string variable;
};

struct UnknownFunction : public std::runtime_error
{
   UnknownFunction(std::string const& unknown_function);
   std::string function;
};

void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove = {' '});

template<typename T>
inline std::optional<T> StringToNumber(std::string_view str);

template<typename NumberT = long double>
class cCalculator
{
public:
    using VariablesT = std::map<std::string, NumberT, std::less<>>;
    using FunctionT = std::function<NumberT(NumberT)>;
    using FunctionsT = std::map<std::string, FunctionT, std::less<>>;

    static FunctionsT const& DefaultFunctions();
    cCalculator(std::string_view expr, FunctionsT const& functions = DefaultFunctions(), VariablesT const& constants = VariablesT());
    void Optimize();
    void ClearVariables();
    void SetVariable(std::string const& name, NumberT value);
    void SetVariables(VariablesT const& variables);
    NumberT GetResult() const;

private:

    std::map<std::string, int, std::less<>> _my_variables_indexes;
    std::vector<std::optional<NumberT>> _my_variables_values;
    static FunctionsT _default_functions;

    struct _sNode
    {
        char operation_type = 0; //0, '+', '*', 'i', 'f', 'x'
        FunctionT func = nullptr;
        NumberT value = 0;
        int variable_index = -1;
        std::vector<_sNode> subnodes;
        bool constant = false;
        bool inversion = false; // true + operation_type('+') = '-'; true + operation_type('*') = '/'
        _sNode* parent = nullptr;

        _sNode() noexcept {};
        _sNode(_sNode&& node) = default;
        _sNode(_sNode const& node) = delete;
        _sNode& operator=(_sNode&& node) = delete;
        _sNode& operator=(_sNode const& node) = delete;

        _sNode& Add()
        {
            subnodes.emplace_back();
            return subnodes.back();
        }
    };

    _sNode _root;

    void _BuildTree(_sNode& root, std::string_view expr, FunctionsT const& functions, VariablesT const& constants);
    void _ValueProcess(_sNode& root, std::string_view expr, VariablesT const& constants);
    void _Optimize(_sNode& root);
    NumberT _Calculate(_sNode const& root) const;
};

}

#include "calc_impl.hh"

// vim: set et ts=4 sw=4:
