//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "../calc.hh"

#include <iostream>
#include <limits>

using namespace Calc;

using NumberT = long double;
using VariablesT = cCalculator<NumberT>::VariablesT;
using ZNumberT = std::complex<NumberT>;
using ZVariablesT = cCalculator<ZNumberT>::VariablesT;

template<typename NumberT = long double>
int check_expr(std::string const& expr,
               typename cCalculator<NumberT>::VariablesT variables,
               typename cCalculator<NumberT>::VariablesT constants,
               NumberT expected_result,
               long double expected_result_window = 0.000000000000001)
{
    cCalculator calc(expr, cCalculator<NumberT>::DefaultFunctions(), constants);
    calc.SetVariables(variables);
    NumberT result = calc.GetResult();
    long double diff = std::abs(result - expected_result);
    if (diff > expected_result_window)
    {
        std::cout << "Expression test failed(" << expr <<"): " << result << " != " << expected_result << std::endl;
        return 1;
    }
    return 0;
}

int main()
{
    using dbl = std::numeric_limits<NumberT>;
    std::cout.precision(dbl::max_digits10);

    VariablesT variables= {{"x", 5}, {"y", 3}, {"zz", 78}};
    VariablesT constants= {{"a", 3}, {"b", 7}};

    int result = 0;
    result += check_expr<long double>("sin(x)*sin(x)+cos(x)*cos(x)", variables, constants, 1.0);
    result += check_expr<long double>("exp(1)", variables, constants, 2.7182818284590452);
    result += check_expr<long double>("-1*9-5*(4+3*(4+8))", variables, constants, -209);
    result += check_expr<long double>("4-3*5", variables, constants, -11);
    result += check_expr<long double>("-(-1*9-5*(4+3*(4+8))/(-4*y))", variables, constants, -7.666666666666666667);
    result += check_expr<long double>("-a", VariablesT(), constants, -3);
    result += check_expr<long double>("5*(-a)", VariablesT(), constants, -15);
    result += check_expr<long double>("5+10-8+15+3+2+8-7-5-6-3+4+3*5*1*6/2/3", VariablesT(), constants, 33);
    result += check_expr<long double>("5/10/5/6*6", VariablesT(), constants, 0.1);
    result += check_expr<long double>("(5)*(6)", VariablesT(), constants, 30);
    result += check_expr<long double>("5+5*6", VariablesT(), constants, 35);
    result += check_expr<long double>("3+5*4+7", VariablesT(), constants, 30);
    result += check_expr<long double>("3-6/5", VariablesT(), constants, 1.8);
    result += check_expr<long double>("2+zz", variables, constants, 80);
    result += check_expr<std::complex<long double>>("1+i", ZVariablesT(), ZVariablesT(), std::complex<long double>(1, 1));
    result += check_expr<std::complex<long double>>("(1+i)*(1-5i)", ZVariablesT(), ZVariablesT(), std::complex<long double>(6, -4));

    return result;
}

// vim: set et ts=4 sw=4:
//

