#include "decision_tree.h"

template <typename T>
T sum(const vector<T>& v){
  return accumulate(v.begin(), v.end(), 0);
}

int argmax(const vector<double>& v){
  int pos_max = -1;
  double max = - numeric_limits<double>::infinity();

  for (int i = 0; i < v.size(); i++){
    if ( v[i] > max ){
      max = v[i];
      pos_max = i;
    }
  }

  return pos_max;
}

vector<string> getKeysFromMap(const map<string,vector<string>>& m){
  vector<string> keys;
  for (map<string,vector<string>>::const_iterator it = m.begin(); it != m.end(); ++it)
    keys.push_back(it->first);
  return keys;
}


/*  ______________________________________________________________________________ */
/* |                                                                             | */
/* |                         DECISION TREE CLASS                                 | */
/* |_____________________________________________________________________________| */


DecisionTree::DecisionTree(){
  root = NULL;
}

DecisionTree::~DecisionTree(){
  destroy(root);
}

void DecisionTree::setPossibleValues(const map<string,vector<string>>& possible_values){
  this->possible_values = possible_values;
}

void DecisionTree::destroy(Node*& node){
  if (node != NULL){
    Node* n;
    if ( !node->isLeaf() ){
      map<string,pair<Node*,int>> br = node->getBranches();
      for (map<string,pair<Node*,int>>::iterator it= br.begin(); it != br.end(); ++it){
        n = it -> second.first;
        destroy(n);
      }
    }

    delete node;
    node = NULL;
  }
}


bool DecisionTree::sameClass(const vector<Example> & examples) const{
  string target = examples[0].getTarget();

  for (int i = 1; i < examples.size(); i++){
    if (examples[i].getTarget() != target)
      return false;
  }
  return true;
}


string DecisionTree::mostCommonTarget(const vector<Example> & examples) const{
  map<string,int> counter = countClasses(examples);
  counter.erase("total");

  string target = "";
  int max = 0;
  for (map<string,int>::iterator it = counter.begin(); it != counter.end(); ++it){
    if ( it -> second > max){
      target = it -> first;
      max = it -> second;
    }
  }
  return target;
}


map<string, int> DecisionTree::countClasses(const vector<Example>& examples) const{
  map<string,int> counter;
  for (int i = 0; i < examples.size(); i++){
    ++counter[examples[i].getTarget()];
    ++counter["total"];
  }
  return counter;
}


map<string, int> DecisionTree::countExamples(const vector<Example>& examples, const string& attribute, const string& value) const{
  map<string,int> counter;
  for (int i = 0; i < examples.size(); i++){
    if (examples[i].at(attribute) == value){
      ++counter[examples[i].getTarget()];
      ++counter["total"];
    }
  }
  return counter;
}


double DecisionTree::entropy(const map<string, int>& counter) const{
  double s = 0.0, p;
  const int T = counter.at("total");
  if (T == 0)
    return 0.0;

  for (map<string, int>::const_iterator it = counter.begin(); it != counter.end(); ++it){
    p = it->second / (1.0 * T);
    if (p != 0.0)
      s -= p * log2(p);
  }
  return s;
}


string DecisionTree::selectBestAttribute(const vector<Example>& examples, const vector<string>& attributes) const{
  const int TAM = attributes.size();
  const int N = examples.size();
  vector<double> gains(TAM);

  map<string, int> counter;
  counter = countClasses(examples);
  double training_entropy = entropy(counter);

  for (int i=0; i < TAM; i++){
    string attribute = attributes[i];
    gains[i] = training_entropy;
    vector<string> values = this->possible_values.at(attribute);
    for (vector<string>::iterator it = values.begin(); it != values.end(); ++it){
      counter = countExamples(examples, attribute, *it);
      gains[i] -= (counter["total"] / (1.0*N))  * entropy(counter);
    }
  }

  return attributes[argmax(gains)];
}


vector<Example> DecisionTree::subsetOfExamples(const vector<Example>& examples, const string & attribute, const string & value) const{
  vector<Example> subset;
  for (int i = 0; i < examples.size() ; i++){
    if ( examples[i].at(attribute) == value )
      subset.push_back(examples[i]);
  }
  return subset;
}


void DecisionTree::ID3(const vector<Example>& examples){
  root = new Node(NULL);

  if ( this->possible_values.empty() )
    return;

  vector<string> attributes;
  for (map<string,vector<string>>::iterator it= possible_values.begin(); it != possible_values.end(); ++it)
    attributes.push_back(it->first);

  ID3(examples, attributes, root);
}


void DecisionTree::ID3(const vector<Example>& examples, vector<string> attributes, Node*& current_node){

  if (sameClass(examples)){
    current_node->setLabel(examples[0].getTarget());
    current_node->markAsLeaf();
    return;
  }

  if (attributes.empty()){
    current_node->setLabel(mostCommonTarget(examples));
    current_node->markAsLeaf();
    return;
  }

  string best_attribute = selectBestAttribute(examples, attributes);
  current_node->setLabel(best_attribute);
  attributes.erase(find(attributes.begin(), attributes.end(), best_attribute));
  vector<string> values = this->possible_values[best_attribute];

  for (vector<string>::iterator it = values.begin(); it != values.end(); ++it){
    Node* node = new Node(current_node);
    current_node->addChild(node, *it);
    vector<Example> subset_of_examples = subsetOfExamples(examples, best_attribute, *it);
    current_node->setInstancesOfBranch(*it, subset_of_examples.size());

    if (subset_of_examples.empty()){
      node->setLabel(mostCommonTarget(examples));
      node->markAsLeaf();
    }
    else
      ID3(subset_of_examples, attributes, node);
  }
}

string DecisionTree::query(const Example& example) const{
  Node* node = this->root;

  while (!node->isLeaf()){
    vector<string> names = node->getBranchesNames();
    vector<string>::iterator it = find(names.begin(), names.end(), example.at(node->getLabel()));
    if (it != names.end())
      node = node->child(*it);
    else
      node = node->child(node->getChildWithMoreInstances());
  }

  return node->getLabel();
}


vector<bool> DecisionTree::predict(const vector<Example>& examples){
  const int N = examples.size();
  vector<bool> predictions = vector<bool>(N);
  for (int i=0; i < N; i++)
    predictions[i] = examples[i].getTarget() == query(examples[i]);

  return predictions;
}


double DecisionTree::accuraccy(const vector<Example>& examples){
  vector<bool> predictions = predict(examples);
  for (auto it = predictions.begin(); it != predictions.end(); ++it){
    cout << *it << " ";
  }
  cout << endl;
  int well_classified = count(predictions.begin(), predictions.end(), true);
  return well_classified / (1.0 * examples.size());
}

/*  ______________________________________________________________________________ */
/* |                                                                             | */
/* |                                NODE CLASS                                   | */
/* |_____________________________________________________________________________| */

Node::Node(Node* parent){
  this->parent = parent;
}

void Node::setLabel(const string & label){
  this->label = label;
}

string Node::getLabel() const{
  return this->label;
}

vector<string> Node::getBranchesNames() const{
  vector<string> names;
  for(map<string,pair<Node*,int>>::const_iterator it = branches.begin(); it != branches.end(); ++it){
    names.push_back(it->first);
  }
  return names;
}

Node* Node::child(const string& name){
  return this->branches.at(name).first;
}

void Node::addChild(Node*& node, string attribute_value){
  this->branches[attribute_value] = make_pair(node,0);
}

void Node::setInstancesOfBranch(string attribute_value, int n){
  this->branches[attribute_value].second = n;
}

string Node::getChildWithMoreInstances(){
  int max = -1;
  string name = "";

  for (map<string,pair<Node*,int>>::const_iterator it = branches.begin(); it != branches.end(); ++it){
    if ( it->second.second > max){
      max = it->second.second;
      name = it->first;
    }
  }
  return name;
}

void Node::markAsLeaf(){
  this->is_leaf = true;
}

bool Node::isLeaf() const{
  return is_leaf;
}

map<string,pair<Node*,int>> Node::getBranches() const{
  return branches;
}


/*  ______________________________________________________________________________ */
/* |                                                                             | */
/* |                               EXAMPLE CLASS                                 | */
/* |_____________________________________________________________________________| */

Example::Example(const vector<string>& names, const vector<string>& values, const string& target){
  this->target = target;
  for (int i=0; i < names.size(); i++)
    this->attributes[names[i]] = values[i];
}

Example::Example(const vector<string>& names, const vector<string>& values){
  for (int i=0; i < names.size(); i++)
    this->attributes[names[i]] = values[i];
}

string Example::at(const string& name) const {
  for (map<string,string>::const_iterator it = attributes.begin(); it != attributes.end(); ++it){
    if (it->first == name)
      return it->second;
  }
  return "";
}

string Example::getTarget() const{
  return target;
};


/*  ______________________________________________________________________________ */
/* |                                                                             | */
/* |                          ARFF READER CLASS                                  | */
/* |_____________________________________________________________________________| */

pair<map<string,vector<string>>, vector<Example>> ARRF_Reader::readFile(const string& path){
  ifstream f(path);
	assert(!f.fail());
  streampos pos;
  string str, attr_name, value;
  map<string,vector<string>> possible_values;
  vector<string> attributes;
  vector<Example> examples;

  // @relation
  getline(f,str,'@');
  getline(f,str,' ');
  getline(f,str,'\n');

  // @attribute
  getline(f,str,'@');
  getline(f,str,' ');

  while (str == "attribute"){
    vector<string> attr_values;
    getline(f,attr_name,' ');
    getline(f,str,'{');
    getline(f,str,'}');
    str += ",}";

    stringstream line(str);
  	while ( line.peek() != '}' ){
  		getline(line, value,',');
      if (value.back() == '\r')
        value.pop_back();
  		attr_values.push_back(value);
  	}

    possible_values[attr_name] = attr_values;
    attributes.push_back(attr_name);

    getline(f,str,'@');
    pos = f.tellg();
    getline(f,str,' ');
  }

  f.seekg(pos);
  possible_values.erase(attr_name);
  attributes.pop_back();

  // @data
  getline(f,str,'\n');
  getline(f,str,'\n');

  while ( !f.eof() ){
    stringstream line(str);

    vector<string> data;
    string target;

    for (int i = 0; i < attributes.size(); i++){
      getline(line, value, ',');
      if (value.back() == '\r')
        value.pop_back();
      data.push_back(value);
    }
    getline(line, target, '\n');
    if (target.back() == '\r')
      target.pop_back();
    examples.push_back(Example(attributes, data, target));
    getline(f,str,'\n');
  }

  f.close();
  return make_pair(possible_values, examples);
}
