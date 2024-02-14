/// @file
/// @brief Dump syntax trees as GraphML
#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <set>
#include <string>

#include "prod.hh"
#include "relmodel.hh"
#include "log.hh"
#include "util.hh"

struct graphml_dumper : prod_visitor {
  std::ostream &o;
  virtual void visit(struct prod *p);
  graphml_dumper(std::ostream &out);
  std::string id(prod *p);
  std::string type(struct prod *p);
  virtual ~graphml_dumper();
};

// template<typename K>
// class ScopeLess {
// public:
//   bool operator()(const K* lhs, const K* rhs) {
//     return lhs < rhs; // the address of lhs is less than that of rhs
//   }
// };

class graph_dumper : public prod_visitor {
public:
    graph_dumper(std::ostream &out) : _os(out) {}
    virtual ~graph_dumper() {}

    virtual std::string id(struct prod* p);
    virtual std::string type(struct prod* p) { return pretty_type(p); }

    virtual void visit(struct prod* p) { this->print(p); }

    virtual void head() {
        _os << "digraph ast {" << std::endl;
        _os << "  rankdir=\"BT\"" << std::endl;
        _os << "  node [fontname=\"Monaco\" fontsize=10 shape=record style=filled fillcolor=\"/oranges9/1\"]" << std::endl;
    }
    virtual void print(struct prod* p);
    virtual void tail() { _os << "}" << std::endl;}

protected:
    std::ostream &_os;
    std::set<scope*> visited_scopes;
};

struct ast_logger : logger {
  std::time_t now;
  int queries = 0;

  ast_logger() : now(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}
  virtual void generated(prod &query);
};
