//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "../calc.hh"

#include <iostream>
#include <limits>

using namespace Calc;

int check_expr(std::string const& expr,
               VariablesT variables,
               NumberT expected_result,
               NumberT expected_result_window = 0.000000000000001)
{
    sNode root = Build(expr);
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

    VariablesT variables= {{"x", 5}, {"y", 3}};

    int result = 0;
    result += check_expr("sin(x)*sin(x)+cos(x)*cos(x)", variables, 1.0);
    result += check_expr("exp(1)", variables, 2.7182818284590452);
    result += check_expr("-(-1*9-5*(4+3*(4+8))/(-4*y))", variables, -7.666666666666666667);

    return result;
}

// vim: set et ts=4 sw=4:
//

