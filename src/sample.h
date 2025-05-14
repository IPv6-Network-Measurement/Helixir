#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_HOPLIMITS 256    // Range of hoplimit values (0-255)
#define NEIGHBOR_RADIUS 3    // Propagation range (¡À3)
#define PRIOR_ALPHA 1.0      // Initial parameter for the Beta distribution
#define PRIOR_BETA 1.0
#define DECAY_FACTOR 0.01    // Confidence decay factor

// Generate a random number from the Beta distribution (approximate)
double beta_sample(double a, double b);

// Select the next hoplimit
int select_next_hoplimit(int index);

// Update observations and propagate the influence to neighboring hoplimits
void update_observations(int hoplimit, int index);

// Initialize the Beta distribution
void initialize(int index);