#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

// this file exists to test the distribution reliability of random outputs

struct distributionResults {
    int leastCommon; // the number that was pulled the least often
    int leastCommonRarity; // the times this number was pulled
    int mostCommon; // the same but the number that was the most common
    int mostCommonRarity;
    int range; // the highest possible number plus one (the range is from 0 to rangeEnd - 1)
    int samples; // the amount of numbers pulled
};

// pulls _samples samples from 0 - (_range - 1) and returns a struct distributionResults
struct distributionResults* runDistributionTest(int _range, int _samples) {
    if (_range <= 0 || _samples <= 0) {
        printf("you need to enter positive values in to runDistributionTest.");
        return NULL;
    }

    struct distributionResults* toReturn = malloc(sizeof(struct distributionResults));
    toReturn -> range = _range;
    toReturn -> samples = _samples;

    int* sampleOccurences = malloc(_range * sizeof(int));
    for (int i = 0; i < _range; i++) {
        sampleOccurences[i] = 0;
    }

    unsigned int randomSample;
    for (int i = 0; i < _samples; i++) {
        getrandom(&randomSample, sizeof(int), 0);
        randomSample = randomSample % _range;
        sampleOccurences[randomSample]++;
    }

    toReturn -> leastCommon = 0;
    toReturn -> leastCommonRarity = _samples;
    toReturn -> mostCommon = 0;
    toReturn -> mostCommonRarity = 0;
    for (int outcome = 0; outcome < _range; outcome++) {
        int rarity = sampleOccurences[outcome];
        if (rarity < toReturn -> leastCommonRarity) {
            toReturn -> leastCommon = outcome;
            toReturn -> leastCommonRarity = rarity;
        } else if (rarity > toReturn -> mostCommonRarity) {
            toReturn -> mostCommon = outcome;
            toReturn -> mostCommonRarity = rarity;
        }
    }

    free(sampleOccurences);
    return toReturn;
}