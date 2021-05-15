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
{
    _BuildTree(_root, expr, functions, constants);
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

size_t find_op(std::string_view expr, size_t pos);
bool inverse(char op);

template<typename NumberT>
void cCalculator<NumberT>::_BuildTree(_sNode& root, std::string_view expr, FunctionsT const& functions, VariablesT const& constants)
{
    root.operation_type = '+';
    _sNode* cur = &root;
    char prev_operation_type = '+';
    size_t prev_operation_pos = -1;
    while (true)
    {
        size_t operation_pos = find_op(expr, prev_operation_pos + 1);
        if (operation_pos == expr.size())
        {
            _sNode& val = cur->Add();
            val.inversion = inverse(prev_operation_type);
            _ValueProcess(val, expr.substr(prev_operation_pos + 1, operation_pos - (prev_operation_pos + 1)), constants);
            return;
        }
        char operation_type = expr[operation_pos];
        if (operation_type == '(')
        {
            std::string_view func_prefix = expr.substr(prev_operation_pos + 1, operation_pos - (prev_operation_pos + 1));

            if (prev_operation_type == ')')
                throw BadExpression(std::string(expr));

            _sNode& func = cur->Add();
            func.operation_type = 'f';
            if (!func_prefix.empty())
            {
                auto found = functions.find(std::string(func_prefix));
                func.func = found->second;
            }
            func.parent = cur;
            func.inversion = inverse(prev_operation_type);

            _sNode& next = func.Add();
            next.operation_type = '+';

            next.parent = &func;
            cur = &next;
        }
        else if (cur->operation_type == '+' && (operation_type == '*' || operation_type == '/'))
        {
            _sNode next;
            next.inversion = inverse(prev_operation_type);
            next.operation_type = '*';
            if (prev_operation_type != ')')
                _ValueProcess(next.Add(), expr.substr(prev_operation_pos + 1, operation_pos - (prev_operation_pos + 1)), constants);
            else
            {
                next.subnodes.push_back(std::move(cur->subnodes.back()));
                cur->subnodes.pop_back();
            }

            next.parent = cur;
            cur->subnodes.push_back(std::move(next));
            cur = &cur->subnodes.back();
        }
        else if (operation_type == ')' || (cur->operation_type == '*' && (operation_type == '+' || operation_type == '-')))
        {
            if (prev_operation_type != ')')
            {
                _sNode& val = cur->Add();
                val.inversion = inverse(prev_operation_type);
                _ValueProcess(val, expr.substr(prev_operation_pos + 1, operation_pos - (prev_operation_pos + 1)), constants);
            }
            if (operation_type == ')')
            {
                while(cur->operation_type != 'f')
                    cur = cur->parent;
            }

            cur = cur->parent;
        }
        else if (prev_operation_type != ')')
        {
            _sNode& val = cur->Add();
            val.inversion = inverse(prev_operation_type);
            _ValueProcess(val, expr.substr(prev_operation_pos + 1, operation_pos - (prev_operation_pos + 1)), constants);
        }
        if (operation_pos == expr.size() - 1 && expr.back() == ')')
            return;
        prev_operation_type = operation_type;
        prev_operation_pos = operation_pos;
    }
}

template<typename NumberT>
void cCalculator<NumberT>::_ValueProcess(_sNode& node, std::string_view expr, VariablesT const& constants)
{
    node.operation_type = 'i';
    if (expr.empty())
    {
        node.value = 0;
        return;
    }

    auto value = StringToNumber<NumberT>(expr);
    if (value)
    {
        node.value = *value;
        return;
    }
    // it is a variable or constant

    auto found_const = constants.find(expr);
    if (found_const != constants.end())
    {
        node.value = found_const->second;
        return;
    }

    node.operation_type = 'x';

    auto lb = _my_variables_indexes.lower_bound(expr);

    if(lb != _my_variables_indexes.end() && expr == lb->first)
        node.variable_index = lb->second;
    else
    {
        _my_variables_values.emplace_back();
        auto new_value = _my_variables_indexes.emplace_hint(lb, std::string(expr), _my_variables_values.size()-1);
        node.variable_index = new_value->second;
    }
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
        {
            auto res = _Calculate(root.subnodes[0]);
            if (!root.func)
                return res;
            return root.func(res);
        }
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
