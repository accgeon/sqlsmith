#!/usr/bin/env python

#
# graphml to graphviz
#

import graphviz as gv
import networkx as nx

dig = nx.read_graphml('sqlsmith-0.xml')
gvgraph = gv.Digraph('ast', node_attr={'shape': 'record', 'rankdir': 'TB', 'fontname': 'monaco', 'fontsize': '10'})

for key in dig.nodes:
    n = dig.nodes[key]
    gvgraph.node(key, f"{{{n['label']}|scope: {n['scope']}|retries: {n['retries']}}}")
    gvgraph.node(n['scope'], n['scope'], shape='note')
    gvgraph.edge(key, n['scope'], dir='none')

for ed in dig.edges:
    gvgraph.edge(ed[0], ed[1])

gvgraph.render('s-0.dot')
