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

using NumberT = long double;

struct sNode
{
    char operation_type = 0; //0, '+', '-', '/', '*', 'i', 'f', 'x'
    std::string operation;
    NumberT value;
    std::vector<sNode> subnodes;
};

inline NumberT StringToNumber(std::string const& str)
{
    return std::stold(str);
}

typedef std::map<std::string, NumberT> VariablesT;
typedef std::map<std::string, std::function<NumberT(NumberT)>> FunctionsT;

sNode Build(std::string const& expr);
FunctionsT DefaultFunctions();
NumberT Calculate(sNode const& root, VariablesT const& variables = VariablesT(), FunctionsT const& functions = DefaultFunctions());

}

// vim: set et ts=4 sw=4:
//

