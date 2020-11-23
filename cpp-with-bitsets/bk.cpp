#include <algorithm>
#include <climits>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::vector;
using setword = unsigned long long;
using Bitset = vector<setword>;

#define BYTES_PER_WORD sizeof(setword)
#define BITS_PER_WORD (CHAR_BIT * BYTES_PER_WORD)
#define WORDS_REQUIRED(n) ((n + BITS_PER_WORD - 1) / BITS_PER_WORD)

static bool test_bit(const Bitset & bitset, int bit)
{
    return 0 != (bitset[bit/BITS_PER_WORD] & (1ull << (bit%BITS_PER_WORD)));
}

static void set_bit(Bitset & bitset, int bit)
{
    bitset[bit/BITS_PER_WORD] |= (1ull << (bit%BITS_PER_WORD));
}

static void unset_bit(Bitset & bitset, int bit)
{
    bitset[bit/BITS_PER_WORD] &= ~(1ull << (bit%BITS_PER_WORD));
}

static bool bitset_empty(const Bitset & bitset, int num_words)
{
    for (int i=0; i<num_words; i++)
        if (bitset[i] != 0)
            return false;
    return true;
}

static int bitset_popcount(const Bitset & bitset, int num_words)
{
    int retval = 0;
    for (int i=0; i<num_words; i++)
        retval += __builtin_popcountll(bitset[i]);
    return retval;
}

static int bitset_intersection_popcount(const Bitset & bitset1,
                                        const Bitset & bitset2,
                                        int num_words)
{
    int retval = 0;
    for (int i=0; i<num_words; i++)
        retval += __builtin_popcountll(bitset1[i] & bitset2[i]);
    return retval;
}

static void bitset_intersection(const Bitset & src1,
                                     const Bitset & src2,
                                     Bitset & dst,
                                     int num_words)
{
    for (int i=0; i<num_words; i++)
        dst[i] = src1[i] & src2[i];
}

static void bitset_difference(const Bitset & src1,
                                     const Bitset & src2,
                                     Bitset & dst,
                                     int num_words)
{
    for (int i=0; i<num_words; i++)
        dst[i] = src1[i] & ~src2[i];
}

static void clear_bitset(Bitset & bitset,
                        int num_words)
{
    for (int i=0; i<num_words; i++)
        bitset[i] = 0ull;
}

template<typename F>
static void bitset_foreach(const Bitset & bitset, F f, int numwords)
{
    for (int i=0; i<numwords; i++) {
        setword word = bitset[i];
        while (word) {
            int bit = __builtin_ctzll(word);
            word ^= (1ull << bit);
            int v = i*BITS_PER_WORD + bit;
            f(v);
        }
    }
}

/** A set of integers in the range [0,n) with fast operations for insertion,
 *  deletion, clearing, and iterating in an unspecified order.
 */
struct QuickSet
{
    int num_words;
    Bitset bitset;

public:
    QuickSet(int n) : num_words(WORDS_REQUIRED(n)), bitset(num_words) {}

    auto & get_bitset() {
        return bitset;
    }

    int get_num_words() {
        return num_words;
    }

    auto empty() -> bool {
        return bitset_empty(bitset, num_words);
    }

    auto size() -> int {
        return bitset_popcount(bitset, num_words);
    }

    auto intersection_size(const Bitset & other) -> int {
        return bitset_intersection_popcount(bitset, other, num_words);
    }

    auto add(int v) -> void {
        set_bit(bitset, v);
    }

    auto remove(int v) -> void {
        unset_bit(bitset, v);
    }

    auto has(int v) -> bool {
        return test_bit(bitset, v);
    }

    auto clear() -> void {
        clear_bitset(bitset, num_words);
    }
};

auto intersection(QuickSet & S, const Bitset & T_bools,
        QuickSet & result) -> void
{
    bitset_intersection(S.bitset, T_bools, result.bitset, S.num_words);
}

class BK
{
    int n;
    int num_words;
    const vector<Bitset> & adjmat;
    const vector<vector<int>> & adjlists;
    vector<std::unique_ptr<QuickSet>> P_sets;
    vector<std::unique_ptr<QuickSet>> X_sets;
    vector<std::unique_ptr<Bitset>> branching_bitsets;
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
            Bitset & bitset = set.get_bitset();
            for (int i=0; i<num_words; i++) {
                setword word = bitset[i];
                while (word) {
                    int bit = __builtin_ctzll(word);
                    word ^= (1ull << bit);
                    int u = i*BITS_PER_WORD + bit;
                    int sz = set_P.intersection_size(adjmat[u]);
                    if (sz > best_intersection_sz) {
                        if (sz >= set_P.size() - 1) {
                            // Similar to Naud\'{e}'s optimisations?
                            return sz == set_P.size() ? -1 : u;
                        }
                        pivot = u;
                        best_intersection_sz = sz;
                    }
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
        auto & branching_bitset = get_preallocated_item(branching_bitsets, R.size());
        bitset_difference(P.get_bitset(), adjmat[u], branching_bitset, num_words);
#ifndef WITHOUT_SORTING
        auto & branching_vertices = get_preallocated_item(branching_lists, R.size());
        int branching_vertices_len = 0;
        bitset_foreach(branching_bitset, [&](int v){
            branching_vertices[branching_vertices_len++] = v;
        }, P.get_num_words());
        std::sort(branching_vertices.begin(), branching_vertices.begin() + branching_vertices_len);
        for (int i=0; i<branching_vertices_len; i++) {
            int v = branching_vertices[i];
            intersection(P, adjmat[v], new_P);
            intersection(X, adjmat[v], new_X);
            R.push_back(v);
            result += bk(R, new_P, new_X);
            R.pop_back();
            P.remove(v);
            X.add(v);
        }
#else
        bitset_foreach(branching_bitset, [&](int v){
            intersection(P, adjmat[v], new_P);
            intersection(X, adjmat[v], new_X);
            R.push_back(v);
            result += bk(R, new_P, new_X);
            R.pop_back();
            P.remove(v);
            X.add(v);
        }, P.get_num_words());
#endif
        return result;
    }

public:
    BK(int n, const vector<Bitset> & adjmat, const vector<vector<int>> & adjlists)
            : n(n), num_words(WORDS_REQUIRED(n)), adjmat(adjmat), adjlists(adjlists)
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

    vector<Bitset> adjmat;
    vector<vector<int>> adjlists;

    int line_num = 0;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line_num > 1) {
            int comma_pos = line.find(",");
            int v = std::stoi(line.substr(0, comma_pos));
            int w = std::stoi(line.substr(comma_pos + 1, line.length()));
            set_bit(adjmat[v], w);
            adjlists[v].push_back(w);
        } else if (line_num == 0) {
            n = std::stoi(line);
            adjmat = vector<Bitset>(n, Bitset(WORDS_REQUIRED(n)));
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
    auto ordered_adjmat = vector<Bitset>(n, Bitset(WORDS_REQUIRED(n)));
    auto ordered_adjlists = vector<vector<int>>(n);
    for (int v=0; v<n; v++) {
        for (int w : adjlists[v]) {
            int new_v = order_inv[v];
            int new_w = order_inv[w];
            ordered_adjlists[new_v].push_back(new_w);
            set_bit(ordered_adjmat[new_v], new_w);
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
