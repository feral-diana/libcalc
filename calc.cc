//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "calc.hh"

namespace Calc {

BadExpression::BadExpression(std::string const& bad_expression)
        : std::runtime_error("Bad expression: " + bad_expression)
        , expression(bad_expression)
{
}

UnknownVariable::UnknownVariable(std::string const& unknown_variable)
    : std::runtime_error("Unknown variable: " + unknown_variable)
    , variable(unknown_variable)
{
}

UnknownFunction::UnknownFunction(std::string const& unknown_function)
       : std::runtime_error("Unknown function: " + unknown_function)
       , function(unknown_function)
{
}

template<>
std::optional<long double> StringToNumber<long double>(std::string_view str)
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
std::optional<int> StringToNumber<int>(std::string_view str)
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

template<>
std::optional<std::complex<long double>> StringToNumber<std::complex<long double>>(std::string_view str)
{
    if (str.empty())
        return {};
    if (str.back() == 'i')
    {
        str.remove_suffix(1);

        if (str.empty())
            return std::complex<long double>(0, 1);

        auto im = StringToNumber<long double>(str);
        if (im)
            return std::complex<long double>(0, *im);
        return {};
    }
    else
    {
        auto re = StringToNumber<long double>(str);
        if (re)
            return std::complex<long double>(*re, 0);
        return {};
    }
}

template<>
cCalculator<long double>::FunctionsT cCalculator<long double>::_default_functions =
{
    {"sqr",  [](long double x) { return x*x;} },
    {"sin",  sin},
    {"cos",  cos},
    {"exp",  exp},
    {"log",  log},
    {"sqrt", sqrt},
};

template<>
cCalculator<std::complex<long double>>::FunctionsT cCalculator<std::complex<long double>>::_default_functions = {};

void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove)
{
    auto end_pos = std::remove_if(expr.begin(), expr.end(), [&chars_to_remove](char c) { return chars_to_remove.count(c);});
    expr.resize(end_pos - expr.begin());
}

}

// vim: set et ts=4 sw=4:
