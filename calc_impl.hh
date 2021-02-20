//          Copyright Diana Feral 2018
// Distributed under the MIT License, see accompanying file LICENSE

#pragma once

#include <complex>

namespace Calc {

template<>
std::optional<long double> StringToNumber<long double>(std::string_view str);

template<>
std::optional<int> StringToNumber<int>(std::string_view str);

template<>
std::optional<std::complex<long double>> StringToNumber<std::complex<long double>>(std::string_view str);

template<typename NumberT>
typename cCalculator<NumberT>::FunctionsT const& cCalculator<NumberT>::DefaultFunctions()
{
    return _default_functions;
}

template<typename NumberT>
cCalculator<NumberT>::cCalculator(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
    : _root(_BuildTree(expr, functions, constants))
{
}

template<typename NumberT>
void cCalculator<NumberT>::Optimize()
{
    _Optimize(_root);
}

template<typename NumberT>
void cCalculator<NumberT>::ClearVariables()
{
    for (auto const& [name, index] : _my_variables_indexes)
    {
        (void)name;
        _my_variables_values[index].reset();
    }
}

template<typename NumberT>
void cCalculator<NumberT>::SetVariable(std::string const& name, NumberT value)
{
    auto found = _my_variables_indexes.find(name);
    if (found != _my_variables_indexes.end())
        _my_variables_values[found->second] = value;
}

template<typename NumberT>
void cCalculator<NumberT>::SetVariables(VariablesT const& variables)
{
    for (auto const& [name, index] : _my_variables_indexes)
    {
        auto found = variables.find(name);
        if (found == variables.end())
            throw UnknownVariable(name);
        _my_variables_values[index] = found->second;
    }
}

template<typename NumberT>
NumberT cCalculator<NumberT>::GetResult() const
{
    for (auto const& [name, index] : _my_variables_indexes)
       if (!_my_variables_values[index])
           throw UnknownVariable(name);

    return _Calculate(_root);
}

template<typename NumberT>
typename cCalculator<NumberT>::_sNode cCalculator<NumberT>::_BuildTree(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
{

    auto find_op = [](std::string_view expr, size_t pos, char op1, char op2)
        {
            int count = 0;
            for (size_t i = pos; i != expr.size(); ++i)
            {
                char c = expr[i];
                if (c =='(')
                    ++count;
                else if (c == ')')
                    --count;
                else if (!count && (c == op1 || c == op2))
                    return i;
            }
            return expr.size();
        };

    auto processor = [this, &find_op, &functions, &constants](_sNode& root, std::string_view expr, char op1, char op2)
    {
        root.operation_type = op1;
        size_t pos = 0;
        while (true)
        {
            size_t next_pos = find_op(expr, pos, op1, op2);
            if (next_pos == expr.size() && pos == 0)//we unable to find the required operators so skip this step
                return;
            _sNode child = !next_pos ? _sNode{'i'}/*ZERO*/ : _BuildTree(expr.substr(pos, next_pos - pos), functions, constants);
            if (pos)
                child.inversion = expr[pos-1] == op2;
            root.subnodes.push_back(std::move(child));
            if (next_pos == expr.size())
                return;
            pos = next_pos + 1;
        }
    };

    _sNode root;

    processor(root, expr, '+', '-');
    if (root.subnodes.size() > 0)
        return root;

    processor(root, expr, '*', '/');

    if (root.subnodes.size() > 0)
        return root;

    if (expr.back() != ')')
        // expression doesn't contain '*', '/', '+', '-' and its last character isn't ')' so it can be constant or variable "5", "x"
        return _ValueProcess(expr, constants);
    if (expr.front() == '(')
        // expression == '(something)' so we will process it as 'something'
        return _BuildTree(expr.substr(1, expr.size() - 2), functions, constants);
    //expression == 'something1(something2)' so we process it as function call
    return _FunctionProcess(expr, functions, constants);
}

template<typename NumberT>
typename cCalculator<NumberT>::_sNode cCalculator<NumberT>::_FunctionProcess(std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
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

template<typename NumberT>
typename cCalculator<NumberT>::_sNode cCalculator<NumberT>::_ValueProcess(std::string_view expr, VariablesT const& constants)
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

template<typename NumberT>
void cCalculator<NumberT>::_Optimize(cCalculator::_sNode& root)
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

template<typename NumberT>
NumberT cCalculator<NumberT>::_Calculate(cCalculator::_sNode const& root) const
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

}

// vim: set et ts=4 sw=4:
