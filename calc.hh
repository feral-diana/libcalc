//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>
#include <memory>
#include <optional>

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

typedef std::map<std::string, NumberT, std::less<>> VariablesT;
typedef std::map<std::string, FunctionT, std::less<>> FunctionsT;

struct sNode
{
    char operation_type = 0; //0, '+', '*', 'i', 'f', 'x'
    std::string operation;
    FunctionT func = 0;    
    NumberT value = 0;
    std::shared_ptr<std::optional<NumberT>> variable;
    std::vector<sNode> subnodes;
    bool constant = false;
    bool inversion = false; // true + operation_type('+') = '-'; true + operation_type('*') = '/'
};

template<typename T>
inline T StringToNumber(std::string const& str);

template<>
inline long double StringToNumber<long double>(std::string const& str)
{
    return std::stold(str);
}

template<>
inline int StringToNumber<int>(std::string const& str)
{
    return std::stoi(str);
}

FunctionsT const& DefaultFunctions();

void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove = {' '});
sNode Build(std::string_view expr, FunctionsT const& functions = DefaultFunctions(), VariablesT const& constants = VariablesT());
void Optimize(sNode& root);
NumberT Calculate(sNode const& root, VariablesT const& variables = VariablesT());

}

// vim: set et ts=4 sw=4:
//

