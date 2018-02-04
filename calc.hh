#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>

namespace Calc {

struct BadExpression : public std::runtime_error
{
    BadExpression(std::string const& bad_expression);
};

struct sNode
{
    std::string operation;
    std::vector<sNode> subnodes;
};

typedef std::map<std::string, long double> VariablesT;
typedef std::map<std::string, std::function<long double(long double)>> FunctionsT;

sNode Build(std::string const& expr);
FunctionsT DefaultFunctions();
long double Calculate(sNode const& root, VariablesT const& variables = VariablesT(), FunctionsT const& functions = DefaultFunctions());

}

// vim: set et ts=4 sw=4:
//

