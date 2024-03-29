#ifndef MD_PITCH_EUCLID_H__
#define MD_PITCH_EUCLID_H__

#include <MD.h>
#include <Sequencer.h>
#include <Scales.h>

class MDPitchEuclid {
 public:
	EuclidDrumTrack track;
	const scale_t *currentScale;
	
	uint8_t pitches[32];
	uint8_t pitches_len;
	uint8_t pitches_idx;

	uint8_t mdTrack;

	uint8_t octaves;
	uint8_t basePitch;

	bool muted;

	MDPitchEuclid();

	void setPitchLength(uint8_t len);
	void randomizePitches();
	void on16Callback(uint32_t counter);

	static const uint8_t NUM_SCALES = 7;
	static const scale_t *scales[NUM_SCALES];
};

#endif /* MD_PITCH_EUCLID_H__ */

