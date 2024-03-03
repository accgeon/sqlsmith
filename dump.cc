#include <string>
#include <sstream>
#include <format>
#include <typeinfo>

#pragma region custom headers
#include "config.h"
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


std::string graph_dumper::id(struct prod* p)
{
  ostringstream os;
  os << this->type(p) << "_" << p;
  return os.str();
}

void graph_dumper::head() {
    _os << "digraph ast {" << std::endl;
    _os << format("  label=\"{} AST\"", PACKAGE_NAME) << endl;
    _os << "  labelloc=\"t\"" << std::endl;
    _os << "  fontname=\"Noto Sans Mono\" fontsize=16" << std::endl;
    _os << "  colorscheme=\"ylorrd9\"" << std::endl;
    _os << "  node [fontname=\"Menlo\" fontsize=10 shape=record style=filled fillcolor=\"/oranges9/1\"]" << std::endl;
    _os << "  edge [color=\"/blues9/8\"]" << std::endl;
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
    else if (prodClassName == "bool_expr") { // p is subclass of bool_expr
        nodeColor = "/blgr9/3";
    }
    _os << format(
        "{} [label='{{{}|<scope>scope: {:x}|retries: {}}}'{}]",
        this->id(p), this->type(p), (uintptr_t)p->scope, p->retries,
        (nodeColor.empty() ? "" : " fillcolor='"+nodeColor+"'")) << endl;
    // scope node
    if (p->scope && visited_scopes.count(p->scope) == 0) { // if there's no scope in visited_scopes inserted previously
        visited_scopes.insert(p->scope); // insert it
        _os << format("'{:x}' [label='scope:\\n'{:x}' shape=note]", (uintptr_t)p->scope, (uintptr_t)p->scope) << endl; // scope node
    }
    // edge to scope node: composition
    _os << format("'{}':scope -> '{:x}' [arrowtail=odiamond, dir=back]", this->id(p), (uintptr_t)p->scope) << endl;
    // edge to parent node
    if (p->pprod) {
        _os << format("'{}' -> '{}'", this->id(p->pprod), this->id(p)) << endl;
    }
}


void ast_logger::generated(prod &query)
{
    ostringstream ss;
    ss << "ast-" << put_time(localtime(&now), "%Y%m%d%H%M%S") << "-" << queries << ".dot" << flush;

    ofstream os(ss.str());
    graph_dumper visitor(os);
    visitor.head();
    query.accept(&visitor);
    visitor.tail();
    queries++;
}
