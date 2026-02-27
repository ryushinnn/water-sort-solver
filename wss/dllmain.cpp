#include <vector>
#include <unordered_set>
#include <algorithm>

#define EXPORT_API __declspec(dllexport)

struct Move {
    int from;
    int to;
};

struct PossibleMove {
    int from;
    int to;
    int priority;
};

typedef std::vector<std::vector<int>> State;

struct StateHash {
    std::size_t operator()(const State& state) const {
        std::size_t seed = state.size();
        for (const auto& bottle : state) {
            std::size_t bottle_hash = bottle.size();
            for (int color : bottle) {
                bottle_hash ^= color + 0x9e3779b9 + (bottle_hash << 6) + (bottle_hash >> 2);
            }
            seed ^= bottle_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

bool isCompleted(const std::vector<int>& bottle) {
    if (bottle.size() != 4) return false;
    int firstColor = bottle[0];
    for (int i=1; i<4;i++) {
        if (bottle[i] != firstColor) return false;
    }
    return true;
}

bool isSingleColor(const std::vector<int>& bottle) {
    if (bottle.empty()) return true;
    int fistColor = bottle[0];
    for (int color : bottle) {
        if (color != fistColor) return false;
    }
    return true;
}

bool isWin(const State& state) {
    for (const auto& bottle: state) {
        if (!bottle.empty() && !isCompleted(bottle)) return false;
    }
    return true;
}

bool executeMove(State& state, int from, int to) {
    if (state[from].empty() || state[to].size() >= 4) return false;
    int colorToMove = state[from].back();
    if (!state[to].empty() && state[to].back() != colorToMove) return false;

    while (!state[from].empty() && state[from].back() == colorToMove && state[to].size() < 4) {
        state[to].push_back(state[from].back());
        state[from].pop_back();
    }
    return true;
}

bool backTrack(State& current, std::vector<Move>& path, std::unordered_set<State, StateHash>& visited) {
    if (isWin(current)) return true;
    if (visited.count(current)) return false;
    visited.insert(current);

    std::vector<PossibleMove> moves;

    for (int i=0; i<current.size(); i++) {
        if (current[i].empty() || isCompleted(current[i])) continue;
        for (int j=0;j<current.size(); j++) {
            if (i==j) continue;

            if (isCompleted(current[j])) continue;
            if (current[j].size() == 4) continue;

            int colorToMove = current[i].back();
            if (!current[j].empty() && current[j].back() != colorToMove) continue;

            if (current[j].empty()) {
                if (isSingleColor(current[i])) continue;
                moves.push_back({i,j,0});
            }else {
                moves.push_back({i,j,1});
            }
        }
    }

    std::sort(moves.begin(), moves.end(), [](const PossibleMove& a,  const PossibleMove& b) {
        return a.priority > b.priority;
    });

    for (const auto& m : moves) {
        State nextState = current;
        executeMove(nextState, m.from, m.to);
        path.push_back({m.from,m.to});
        if (backTrack(nextState, path, visited)) return true;
        path.pop_back();
    }
    
    return false;
}

extern "C"{
    EXPORT_API int solve(int* color, int numBottles, int* outSteps) {
        State initialState(numBottles);
        for (int i=0;i<numBottles;i++) {
            for (int j=0; j<4;j++) {
                int c = color[i*4+j];
                if (c!=-1) initialState[i].push_back(c);
            }
        }

        std::vector<Move> path;
        std::unordered_set<State, StateHash> visited;

        if (backTrack(initialState, path, visited)) {
            for (int i=0; i<path.size(); i++) {
                outSteps[i*2] = path[i].from;
                outSteps[i*2+1] = path[i].to;
            }
            return (int)path.size();
        }

        return 0;
    }
}