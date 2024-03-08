// -*- C++ -*-
/// @file
/// @brief Dump syntax trees as GraphViz dot (and/or GraphML)

#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#pragma region custom headers
#include "config.h"
#include "expr.hh"
#include "dump.hh"
#include "util.hh"
#pragma endregion

using namespace std;

std::string graphml_dumper::id(struct prod *p)
{
  ostringstream os;
  os << pretty_type(p) << "_" << p;
  return os.str();
}

graphml_dumper::graphml_dumper(ostream &out)
  : o(out)
{
  o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl <<
    "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" " <<
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " <<
    "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns " <<
    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">" << endl;

  o << "<key id=\"retries\" for=\"node\" "
            "attr.name=\"retries\" attr.type=\"double\" />" << endl;
  o << "<key id=\"label\" for=\"node\" "
            "attr.name=\"label\" attr.type=\"string\" />" << endl;
  o << "<key id=\"scope\" for=\"node\" "
            "attr.name=\"scope\" attr.type=\"string\" />" << endl;

  o << "<graph id=\"ast\" edgedefault=\"directed\">" << endl;
  
}

void graphml_dumper::visit(struct prod *p)
{
  o << "<node id=\"" << id(p) <<  "\">";
  o << "<data key=\"retries\">" << p->retries << "</data>";
  o << "<data key=\"label\">" << pretty_type(p) << "</data>";
  o << "<data key=\"scope\">" << p->scope << "</data>";
  o << "</node>" << endl;
  if (p->pprod) {
    o << "<edge source=\"" << id(p) << "\" target=\"" << id(p->pprod) << "\"/>";
  }
  o << endl;
}

graphml_dumper::~graphml_dumper()
{
  o << "</graph></graphml>" << endl;
}

////////////////////////////////////////
// Visitor to dump AST as graphviz
//

void Subgraph::head() {
    ss << format("  subgraph \"cluster_{}\" {{", this->id) << endl;
    ss << format("    label=\"{}\"", this->label) << endl;
    ss <<        "    fontname=\"Monaco\" fontsize=12" << endl;
    ss <<        "    labeljust=\"l\" rank=source" << endl;
    ss <<        "    color=\"/blues9/4\"" << endl;
    ss <<        "    style=\"filled\" fillcolor=\"/blues9/1\"" << endl;

}

string graph_dumper::id(struct prod* p)
{
  return format("{}_{:#018x}", this->type(p), (uintptr_t)p);
}

inline
void graph_dumper::visit(struct prod *p) {
    this->print(p);
}

void graph_dumper::head() {
    _os <<        "digraph ast {" << std::endl;
    _os << format("  label=\"{} AST\"", PACKAGE_NAME) << endl;
    _os <<        "  labelloc=\"t\"" << std::endl;
    _os <<        "  fontname=\"Noto Sans Mono\" fontsize=20" << std::endl;
    _os <<        "  colorscheme=\"ylorrd9\"" << std::endl;
    _os <<        "  node [fontname=\"Noto Sans Mono\" fontsize=10 shape=record style=filled fillcolor=\"/oranges9/1\"]" << std::endl;
    _os <<        "  edge [color=\"/blues9/8\"]" << std::endl;
}

void graph_dumper::print(struct prod* p)
{
    // node
    string prodClassName = this->type(p);
    string nodeColor;
    if (prodClassName == "query_spec") {
        nodeColor = "goldenrod1";
    }
    else if (prodClassName == "from_clause") {
        nodeColor = "/purples9/3";
    }
    else if (prodClassName == "select_list") {
        nodeColor = "/orrd9/3";
    }
    else if (dynamic_cast<bool_expr*>(p)) { // p is subclass of bool_expr
        nodeColor = "/gnbu9/3";
    }

    ostream* pos = &_os;
    // scope node
    auto scopeId = Subgraph::makeId(p->scope);
    Subgraph* subgraph = nullptr;
    if (p->scope) {
        subgraph = _visitedScopes[scopeId];
        if (!subgraph) { // if there's no scope in _visitedScopes inserted previously
            // start subgraph block
            subgraph = new Subgraph(p->scope);
            _visitedScopes[subgraph->id] = subgraph; // insert it
            subgraph->head();
            subgraph->ss << format(
            "    \"{0:#018x}\" [label=\"scope:\\n{0:#018x}\" shape=note fillcolor=\"/ylorbr9/2\"]\n", (uintptr_t)p->scope) << endl; // scope node
        }
        pos = &(subgraph->ss);
    }
    (*pos) << format(
            "    \"{}\" [label=\"{{{}|<scope>scope: {}|retries: {}}}\" {}]",
            this->id(p), this->type(p), subgraph->id, p->retries,
            (nodeColor.empty() ? "" : "fillcolor=\""+nodeColor+"\"")) << endl;

    // edge to parent node
    if (p->pprod) {
        (*pos) << format("    \"{}\" -> \"{}\"", this->id(p->pprod), this->id(p)) << endl;
    }
}

void graph_dumper::tail() {
    // close subgraph clusters
    for (auto& [id, subgraph] : _visitedScopes) {
        subgraph->tail();
        _os << subgraph->ss.str() << endl;
    }
    // close main graph
    _os << "}" << endl;
}

////////////////////////////////////////
// dump AST for the query
//
void ast_logger::generated(prod &query)
{
    string filename = makeFilename();
    ostringstream ss;
    graph_dumper* pvisitor = new graph_dumper(ss);
    pvisitor->head();
    query.accept(pvisitor);
    pvisitor->tail();
    queries++;

    ofstream os(filename);
    os << ss.str() << flush;
}
