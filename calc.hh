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
using VariablesT = std::map<std::string, NumberT, std::less<>>;
using FunctionT = std::function<NumberT(NumberT)>;
using FunctionsT = std::map<std::string, FunctionT, std::less<>>;

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
inline std::optional<T> StringToNumber(std::string_view str);

template<>
inline std::optional<long double> StringToNumber<long double>(std::string_view str)
{
    bool point = false;
    long double multiplier = 0.1;
    long double r = 0;
    for (auto c : str)
    {
        switch (c)
	{
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
	case '8':
        case '9':
	    if (point)
	    {
                r += (c-'0')*multiplier;
		multiplier*=0.1;
	    }
	    else
                r = r*10+c-'0';
	    break;
	case ',':
	case '.':
            if (point)
                return {};
            point = true;
            break;
        default:
            return {};
	}
    }
    return r;
}

template<>
inline std::optional<int> StringToNumber<int>(std::string_view str)
{
    int r = 0;
    for (auto c : str)
    {
        if (c >= '0' and c <= '9')
            r = r*10+c-'0';
        else
            return {};
    }
    return r;
}

FunctionsT const& DefaultFunctions();
void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove = {' '});
sNode Build(std::string_view expr, FunctionsT const& functions = DefaultFunctions(), VariablesT const& constants = VariablesT());
void Optimize(sNode& root);
NumberT Calculate(sNode const& root, VariablesT const& variables = VariablesT());

}

// vim: set et ts=4 sw=4:
//

