// -*- C++ -*-
/// @file
/// @brief Dump syntax trees as GraphViz dot (and/or GraphML)

#pragma once

#include <chrono>
#include <ctime>
#include <format>
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
    struct scope* _scope;
public:
    std::string id;
    std::string label;
    std::string color;
    std::string fillcolor;

    // sstream for subgraph
    std::ostringstream ss;

    const char* pdefaultColor = "/blues9/4";
    const char* pdefaultFillcolor = "/blues9/1";

    Subgraph(struct scope* scope = nullptr, std::string color = "", std::string fillcolor = "") {
        this->setScope(scope);
        this->initColors(color, fillcolor);
    }

    static std::string makeId(struct scope* scope) { return std::format("{:#018x}", (uintptr_t)scope); }

    void setScope(struct scope* scope) {
        _scope = scope;
        id = this->makeId(_scope);
        label = "scope: "+id;
    }
    void initColors(std::string color = "", std::string fillcolor = "") {
        color = color.empty() ? pdefaultColor : color;
        fillcolor = fillcolor.empty() ? pdefaultFillcolor : fillcolor;
    }

    void head();
    void print(struct prod* p);
    void tail() { ss << "  }" << std::endl; }

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

    ast_logger()
          : now(std::chrono::system_clock::now()), 
            start_time(std::chrono::system_clock::to_time_t(now)) {}
    virtual void generated(prod &query);
    std::string makeFilename() {
        tm *t = localtime(&start_time);
        return format("ast-{}{:02}{:02}{:02}{:02}{:02}-{}.dot",
                1900 + t->tm_year, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec, queries);
    }
};
