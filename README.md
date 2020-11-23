# Algorithms for maximal clique enumeration

This repository implements some variants of the Bron-Kerbosch algorithm.  The C++ versions were inspired by work by San Segundo, Artieda, and Strash (GreedyBB); Tomita; and Naudé.

The file format is based on the [custom format used in quick-cliques](https://github.com/darrenstrash/quick-cliques/blob/7f7d0b7d1534a0fefae8e710a8deb9486f597149/README.md).  For example, a graph with five vertices (numbered from 0 to 4) and a single edge with endpoints 0 and 3 is stored as follows (where the second line is twice the number of edges):

```
5
2
0,3
3,0
```

All programs read from standard input and write the number of maximal cliques found as the final line of output.
