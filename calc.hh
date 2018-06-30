//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>

namespace Calc {

struct BadExpression : public std::runtime_error
{
    BadExpression(std::string const& bad_expression);
};

struct UnknownVariable : public std::runtime_error
{
    UnknownVariable(std::string const& unknown_var);
};

struct UnknownFunction : public std::runtime_error
{
   UnknownFunction(std::string const& unknown_func);
};

using NumberT = long double;
using FunctionT = std::function<NumberT(NumberT)>;

typedef std::map<std::string, NumberT> VariablesT;
typedef std::map<std::string, FunctionT> FunctionsT;

struct sNode
{
    char operation_type = 0; //0, '+', '*', 'i', 'f', 'x'
    std::string operation;
    FunctionT func = 0;    
    NumberT value = 0;
    std::vector<sNode> subnodes;
    bool constant = false;
    bool inversion = false; // true + operation_type('+') = '-'; true + operation_type('*') = '/'
};

inline NumberT StringToNumber(std::string const& str)
{
    return std::stold(str);
}

FunctionsT DefaultFunctions();

sNode Build(std::string const& expr, FunctionsT const& functions = DefaultFunctions(), VariablesT const& constants = VariablesT());
void Optimize(sNode& root);
NumberT Calculate(sNode const& root, VariablesT const& variables = VariablesT());

}

// vim: set et ts=4 sw=4:
//

