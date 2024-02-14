#include <string>
#include <sstream>

#include "dump.hh"
#include "util.hh"

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

void graph_dumper::print(struct prod* p)
{
    // node
    _os << this->id(p) << " [label=\"{" << this->type(p)
        << "|" << "<scope>scope: " << p->scope
        << "|" << "retries: " << p->retries
        << "}\"]" << endl;
    // scope node
    if (p->scope && visited_scopes.count(p->scope) == 0) { // if there's no scope in visited_scopes inserted previously
        visited_scopes.insert(p->scope); // insert it
        _os << "\"" << p->scope << "\" [label=\"scope:\\n" << p->scope << "\" shape=note]" << endl; // scope node
    }
    // edge to scope node: composition
    _os << this->id(p) << ":scope" << " -> \"" << p->scope << "\" [arrowtail=odiamond, dir=back]" << endl;
    // edge to parent node
    if (p->pprod) {
        _os << this->id(p) << " -> " << this->id(p->pprod) << endl;
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
