#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::vector;

/** A set of integers in the range [0,n) with fast operations for insertion,
 *  deletion, clearing, and iterating in an unspecified order.
 */
class QuickSet
{
    vector<int> is_member;  //!< v is in the set if and only iff if
                            //!< is_member[v] == k
    int k = 1;
    vector<int> vec;        //!< a list of the set's elements
    int vec_len = 0;        //!< The number of elements in vec (vec is used
                            //!< like a C-style array for speed)
    vector<int> position_in_vec;  //!< if v is in the set, then
                                  //!< vec[position_in_vec[v]] == v

public:
    QuickSet(int n) : is_member(n), vec(n), position_in_vec(n) {}

    auto begin() {
        return vec.cbegin();
    }

    auto end() {
        return vec.cbegin() + vec_len;
    }

    auto empty() -> bool {
        return vec_len == 0;
    }

    auto size() -> int {
        return vec_len;
    }

    auto add(int v) -> void {
        is_member[v] = k;
        position_in_vec[v] = vec_len;
        vec[vec_len++] = v;
    }

    auto remove(int v) -> void {
        int position = position_in_vec[v];
        is_member[v] = 0;
        int w = vec[vec_len - 1];
        position_in_vec[w] = position;
        vec[position] = w;
        --vec_len;
    }

    auto has(int v) -> bool {
        return is_member[v] == k;
    }

    auto clear() -> void {
        vec_len = 0;
        ++k;
        if (k == 2000000000) {
            k = 1;
            for (int & val : is_member) {
                val = 0;
            }
        }
    }
};

auto intersection(QuickSet & S, const vector<int> & T, const vector<char> & T_bools,
        QuickSet & result) -> void
{
    result.clear();
    if (S.size() < T.size()) {
        for (int v : S) {
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

auto intersection_size(QuickSet & S, const vector<int> & T, const vector<char> & T_bools)
        -> int
{
    int result = 0;
    if (S.size() < T.size()) {
        for (int v : S) {
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
    const vector<vector<char>> & adjmat;
    const vector<vector<int>> & adjlists;
    vector<std::unique_ptr<QuickSet>> P_sets;
    vector<std::unique_ptr<QuickSet>> X_sets;
    vector<std::unique_ptr<vector<int>>> branching_lists;
    long step_count = 0;

    auto choose_pivot(QuickSet & set_P,
            QuickSet & set_X) -> int
    {
        int pivot = -1;
        int best_intersection_sz = -1;
        QuickSet * sets[] = {&set_X, &set_P};
        for (int i=0; i<2; i++) {
            QuickSet & set = *sets[i];
            for (int u : set) {
                int sz = intersection_size(set_P, adjlists[u], adjmat[u]);
                if (sz > best_intersection_sz) {
                    if (sz >= set_P.size() - 1) {
                        // Some of Naud\'{e}'s optimisations
                        return sz == set_P.size() ? -1 : u;
                    }
                    pivot = u;
                    best_intersection_sz = sz;
                }
            }
        }
        return pivot;
    }

    template<typename T>
    auto get_preallocated_item(vector<std::unique_ptr<T>> & items, int search_depth) -> T &
    {
        if (items.size() <= search_depth) {
            items.push_back(std::make_unique<T>(n));
        }
        return *items[search_depth];
    }

    auto bk(vector<int> & R,
            QuickSet & P,
            QuickSet & X) -> long
    {
        ++step_count;
        if (P.empty()) {
            return X.empty() ? 1 : 0;
        }
        QuickSet & new_P = get_preallocated_item(P_sets, R.size());
        QuickSet & new_X = get_preallocated_item(X_sets, R.size());
        int u = choose_pivot(P, X);
        if (u == -1) {
            return 0;
        }
        long result = 0;
        auto & branching_vertices = get_preallocated_item(branching_lists, R.size());
        branching_vertices.clear();
        for (int v : P) {
            if (!adjmat[u][v]) {
                branching_vertices.push_back(v);
            }
        }
#ifndef WITHOUT_SORTING
        std::sort(branching_vertices.begin(), branching_vertices.end());
#endif
        for (int v : branching_vertices) {
            intersection(P, adjlists[v], adjmat[v], new_P);
            intersection(X, adjlists[v], adjmat[v], new_X);
            R.push_back(v);
            result += bk(R, new_P, new_X);
            R.pop_back();
            P.remove(v);
            X.add(v);
        }
        return result;
    }

public:
    BK(int n, const vector<vector<char>> & adjmat, const vector<vector<int>> & adjlists)
            : n(n), adjmat(adjmat), adjlists(adjlists)
    {
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

    vector<vector<char>> adjmat;
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
            adjmat = vector<vector<char>>(n, vector<char>(n));
            adjlists = vector<vector<int>>(n);
        } else {
            m2 = std::stoi(line);
        }
        ++line_num;
    }

    if (line_num - 2 != m2) {
        std::cout << "Warning: " << (line_num - 2) << " edges read; " << m2 << " expected." << std::endl;
    }

#ifdef WITHOUT_SORTING
    BK bk(n, adjmat, adjlists);
#else
    vector<int> order;         // vertex map from ordered to input graph
    vector<int> order_inv(n);  // vertex map from input to ordered graph
    for (int i=0; i<n; i++) {
        order.push_back(i);
    }
    std::sort(order.begin(), order.end(),
            [&](int v, int w){return adjlists[v].size() < adjlists[w].size();});
    for (int v=0; v<n; v++) {
        order_inv[order[v]] = v;
    }
    auto ordered_adjmat = vector<vector<char>>(n, vector<char>(n));
    auto ordered_adjlists = vector<vector<int>>(n);
    for (int v=0; v<n; v++) {
        for (int w : adjlists[v]) {
            int new_v = order_inv[v];
            int new_w = order_inv[w];
            ordered_adjlists[new_v].push_back(new_w);
            ordered_adjmat[new_v][new_w] = true;
        }
    }
    for (int v=0; v<n; v++) {
        std::sort(ordered_adjlists[v].begin(), ordered_adjlists[v].end());
    }
    BK bk(n, ordered_adjmat, ordered_adjlists);
#endif
    long result = bk.count_maximal_cliques();
    std::cout << bk.get_step_count() << std::endl;
    std::cout << result << std::endl;
}
