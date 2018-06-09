//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "calc.hh"

#include <set>
#include <cmath>
#include <algorithm>

namespace Calc {
namespace {

std::set<char> plus_minus = {'+', '-'};
std::set<char> mul_div = {'*', '/'};

sNode EmptyPostProcess(std::string expr);
sNode BuildTree(std::string expr, std::set<char> const& operations, std::function<sNode(std::string)> post_process = EmptyPostProcess);
sNode MainProcess(std::string operation);

FunctionsT functions =
{
    {"sqr", [](NumberT x) { return x*x;} },
    {"sin", sin},
    {"cos", cos},
    {"exp", exp},
    {"log", log},
    {"sqrt", sqrt},
};

sNode EmptyPostProcess(std::string expr)
{
    if (expr.empty())
        throw BadExpression(expr);

    if (expr.back() == ')')
    {
        for (auto function : functions)
        {
            if (expr.find(function.first + "(") == 0)
            {
                sNode node;
                node.operation_type = 'f';
                node.operation = function.first;
                sNode subnode = MainProcess(std::string(expr, function.first.size() + 1, expr.size() - function.first.size() - 2));
                node.subnodes.push_back(subnode);
                return node;
            }
        }
    }

    sNode node;

    try
    {
         node.value = StringToNumber(expr);
         node.operation_type = 'i';
         return node;
    }
    catch(std::exception const&)
    {
        // it is a variable
    }

    node.operation_type = 'x';
    node.operation = expr;
    return node;
}

sNode MainProcess(std::string operation)
{
    return BuildTree(operation, plus_minus, [](std::string expr)
                    {
                        return BuildTree(expr, mul_div);
                    });
}

sNode BuildTree(std::string expr, std::set<char> const& operations, std::function<sNode(std::string)> post_process)
{
    if (expr[0] == '+' || expr[0] == '-')
        expr = "0" + expr;

    sNode root;
    size_t start(0);
    int count(0);
    for (size_t i = 0; i != expr.size(); i++)
    {
        if (expr[i] == '(')
            count++;
        else if (expr[i] == ')')
            count--;
        else if (operations.count(expr[i]) && !count)
        {
            sNode subnode = MainProcess(std::string(expr, start, i - start));
            root.subnodes.push_back(subnode);
            start = i + 1;

            if (root.operation_type)
            {
                sNode old_root(root);
                root.subnodes.clear();
                root.subnodes.push_back(old_root);
            }

            root.operation_type = expr[i];
        }
    }

    if (!root.operation_type)
    {
        if (expr.empty())
            throw BadExpression(expr);
        if (expr.front() == '(')
        {
            if (expr.back() != ')')
                root = post_process(expr);
            else
                root = MainProcess(std::string(expr.begin() + 1, expr.end() - 1));
        }
        else
            root = post_process(expr);
    }
    else
    {
        std::string operation(expr, start, expr.size() - start);
        sNode subnode(MainProcess(operation));
        root.subnodes.push_back(subnode);
    }

    return root;
}

}

BadExpression::BadExpression(std::string const& bad_expression)
    : std::runtime_error("Bad expression: " + bad_expression)
{};

UnknownVariable::UnknownVariable(std::string const& unknown_var)
    : std::runtime_error("Unknown variable: " + unknown_var)
{
};

FunctionsT DefaultFunctions()
{
    return functions;
}

sNode Build(std::string const& expr)
{
    std::string clear_expr(expr);

    std::string::iterator end_pos = std::remove(clear_expr.begin(), clear_expr.end(), ' ');
    clear_expr.erase(end_pos, clear_expr.end());

    return MainProcess(clear_expr);
}

NumberT Calculate(sNode const& root, VariablesT const& variables, FunctionsT const& functions)
{
    switch (root.operation_type)
    {
        case '+':
            return Calculate(root.subnodes[0], variables, functions) + Calculate(root.subnodes[1], variables, functions);
        case '-':
            return Calculate(root.subnodes[0], variables, functions) - Calculate(root.subnodes[1], variables, functions);
        case '*':
            return Calculate(root.subnodes[0], variables, functions) * Calculate(root.subnodes[1], variables, functions);
        case '/':
           return Calculate(root.subnodes[0], variables, functions) / Calculate(root.subnodes[1], variables, functions);
        case 'f':
        {
            auto func = functions.find(root.operation);
            return func->second(Calculate(root.subnodes[0], variables, functions));
        }
        case 'x':
        {
            auto found = variables.find(root.operation);
            if (found == variables.end())
                throw UnknownVariable(root.operation);
            return found->second;
        }
        case 'i':
            return root.value;
        defalt: //never
            throw std::exception();
    }
}

}

// vim: set et ts=4 sw=4:
//

