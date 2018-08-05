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

using NumberT = long double;
using VariablesT = std::map<std::string, NumberT, std::less<>>;
using FunctionT = std::function<NumberT(NumberT)>;
using FunctionsT = std::map<std::string, FunctionT, std::less<>>;

class cCalculator
{
public:
    static FunctionsT const& DefaultFunctions();
    static void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove = {' '});

    cCalculator(std::string_view expr, FunctionsT const& functions = DefaultFunctions(), VariablesT const& constants = VariablesT());
    void Optimize();
    void ClearVariables();
    void SetVariable(std::string const& name, NumberT value);
    void SetVariables(VariablesT const& variables);
    NumberT GetResult() const;

private:

    using _MyVariablesT = std::map<std::string, std::shared_ptr<std::optional<NumberT>>, std::less<>>;
    _MyVariablesT _my_variables;

    struct _sNode
    {
        char operation_type = 0; //0, '+', '*', 'i', 'f', 'x'
        FunctionT func = 0;    
        NumberT value = 0;
        std::shared_ptr<std::optional<NumberT>> variable;
        std::vector<_sNode> subnodes;
        bool constant = false;
        bool inversion = false; // true + operation_type('+') = '-'; true + operation_type('*') = '/'

        _sNode(char op_type = 0)
            : operation_type(op_type)
        {
        }
    };

    static const inline _sNode ZERO = {'i'};

    _sNode _root;

    _sNode _BuildTree(std::string_view expr, FunctionsT const& functions, VariablesT const& constants);
    _sNode _FunctionProcess(std::string_view expr, FunctionsT const& functions, VariablesT const& constants);
    _sNode _ValueProcess(std::string_view expr, VariablesT const& constants);
    void _Optimize(_sNode& node);
    NumberT _Calculate(_sNode const& node) const;
};

}

// vim: set et ts=4 sw=4:
