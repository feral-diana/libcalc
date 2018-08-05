//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#include "calc.hh"

#include <cmath>
#include <algorithm>
#include <string_view>

namespace Calc {
namespace {

std::set<char> all_operations = {'+', '-', '*', '/'};

FunctionsT default_functions =
{
    {"sqr",  [](NumberT x) { return x*x;} },
    {"sin",  sin},
    {"cos",  cos},
    {"exp",  exp},
    {"log",  log},
    {"sqrt", sqrt},
};

}

cCalculator::_sNode cCalculator::_FunctionProcess(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
{
    auto found = expr.find('(');
    if (found == std::string_view::npos)
        throw BadExpression(std::string(expr.begin(), expr.end()));

    std::string_view func = expr.substr(0, found);
    auto found_func = functions.find(func);
    if (found_func == functions.end())
        throw UnknownFunction(std::string(func));

    _sNode subnode = _BuildTree(expr.substr(found + 1, expr.size() - found - 2), functions, constants);
    _sNode node = {'f'};
    node.func = found_func->second;
    node.subnodes.push_back(std::move(subnode));
    return node;
}

cCalculator::_sNode cCalculator::_ValueProcess(std::string_view expr, VariablesT const& constants)
{
    _sNode node = {'i'};

    auto value = StringToNumber<NumberT>(expr);
    if (value)
    {
        node.value = *value;
        return node;
    }
    // it is a variable or constant

    auto found_const = constants.find(expr);
    if (found_const != constants.end())
    {
        node.value = found_const->second;
        return node;
    }

    node.operation_type = 'x';
    
    auto lb = _my_variables_indexes.lower_bound(expr);

    if(lb != _my_variables_indexes.end() && expr == lb->first)
    {
        node.variable_index = lb->second;
    }
    else
    {
        _my_variables_values.emplace_back();
        auto new_value = _my_variables_indexes.insert(lb, std::make_pair(std::string(expr), _my_variables_values.size()-1));
        node.variable_index = new_value->second;
    }
 
    return node;
}

cCalculator::_sNode cCalculator::_BuildTree(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
{
    if (expr.empty())
        throw BadExpression(std::string(expr.begin(), expr.end()));

    size_t start(0);
    int count(0);
    _sNode root = {'+'};
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
            _sNode subnode = !i and is_plus_minus ? ZERO : _BuildTree(expr.substr(start, i - start), functions, constants);
            subnode.inversion = operation_type == '-' or operation_type == '/';
            if (operation_type == '+' or operation_type == '-')
            {
                if (is_plus_minus)
                    root.subnodes.push_back(std::move(subnode));
                else
                {
                    _sNode mul_node;
                    mul_node.operation_type = '*';
                    if (!subnode.inversion)
                        mul_node.subnodes.push_back(std::move(subnode));
                    else
                    {
                        _sNode add_node = {'+'};
                        add_node.subnodes.push_back(ZERO);
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
                return _BuildTree(expr.substr(1, expr.size() - 2), functions, constants);
            else
                return _FunctionProcess(expr, functions, constants);
        }
        else
            return _ValueProcess(expr, constants);
    }
    else
    {
        _sNode subnode(_BuildTree(expr.substr(start), functions, constants));
        subnode.inversion = operation_type == '-' or operation_type == '/';

        if (operation_type == '+' or operation_type == '-')
            root.subnodes.push_back(std::move(subnode));
        else
            root.subnodes.back().subnodes.push_back(std::move(subnode));
    }

    return root;
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

FunctionsT const& cCalculator::DefaultFunctions()
{
    return default_functions;
}

void cCalculator::ClearExpression(std::string& expr, std::set<char> const& chars_to_remove)
{
    auto end_pos = std::remove_if(expr.begin(), expr.end(), [&chars_to_remove](char c) { return chars_to_remove.count(c);});
    expr.resize(end_pos - expr.begin());
}

cCalculator::cCalculator(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
    : _root(_BuildTree(expr, functions, constants))
{
}

NumberT cCalculator::_Calculate(_sNode const& root) const
{
    switch (root.operation_type)
    {
        case '+':
        {
            NumberT r = 0;
            for (auto const& node : root.subnodes)
            {
                NumberT next = _Calculate(node);
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
                NumberT next = _Calculate(node);
                if (node.inversion)
                    r /= next;
                else
                    r *= next;
            }
            return r;
        }
        case 'f':
            return root.func(_Calculate(root.subnodes[0]));
        case 'x':
            return *(_my_variables_values[root.variable_index]);
        case 'i':
            return root.value;
        default: //never
            throw std::exception();
    }
}

void cCalculator::_Optimize(_sNode& root)
{
    if (root.operation_type == 'i')
    {
        root.constant = true;
    }
    else if (root.operation_type != 'x')
    {
        for (auto& node : root.subnodes)
            _Optimize(node);

        auto bound = std::partition(root.subnodes.begin(), root.subnodes.end(), [](auto const& node) { return !node.constant; });
        root.constant = bound == root.subnodes.begin();
        if (root.constant)
        {
            root.value = _Calculate(root);
            root.operation_type = 'i';
            root.func = 0;    
            root.subnodes.clear();
        }
        else if (root.subnodes.end() - bound > 1)
        {
            _sNode const_part;
            const_part.operation_type = root.operation_type;       
            const_part.subnodes.insert(const_part.subnodes.end(), bound, root.subnodes.end());
            const_part.value = _Calculate(const_part);
            const_part.operation_type = 'i';
            const_part.subnodes.clear();
            const_part.constant = true;
            root.subnodes.erase(bound, root.subnodes.end());
            root.subnodes.push_back(std::move(const_part));
        }
    }
}

void cCalculator::Optimize()
{
    _Optimize(_root);
}

NumberT cCalculator::GetResult() const
{
    for (auto const& [name, index] : _my_variables_indexes)
       if (!_my_variables_values[index])
           throw UnknownVariable(name);

    return _Calculate(_root);
}

void cCalculator::ClearVariables()
{
    for (auto const& [name, index] : _my_variables_indexes)
    {
        (void)name;
        _my_variables_values[index].reset();
    }
}

void cCalculator::SetVariable(std::string const& name, NumberT value)
{
    auto found = _my_variables_indexes.find(name);
    if (found != _my_variables_indexes.end())
        _my_variables_values[found->second] = value;
}

void cCalculator::SetVariables(VariablesT const& variables)
{
    for (auto const& [name, index] : _my_variables_indexes)
    {
        auto found = variables.find(name);
        if (found == variables.end())
            throw UnknownVariable(name);
        _my_variables_values[index] = found->second;
    }
}

}

// vim: set et ts=4 sw=4:
//

