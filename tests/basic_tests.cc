//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "../calc.hh"

#include <iostream>
#include <limits>

using namespace Calc;

int check_expr(std::string const& expr,
               VariablesT variables,
               VariablesT constants,
               NumberT expected_result,
               NumberT expected_result_window = 0.000000000000001)
{
    sNode root = Build(expr, DefaultFunctions(), constants);
    NumberT result = Calculate(root, variables);
    NumberT diff = result - expected_result;
    if (diff > expected_result_window || diff < -expected_result_window)
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
    result += check_expr("sin(x)*sin(x)+cos(x)*cos(x)", variables, constants, 1.0);
    result += check_expr("exp(1)", variables, constants, 2.7182818284590452);
    result += check_expr("-1*9-5*(4+3*(4+8))", variables, constants, -209);
    result += check_expr("4-3*5", variables, constants, -11);
    result += check_expr("-(-1*9-5*(4+3*(4+8))/(-4*y))", variables, constants, -7.666666666666666667);
    result += check_expr("-a", VariablesT(), constants, -3);
    result += check_expr("5*(-a)", VariablesT(), constants, -15);
    result += check_expr("5+10-8+15+3+2+8-7-5-6-3+4+3*5*1*6/2/3", VariablesT(), constants, 33);
    result += check_expr("5/10/5/6*6", VariablesT(), constants, 0.1);
    result += check_expr("(5)*(6)", VariablesT(), constants, 30);
    result += check_expr("5+5*6", VariablesT(), constants, 35);
    result += check_expr("3+5*4+7", VariablesT(), constants, 30);
    result += check_expr("3-6/5", VariablesT(), constants, 1.8);
    result += check_expr("2+zz", variables, constants, 80);

    return result;
}

// vim: set et ts=4 sw=4:
//

