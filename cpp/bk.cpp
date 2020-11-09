#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::vector;

class QuickSet
{
    vector<int> data;
    int k = 1;

public:
    vector<int> vec;  // the elements as a list

    QuickSet(int n) : data(n) {}

    auto empty() -> bool {
        return vec.empty();
    }

    auto size() -> unsigned {
        return vec.size();
    }

    auto add(int v) -> void {
        data[v] = k;
        vec.push_back(v);
    }

    auto remove_at(int position) -> void {
        int v = vec[position];
        data[v] = 0;
        vec[position] = vec.back();
        vec.pop_back();
    }

    auto has(int v) -> bool {
        return data[v] == k;
    }

    auto clear() -> void {
        vec.clear();
        ++k;
        if (k == 2000000000) {
            k = 1;
            for (int & val : data) {
                val = 0;
            }
        }
    }
};

auto intersection(QuickSet & S, const vector<int> & T, const vector<bool> & T_bools,
        QuickSet & result) -> void
{
    result.clear();
    if (S.size() < T.size()) {
        for (int v : S.vec) {
            if (T_bools[v]) {
                result.add(v);
            }
        }
    } else {
        for (int v : T) {
            if (S.has(v)) {
                result.add(v);
            }
        }
    }
}

auto intersection_size(QuickSet & S, const vector<int> & T, const vector<bool> & T_bools)
        -> int
{
    int result = 0;
    if (S.size() < T.size()) {
        for (int v : S.vec) {
            result += T_bools[v];
        }
    } else {
        for (int v : T) {
            result += S.has(v);
        }
    }
    return result;
}

class BK
{
    int n;
    const vector<vector<bool>> & adjmat;
    const vector<vector<int>> & adjlists;
    vector<std::unique_ptr<QuickSet>> P_sets;
    vector<std::unique_ptr<QuickSet>> X_sets;
    long step_count = 0;

    auto choose_pivot(QuickSet & set_P,
            QuickSet & set_X) -> int
    {
        int pivot = -1;
        int best_intersection_sz = -1;
        for (int u : set_X.vec) {
            int sz = intersection_size(set_P, adjlists[u], adjmat[u]);
            if (sz > best_intersection_sz) {
                pivot = u;
                best_intersection_sz = sz;
            }
        }
        for (int u : set_P.vec) {
            int sz = intersection_size(set_P, adjlists[u], adjmat[u]);
            if (sz > best_intersection_sz) {
                pivot = u;
                best_intersection_sz = sz;
            }
        }
        return pivot;
    }

    auto bk(vector<int> & R,
            QuickSet & P,
            QuickSet & X) -> long
    {
        ++step_count;
        if (P.empty()) {
            return X.empty() ? 1 : 0;
        }
        QuickSet & new_P = get_preallocated_set(P_sets, R.size());
        QuickSet & new_X = get_preallocated_set(X_sets, R.size());
        int u = choose_pivot(P, X);
        long result = 0;
        for (int i=P.size(); i--; ) {
            int v = P.vec[i];
            if (adjmat[u][v]) {
                continue;
            }
            intersection(P, adjlists[v], adjmat[v], new_P);
            intersection(X, adjlists[v], adjmat[v], new_X);
            R.push_back(v);
            result += bk(R, new_P, new_X);
            R.pop_back();

            // remove v from P
            P.remove_at(i);

            X.add(v);
        }
        return result;
    }

public:
    BK(int n, const vector<vector<bool>> & adjmat, const vector<vector<int>> & adjlists)
            : n(n), adjmat(adjmat), adjlists(adjlists)
    {
    }

    auto get_preallocated_set(vector<std::unique_ptr<QuickSet>> & sets, int search_depth) -> QuickSet &
    {
        if (sets.size() <= search_depth) {
            sets.push_back(std::make_unique<QuickSet>(n));
        }
        return *sets[search_depth];
    }

    auto count_maximal_cliques() -> long
    {
        vector<int> R;
        QuickSet P(n);
        for (int i=0; i<n; i++) {
            P.add(i);
        }
        QuickSet X(n);
        return bk(R, P, X);
    }

    auto get_step_count() -> long
    {
        return step_count;
    }
};

auto main(int argc, char **argv) -> int
{
    int n = 0;
    int m2 = 0;

    vector<vector<bool>> adjmat;
    vector<vector<int>> adjlists;

    int line_num = 0;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line_num > 1) {
            int comma_pos = line.find(",");
            int v = std::stoi(line.substr(0, comma_pos));
            int w = std::stoi(line.substr(comma_pos + 1, line.length()));
            adjmat[v][w] = true;
            adjlists[v].push_back(w);
        } else if (line_num == 0) {
            n = std::stoi(line);
            adjmat = vector<vector<bool>>(n, vector<bool>(n));
            adjlists = vector<vector<int>>(n);
        } else {
            m2 = std::stoi(line);
        }
        ++line_num;
    }

    if (line_num - 2 != m2) {
        std::cout << "Warning: " << (line_num - 2) << " edges read; " << m2 << " expected." << std::endl;
    }

    BK bk(n, adjmat, adjlists);
    long result = bk.count_maximal_cliques();
    std::cout << bk.get_step_count() << std::endl;
    std::cout << result << std::endl;
}
