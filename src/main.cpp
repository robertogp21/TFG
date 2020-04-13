
#include "decision_tree.h"
using namespace std;

int main(int argc, char* argv[]){
  DecisionTree dt;
  pair<map<string,vector<string>>, vector<Example>> p = ARRF_Reader::readFile(argv[1]);
  map<string,vector<string>> possible_values = p.first;
  vector<Example> examples = p.second;

  dt.setPossibleValues(possible_values);
  dt.ID3(examples);
}
