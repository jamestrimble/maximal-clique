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
    QuickSet(int n) : data(n) {}
    
    auto add(int v) -> void {
        data[v] = k;
    }

    auto remove(int v) -> void {
        data[v] = 0;
    }

    auto has(int v) -> bool {
        return data[v] == k;
    }

    auto clear() -> void {
        ++k;
        if (k == 2000000000) {
            k = 1;
            for (int & val : data) {
                val = 0;
            }
        }
    }
};

auto intersection(vector<int> & S, QuickSet & S_QSet, const vector<int> & T, const vector<bool> & T_bools,
        vector<int> & result) -> void
{
    result.clear();
    if (S.size() < T.size()) {
        for (int v : S) {
            if (T_bools[v]) {
                result.push_back(v);
            }
        }
    } else {
        for (int v : T) {
            if (S_QSet.has(v)) {
                result.push_back(v);
            }
        }
    }
}

auto intersection(vector<int> & S, QuickSet & S_QSet, const vector<int> & T, const vector<bool> & T_bools)
        -> vector<int>
{
    vector<int> result;
    if (S.size() < T.size()) {
        for (int v : S) {
            if (T_bools[v]) {
                result.push_back(v);
            }
        }
    } else {
        for (int v : T) {
            if (S_QSet.has(v)) {
                result.push_back(v);
            }
        }
    }
    return result;
}

auto intersection_size(vector<int> & S, QuickSet & S_QSet, const vector<int> & T, const vector<bool> & T_bools)
        -> int
{
    int result = 0;
    if (S.size() < T.size()) {
        for (int v : S) {
            result += T_bools[v];
        }
    } else {
        for (int v : T) {
            result += S_QSet.has(v);
        }
    }
    return result;
}

auto choose_pivot(vector<int> & P,
        QuickSet & set_P,
        const vector<vector<bool>> & adjmat,
        const vector<vector<int>> & adjlists) -> int
{
    int pivot = -1;
    int best_intersection_sz = -1;
    for (int u : P) {
        int sz = intersection_size(P, set_P, adjlists[u], adjmat[u]);
        if (sz > best_intersection_sz) {
            pivot = u;
            best_intersection_sz = sz;
        }
    }
    return pivot;
}

auto bk(vector<int> & R,
        vector<int> & P,
        vector<int> & X,
        const vector<vector<bool>> & adjmat,
        const vector<vector<int>> & adjlists,
        vector<std::unique_ptr<QuickSet>> & P_sets,
        vector<std::unique_ptr<QuickSet>> & X_sets,
        vector<std::unique_ptr<vector<int>>> & new_Ps,
        vector<std::unique_ptr<vector<int>>> & new_Xs) -> int
{
    if (P.empty()) {
        return X.empty() ? 1 : 0;
    }
    auto P_ = P;
    auto X_ = X;
    if (P_sets.size() <= R.size()) {
        P_sets.push_back(std::make_unique<QuickSet>(int(adjmat.size())));
        X_sets.push_back(std::make_unique<QuickSet>(int(adjmat.size())));
        new_Ps.push_back(std::make_unique<vector<int>>());
        new_Xs.push_back(std::make_unique<vector<int>>());
    }
    QuickSet & set_P_ = *P_sets[R.size()];
    QuickSet & set_X_ = *X_sets[R.size()];
    set_P_.clear();
    set_X_.clear();
    for (int v : P_) {
        set_P_.add(v);
    }
    for (int v : X_) {
        set_X_.add(v);
    }
    int u = choose_pivot(P, set_P_, adjmat, adjlists);
    int result = 0;
    for (int i=P.size(); i--; ) {
        int v = P[i];
        if (adjmat[u][v]) {
            continue;
        }
        vector<int> & new_P = *new_Ps[R.size()];
        vector<int> & new_X = *new_Xs[R.size()];
        intersection(P_, set_P_, adjlists[v], adjmat[v], new_P);
        intersection(X_, set_X_, adjlists[v], adjmat[v], new_X);
        R.push_back(v);
        result += bk(R, new_P, new_X, adjmat, adjlists, P_sets, X_sets, new_Ps, new_Xs);
        R.pop_back();

        // remove v from P_
        P_[i] = P_.back();
        P_.pop_back();
        set_P_.remove(v);

        X_.push_back(v);
        set_X_.add(v);
    }
    return result;
}

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

    vector<int> R;
    vector<int> P;
    P.reserve(n);
    for (int i=0; i<n; i++) {
        P.push_back(i);
    }
    vector<int> X;
    vector<std::unique_ptr<QuickSet>> P_sets;
    vector<std::unique_ptr<QuickSet>> X_sets;
    vector<std::unique_ptr<vector<int>>> new_Ps;
    vector<std::unique_ptr<vector<int>>> new_Xs;
    int result = bk(R, P, X, adjmat, adjlists, P_sets, X_sets, new_Ps, new_Xs);
    std::cout << result << std::endl;
}
