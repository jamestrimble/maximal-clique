from collections import deque
import random
import sys
import time

class Graph(object):
    def __init__(self, adjmat):
        self.n = len(adjmat)
        self.adjmat = adjmat
        self.degree = [sum(row) for row in self.adjmat]

if __name__ == "__main__":
    n = int(sys.argv[1])
    p = float(sys.argv[2])

    adjmat = [[False] * n for _ in range(n)]
    for i in range(n-1):
        for j in range(i+1, n):
            if random.random() < p:
                adjmat[i][j] = adjmat[j][i] = True

    g = Graph(adjmat)

    num_edges = sum(sum(row) for row in g.adjmat)/2
    print("{}".format(g.n))
    print("{}".format(num_edges * 2))
    for i in range(g.n-1):
        for j in range(i+1, g.n):
            if g.adjmat[i][j]:
                print("{},{}".format(i, j))
                print("{},{}".format(j, i))
