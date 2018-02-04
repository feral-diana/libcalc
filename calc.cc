#include "calc.hh"

#include <set>
#include <cmath>

namespace Calc {
namespace {

std::set<char> plus_minus = {'+', '-'};
std::set<char> mul_div = {'*', '/'};

sNode EmptyPostProcess(std::string expr);
sNode BuildTree(std::string expr, std::set<char> operations, std::function<sNode(std::string)> post_process = EmptyPostProcess);
sNode MainProcess(std::string operation);

FunctionsT functions =
{
    {"sqr", [](long double x) { return x*x;} },
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
                node.operation = function.first;
                sNode subnode = MainProcess(std::string(expr, function.first.size() + 1, expr.size() - function.first.size() - 2));
                node.subnodes.push_back(subnode);
                return node;
            }
        }
    }

    sNode node;
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

sNode BuildTree(std::string expr, std::set<char> operations, std::function<sNode(std::string)> post_process)
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

            if (!root.operation.empty())
            {
                sNode old_root(root);
                root.subnodes.clear();
                root.subnodes.push_back(old_root);
            }

            root.operation = expr[i];
        }
    }

    if (root.operation.empty())
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

FunctionsT DefaultFunctions()
{
    return functions;
}

sNode Build(std::string const& expr)
{
    return MainProcess(expr);
}

long double Calculate(sNode const& root, VariablesT const& variables, FunctionsT const& functions)
{
    if (root.operation == "+")
        return Calculate(root.subnodes[0], variables) + Calculate(root.subnodes[1], variables);
    else if (root.operation == "-")
        return Calculate(root.subnodes[0], variables) - Calculate(root.subnodes[1], variables);
    else if (root.operation == "*")
        return Calculate(root.subnodes[0], variables) * Calculate(root.subnodes[1], variables);
    else if (root.operation == "/")
        return Calculate(root.subnodes[0], variables) / Calculate(root.subnodes[1], variables);
    else if (functions.count(root.operation))
    {
	auto func = functions.find(root.operation);
        return func->second(Calculate(root.subnodes[0], variables));
    }
    else
    {
        auto found = variables.find(root.operation);
        if (found == variables.end())
            return std::stold(root.operation);
        return found->second;
    }
}

}

// vim: set et ts=4 sw=4:
//

