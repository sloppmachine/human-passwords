struct distributionResults {
    int leastCommon; // the number that was pulled the least often
    int leastCommonRarity; // the times this number was pulled
    int mostCommon; // the same but the number that was the most common
    int mostCommonRarity;
    int range; // the highest possible number plus one (the range is from 0 to rangeEnd - 1)
    int samples; // the amount of numbers pulled
};

// pulls _samples samples from 0 - (_range - 1) and returns a struct distributionResults
struct distributionResults* runDistributionTest(int _range, int _samples);