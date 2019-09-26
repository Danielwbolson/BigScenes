#ifndef BLOOM_H
#define BLOOM_H

void initBloom();
void computeBloomBlur();

//Configuration values that can be set:
extern unsigned int numBloomPasses;

//Values we write:
extern bool horizontal;

#endif //BLOOM_H