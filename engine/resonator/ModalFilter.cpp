#include "resonator/ModalFilter.h"

namespace engine {

void ModalFilter::prepare(float newSampleRate)
{
    sampleRate = newSampleRate;
    filter.prepare(sampleRate);
    filter.setPeakingEQ(params.frequencyHz, params.Q, params.gainDb);
}

void ModalFilter::reset()
{
    filter.reset();
}

void ModalFilter::setParams(const ModalFilterParams& newParams)
{
    params = newParams;
    filter.setPeakingEQ(params.frequencyHz, params.Q, params.gainDb);
}

float ModalFilter::processSample(float input)
{
    return filter.processSample(input);
}

} // namespace engine
