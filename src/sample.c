#include "sample.h"

// Generate a random number from the Beta distribution (approximate)
double beta_sample(double a, double b) {
    double x = rand() / (double)RAND_MAX;
    double y = rand() / (double)RAND_MAX;
    return pow(x, 1.0 / a) / (pow(x, 1.0 / a) + pow(y, 1.0 / b));
}

// Select the next hoplimit
int select_next_hoplimit(int index) {
    PrefixInfo* tPCS = &prefix_table[index];
    double max_sample = -1.0;
    int best_hoplimit = 0;
    for (int i = 0; i < tPCS -> hop_candidate_count; i++) {
        uint8_t hop = tPCS -> hop_candidates[i];
        double sample = beta_sample(tPCS->alpha_param[hop], tPCS->beta_param[hop]);
        if (sample > max_sample) {
            max_sample = sample;
            best_hoplimit = hop;
        }
    }
    return best_hoplimit;
}

// Update observations and propagate the influence to neighboring hoplimits
void update_observations(int hoplimit, int index) {
    PrefixInfo* tPCS = &prefix_table[index];
    // Update alpha only
    tPCS->alpha_param[hoplimit] += 1.0;

    // Propagate to neighbors (¡ÀNEIGHBOR_RADIUS)
    for (int i = -NEIGHBOR_RADIUS; i <= NEIGHBOR_RADIUS; i++) {
        int neighbor = hoplimit + i;
        if (neighbor >= 0 && neighbor < NUM_HOPLIMITS) {
            if (tPCS -> hop_candidate_flags[neighbor] == 0)
            {
                tPCS -> hop_candidate_flags[neighbor] = 1;
                tPCS -> hop_candidates[tPCS->hop_candidate_count ++] = neighbor;
            }
            double influence = exp(-fabs(i) / NEIGHBOR_RADIUS); // Influence decreases with distance
            tPCS->alpha_param[neighbor] += influence * 0.5;
        }
    }

    for (int i = 0; i < NUM_HOPLIMITS; i++) {
        if (i != hoplimit) {
            tPCS->beta_param[i] += DECAY_FACTOR;  // Decrease confidence for hoplimits not chosen for a long time
        }
    }
}

// Initialize the Beta distribution
void initialize(int index) {
    PrefixInfo* tPCS = &prefix_table[index];
    for (int i = 0; i < NUM_HOPLIMITS; i++) {
        tPCS->alpha_param[i] = PRIOR_ALPHA;
        tPCS->beta_param[i] = PRIOR_BETA;
    }
}