import sys


def choose_pivot(P, G):
    pivot = -1
    intersection_size = -1
    for u in P:
        sz = len(P & G[u])
        if sz > intersection_size:
            pivot = u
            intersection_size = sz
    return pivot


def bk(R, P, X, G):
    if not P and not X:
        return 1
    if not P:
        return 0
    P_ = set(P)
    X_ = set(X)
    result = 0
    u = choose_pivot(P, G)
    for v in P - G[u]:
        R.append(v)
        result += bk(R, P_ & G[v], X_ & G[v], G)
        R.remove(v)
        P_.remove(v)
        X_.add(v)
    return result


lines = []
for line in sys.stdin.readlines():
    lines.append([int(token) for token in line.strip().split(",")])
n = lines[0][0]
edges = lines[2:]
if len(edges) != lines[1][0]:
    raise Exception("Unexpected number of edges")
G = [set() for v in range(n)]
for v, w in edges:
    G[v].add(w)
print(bk([], set(range(len(G))), set(), G))
