/// @file
/// @brief Dump syntax trees as GraphML
#pragma once

#include <chrono>
#include <ctime>
#include <map>
#include <string>
#include <sstream>

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

class Subgraph {
public:
    std::string id;
    std::string label;
    std::string color;
    std::string fillcolor;

    // sstream for subgraph
    std::ostringstream ss;

    Subgraph() : _scope(nullptr) {}

    Subgraph(std::string id, struct scope* scope,
            std::string label = "", std::string color = "", std::string fillcolor = "")
            : id(id), label(label), color(color), fillcolor(fillcolor),
            _scope(scope) {
        if (label.empty()) label = "scope: "+id;
        if (color.empty()) color = "/blues9/4";
        if (fillcolor.empty()) fillcolor = "/blues9/1";
    }

    void head();
    void print(struct prod* p);
    void tail() { ss << "}" << std::endl; }

private:
    struct scope* _scope;
};

class graph_dumper : public prod_visitor {
public:
    graph_dumper(std::ostream &out)
        : _os(out),
        _headAdded(false), _tailAdded(false) {}
    virtual ~graph_dumper() {}

    virtual std::string id(struct prod* p);
    virtual std::string type(struct prod* p) { return pretty_type(p); }

    virtual void visit(struct prod* p);

    virtual void head();
    virtual void print(struct prod* p);
    virtual void tail();

protected:
    std::ostream &_os;
    std::map<std::string, Subgraph*> _visitedScopes;

    bool _headAdded;
    bool _tailAdded;
};

struct ast_logger : logger {
    std::chrono::system_clock::time_point now;
    std::time_t start_time;
    int queries = 0;

    ast_logger() : now(std::chrono::system_clock::now()), start_time(std::chrono::system_clock::to_time_t(now)) {}
    virtual void generated(prod &query);
};
