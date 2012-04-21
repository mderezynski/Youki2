#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <string>

typedef std::tr1::unordered_map<std::string, long> MapType;
typedef std::tr1::unordered_set<std::string> SetType;

typedef MapType::value_type ValuePair;
typedef SetType::value_type Value;

int main (int n_args, char **args)
{
  MapType m;
  SetType s;

  return 0;
}
