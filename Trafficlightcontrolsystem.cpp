#include <iostream>
#include <unordered_map>
#include <vector>
#include <limits>
using namespace std;
const int INF = numeric_limits<int>::max();
const int MAX_CARS = 100; // Max cars in a queue for each direction
const int MAX_TIME = 100; // Max time steps
const int GREEN_DURATION = 10; // Min green light duration
const int RED_DURATION = 3; // Min all-red duration

enum LightPhase { NS_GREEN, EW_GREEN, ALL_RED };

struct State {
    int ns_cars, ew_cars;
    LightPhase phase;
    bool operator==(const State& other) const {
        return ns_cars == other.ns_cars && ew_cars == other.ew_cars && phase == other.phase;
    }
};

// Hash function for the State struct
namespace std {
    template <>
    struct hash<State> {
        size_t operator()(const State& s) const {
            return ((hash<int>()(s.ns_cars) ^ (hash<int>()(s.ew_cars) << 1)) >> 1) ^ std::hash<int>()(s.phase);
        }
    };
}

// Transition function to update state based on current traffic flow and light phase
State transition(const State& state, int ns_arrivals, int ew_arrivals, LightPhase new_phase) {
    int ns_cars = state.ns_cars + ns_arrivals;
    int ew_cars = state.ew_cars + ew_arrivals;
    switch (new_phase) {
        case NS_GREEN:
            ns_cars = max(0, ns_cars - GREEN_DURATION); // Reduce cars by green duration
            break;
        case EW_GREEN:
            ew_cars = max(0, ew_cars - GREEN_DURATION); // Reduce cars by green duration
            break;
        case ALL_RED:
            // No change in cars; intersection clearing phase
            break;
    }
    return {ns_cars, ew_cars, new_phase};
}

// Cost function to calculate waiting cost based on queue lengths
int waitingCost(int ns_cars, int ew_cars) {
    return ns_cars + ew_cars; // Simple cost as total cars waiting
}

int trafficLightDP(int steps, const vector<int>& ns_arrival,
         const vector<int>& ew_arrival) {
    // DP table with state and time index
   unordered_map<State, int> dp[MAX_TIME + 1];

    // Initial state: no cars, all-red phase
    State initial = {0, 0, ALL_RED};
    dp[0][initial] = 0;

    for (int t = 0; t < steps; ++t) {
        for (const auto& [state, cost] : dp[t]) {
            // Evaluate transitions for each light phase
            for (LightPhase new_phase : {NS_GREEN, EW_GREEN, ALL_RED}) {
                State next_state = transition(state, ns_arrival[t], ew_arrival[t], new_phase);
                int new_cost = cost + waitingCost(next_state.ns_cars, next_state.ew_cars);

                // Update DP table if a cheaper path is found
                if (dp[t + 1].count(next_state) == 0 || new_cost < dp[t + 1][next_state]) {
                    dp[t + 1][next_state] = new_cost;
                }
            }
        }
    }

    // Retrieve minimum cost at final time step
    int min_cost = INF;
    for (const auto& [state, cost] : dp[steps]) {
        min_cost = min(min_cost, cost);
    }
    return min_cost;
}

int main() {
    int num_steps = 10;
    vector<int> ns_arrival = {3, 4, 2, 5, 3, 4, 1, 2, 3, 4}; // Simulated arrival rates
    vector<int> ew_arrival = {2, 3, 5, 2, 1, 3, 4, 3, 2, 3};

    int result = trafficLightDP(num_steps, ns_arrival, ew_arrival);
    cout << "Minimum cumulative waiting time: " << result << endl;
    return 0;
}
