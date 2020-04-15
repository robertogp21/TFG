#ifndef __DecisionTree_h__
#define __DecisionTree_h__

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <string>
#include <map>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <limits>
#include <sstream>
using namespace std;

class Node;
class Example;

class DecisionTree {
  private:
    Node* root;
    map<string,vector<string>> possible_values;
    void destroy(Node*& node);
    bool sameClass(const vector<Example>& examples) const;
    string mostCommonTarget(const vector<Example>& examples) const;
    map<string, int> countClasses(const vector<Example>& examples) const;
    map<string, int> countExamples(const vector<Example>& examples, const string& attribute, const string& value) const;
    double entropy(const map<string, int>& counter) const;
    string selectBestAttribute(const vector<Example>& examples, const vector<string>& attributes) const;
    vector<Example> subsetOfExamples(const vector<Example>& examples, const string & attribute, const string & value) const;
    void ID3(const vector<Example>& examples, vector<string> attributes, Node*& current_node);

  public:
    DecisionTree();
    ~DecisionTree();
    void setPossibleValues(const map<string,vector<string>>& possible_values);
    void ID3(const vector<Example>& examples);
    string query(const Example& example) const;
    vector<bool> predict(const vector<Example>& examples);
    double accuraccy(const vector<Example>& examples);
};

class Node {
  private:
    Node* parent;
    string label;
    map<string,pair<Node*,int>> branches;
    bool is_leaf;

  public:
    Node(Node* parent);
    void setLabel(const string & label);
    string getLabel() const;
    vector<string> getBranchesNames() const;
    Node* child(const string& name);
    void addChild(Node*& node, string attribute_value);
    void setInstancesOfBranch(string attribute_value, int n);
    string getChildWithMoreInstances();
    void markAsLeaf();
    bool isLeaf() const;
    map<string,pair<Node*,int>> getBranches() const;
};

class Example {
  private:
    map<string,string> attributes;
    string target;

  public:
    Example(const vector<string>& names, const vector<string>& values, const string& target);
    Example(const vector<string>& names, const vector<string>& values);
    string at(const string& name) const;
    string getTarget() const;
};

class ARRF_Reader {
  public:
    static pair<map<string,vector<string>>, vector<Example>> readFile(const string& path);
};

#endif
