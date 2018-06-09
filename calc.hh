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

struct sNode
{
    std::string operation;
    std::vector<sNode> subnodes;
};

using NumberT = long double;

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

