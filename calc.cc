//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "calc.hh"

#include <set>
#include <cmath>
#include <algorithm>

namespace Calc {
namespace {

std::set<char> all_operations = {'+', '-', '*', '/'};

sNode BuildTree(std::string const& expr, FunctionsT const& functions, VariablesT const& constants);

FunctionsT default_functions =
{
    {"sqr", [](NumberT x) { return x*x;} },
    {"sin", sin},
    {"cos", cos},
    {"exp", exp},
    {"log", log},
    {"sqrt", sqrt},
};

sNode FunctionProcess(std::string const& expr, FunctionsT const& functions, VariablesT const& constants)
{
    auto found = expr.find('(');
    if (found == std::string::npos)
        throw BadExpression(expr);

    std::string func(expr.begin(), expr.begin() + found);
    auto found_func = functions.find(func);
    if (found_func == functions.end())
        throw UnknownFunction(func);

    sNode subnode = BuildTree(expr.substr(found + 1, expr.size() - found - 2), functions, constants);
    sNode node;
    node.constant = subnode.constant;
    node.operation_type = 'f';
    node.func = found_func->second;
    node.subnodes.push_back(subnode);
    return node;
}

sNode ValueProcess(std::string const& expr, VariablesT const& constants)
{
    sNode node;

    try
    {
        node.value = StringToNumber(expr);
        node.operation_type = 'i';
        node.constant = true;
        return node;
    }
    catch(std::exception const&)
    {
        // it is a variable or constant
        auto found_const = constants.find(expr);
        if (found_const != constants.end())
        {
            node.value = found_const->second;
            node.operation_type = 'i';
            node.constant = true;
            return node;
        }
    }

    node.operation_type = 'x';
    node.operation = expr[0];
    return node;
}

sNode BuildTree(std::string const& expr, FunctionsT const& functions, VariablesT const& constants)
{
    if (expr.empty())
        throw BadExpression(expr);

    sNode zero;
    zero.operation_type = 'i';
    
    sNode root;
    size_t start(0);
    int count(0);
    root.operation_type = '+';
    char operation_type = '+';

    for (size_t i = 0; i != expr.size(); i++)
    {
        if (expr[i] == '(')
            ++count;
        else if (expr[i] == ')')
            --count;
        else if (all_operations.count(expr[i]) and !count)
        {
            bool is_plus_minus = expr[i] == '+' or expr[i] == '-';
            sNode subnode = !i and is_plus_minus ? zero : BuildTree(expr.substr(start, i - start), functions, constants);
            subnode.inversion = operation_type == '-' or operation_type == '/';
            if (operation_type == '+' or operation_type == '-')
            {
                if (is_plus_minus)
                    root.subnodes.push_back(subnode);
                else
                {
                    sNode mul_node;
                    mul_node.operation_type = '*';
                    if (!subnode.inversion)
                        mul_node.subnodes.push_back(subnode);
                    else
                    {
                        sNode add_node;
                        add_node.operation_type = '+';
                        add_node.subnodes.push_back(zero);
                        add_node.subnodes.push_back(subnode);
                        mul_node.subnodes.push_back(add_node);
                    }
                    root.subnodes.push_back(mul_node);
                }
            }
            else
                root.subnodes.back().subnodes.push_back(subnode);

            start = i + 1;

            operation_type = expr[i];
        }
    }

    if (root.subnodes.empty())
    {
        //1."cos(5+x)"
        //2."y" - variable or const
        //3."(7-z)" 
        if (expr.back() == ')')
        {
            if (expr.front() == '(')
                root = BuildTree(expr.substr(1, expr.size() - 2), functions, constants);
            else
                root = FunctionProcess(expr, functions, constants);
        }
        else
            root = ValueProcess(expr, constants);
    }
    else
    {
        sNode subnode(BuildTree(expr.substr(start), functions, constants));
        subnode.inversion = operation_type == '-' or operation_type == '/';

        if (operation_type == '+' or operation_type == '-')
            root.subnodes.push_back(subnode);
        else
            root.subnodes.back().subnodes.push_back(subnode);

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

UnknownFunction::UnknownFunction(std::string const& unknown_func)
    : std::runtime_error("Unknown function: " + unknown_func)
{
};

FunctionsT DefaultFunctions()
{
    return default_functions;
}

sNode Build(std::string const& expr, FunctionsT const& functions, VariablesT const& constants)
{
    std::string clear_expr(expr);

    std::string::iterator end_pos = std::remove(clear_expr.begin(), clear_expr.end(), ' ');
    clear_expr.erase(end_pos, clear_expr.end());

    return BuildTree(clear_expr, functions, constants);
}

NumberT Calculate(sNode const& root, VariablesT const& variables)
{
    switch (root.operation_type)
    {
        case '+':
        {
            NumberT r = 0;
            for (auto const& node : root.subnodes)
            {
                NumberT next = Calculate(node, variables);
                if (node.inversion)
                    r -= next;
                else
                    r += next;
            }
            return r;
        }
        case '*':
        {
            NumberT r = 1;
            for (auto const& node : root.subnodes)
            {
                NumberT next = Calculate(node, variables);
                if (node.inversion)
                    r /= next;
                else
                    r *= next;
            }
            return r;
        }
        case 'f':
            return root.func(Calculate(root.subnodes[0], variables));
        case 'x':
        {
            auto found = variables.find(root.operation);
            if (found != variables.end())
            	return found->second;
            throw UnknownVariable(root.operation);
        }
        case 'i':
            return root.value;
        defalt: //never
            throw std::exception();
    }
}

void Optimize(sNode& root)
{
    if (root.operation_type == 'i')
    {
        root.constant = true;
    }
    else if (root.operation_type != 'x')
    {
        for (auto& node : root.subnodes)
            Optimize(node);

        auto bound = std::partition(root.subnodes.begin(), root.subnodes.end(), [](auto const& node) { return !node.constant; });
        root.constant = bound == root.subnodes.begin();
        if (root.constant)
        {
            root.value = Calculate(root);
            root.operation_type = 'i';
            root.operation.clear();
            root.func = 0;    
            root.subnodes.clear();
        }
        else if (root.subnodes.end() - bound > 1)
        {
            sNode const_part;
            const_part.operation_type = root.operation_type;       
            const_part.subnodes.insert(const_part.subnodes.end(), bound, root.subnodes.end());
            const_part.value = Calculate(const_part);
            const_part.operation_type = 'i';
            const_part.subnodes.clear();
            const_part.constant = true;
            root.subnodes.erase(bound, root.subnodes.end());
            root.subnodes.push_back(const_part);
        }
    }
}

}

// vim: set et ts=4 sw=4:
//

