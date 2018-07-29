//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "calc.hh"

#include <set>
#include <cmath>
#include <algorithm>
#include <string_view>
#include <iostream>

namespace Calc {
namespace {

std::set<char> all_operations = {'+', '-', '*', '/'};

using MyVariablesT = std::map<std::string, std::shared_ptr<std::optional<NumberT>>, std::less<>>;

sNode BuildTree(std::string_view expr, FunctionsT const& functions, MyVariablesT& my_variables, VariablesT const& constants);

FunctionsT default_functions =
{
    {"sqr",  [](NumberT x) { return x*x;} },
    {"sin",  sin},
    {"cos",  cos},
    {"exp",  exp},
    {"log",  log},
    {"sqrt", sqrt},
};

sNode FunctionProcess(std::string_view expr, FunctionsT const& functions, MyVariablesT& my_variables, VariablesT const& constants)
{
    auto found = expr.find('(');
    if (found == std::string_view::npos)
        throw BadExpression(std::string(expr.begin(), expr.end()));

    std::string_view func = expr.substr(0, found);
    auto found_func = functions.find(func);
    if (found_func == functions.end())
        throw UnknownFunction(std::string(func));

    sNode subnode = BuildTree(expr.substr(found + 1, expr.size() - found - 2), functions, my_variables, constants);
    sNode node;
    node.constant = subnode.constant;
    node.operation_type = 'f';
    node.func = found_func->second;
    node.subnodes.push_back(std::move(subnode));
    return node;
}

bool IsNumber(std::string_view expr)
{
    for (auto const& c : expr)
        if (c != '.' && c != ',' && (c < '0' || c > '9'))
            return false;
    return true;
}

sNode ValueProcess(std::string_view expr, MyVariablesT& my_variables, VariablesT const& constants)
{
    sNode node;

    try
    {
        if (IsNumber(expr))
        {
            node.value = StringToNumber<NumberT>(std::string(expr.begin(), expr.end()));
            node.operation_type = 'i';
            return node;
        }
    }
    catch(std::exception const&)
    {
        // it is a variable or constant
    }

    auto found_const = constants.find(expr);
    if (found_const != constants.end())
    {
        node.value = found_const->second;
        node.operation_type = 'i';
        return node;
    }

    node.operation_type = 'x';
    node.operation = expr;
    
    auto lb = my_variables.lower_bound(expr);

    if(lb != my_variables.end() && expr == lb->first)
    {
        node.variable = lb->second;
    }
    else
    {
        auto new_value = my_variables.insert(lb, std::make_pair(std::string(expr), new std::optional<NumberT>()));
        node.variable = new_value->second;
    }
 
    return node;
}

sNode BuildTree(std::string_view expr, FunctionsT const& functions, MyVariablesT& my_variables, VariablesT const& constants)
{
    if (expr.empty())
        throw BadExpression(std::string(expr.begin(), expr.end()));

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
            sNode subnode = !i and is_plus_minus ? zero : BuildTree(std::string_view(expr).substr(start, i - start), functions, my_variables, constants);
            subnode.inversion = operation_type == '-' or operation_type == '/';
            if (operation_type == '+' or operation_type == '-')
            {
                if (is_plus_minus)
                    root.subnodes.push_back(std::move(subnode));
                else
                {
                    sNode mul_node;
                    mul_node.operation_type = '*';
                    if (!subnode.inversion)
                        mul_node.subnodes.push_back(std::move(subnode));
                    else
                    {
                        sNode add_node;
                        add_node.operation_type = '+';
                        add_node.subnodes.push_back(zero);
                        add_node.subnodes.push_back(std::move(subnode));
                        mul_node.subnodes.push_back(std::move(add_node));
                    }
                    root.subnodes.push_back(std::move(mul_node));
                }
            }
            else
                root.subnodes.back().subnodes.push_back(std::move(subnode));

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
                root = BuildTree(std::string_view(expr).substr(1, expr.size() - 2), functions, my_variables, constants);
            else
                root = FunctionProcess(expr, functions, my_variables, constants);
        }
        else
            root = ValueProcess(expr, my_variables, constants);
    }
    else
    {
        sNode subnode(BuildTree(std::string_view(expr).substr(start), functions, my_variables, constants));
        subnode.inversion = operation_type == '-' or operation_type == '/';

        if (operation_type == '+' or operation_type == '-')
            root.subnodes.push_back(std::move(subnode));
        else
            root.subnodes.back().subnodes.push_back(std::move(subnode));

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

FunctionsT const& DefaultFunctions()
{
    return default_functions;
}

void ClearExpression(std::string& expr, std::set<char> const& chars_to_remove)
{
    auto end_pos = std::remove_if(expr.begin(), expr.end(), [&chars_to_remove](char c) { return chars_to_remove.count(c);});
    expr.resize(end_pos - expr.begin());
}

sNode Build(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
{
    MyVariablesT my_variables;
    return BuildTree(expr, functions, my_variables, constants);
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
            if (*root.variable)
                return **root.variable;

            auto found = variables.find(root.operation);
            if (found != variables.end())
            {
                //return found->second;
                *root.variable = found->second;
                return **root.variable;
            }

            throw UnknownVariable(root.operation);
        }
        case 'i':
            return root.value;
        default: //never
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
            root.subnodes.push_back(std::move(const_part));
        }
    }
}

}

// vim: set et ts=4 sw=4:
//

