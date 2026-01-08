#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

#include <interface.h>

int main() {
    struct distributionResults* results = runDistributionTest(100, 1000000);
    printf(
        "the rarest number is %i, it occured %i times\n", 
        results -> leastCommon,
        results -> leastCommonRarity
    );
    printf(
        "the most common number is %i, it occured %i times\n",
        results -> mostCommon,
        results -> mostCommonRarity
    );

    free(results);
}