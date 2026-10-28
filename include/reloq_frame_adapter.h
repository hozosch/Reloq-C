#ifndef RELOQ_FRAME_ADAPTER_H
#define RELOQ_FRAME_ADAPTER_H

#include "reloq_core.h"
#include "reloq_parameter_stream.h"

/* Map the 62 numeric controls of one already scheduled Klatt frame to the
 * typed C renderer. Control 0 supplies the frame duration in milliseconds;
 * a parameter stream's tick spacing does not determine this duration.
 *
 * Controls 11, 12 and 52..61 have no mapped effect in the verified numeric
 * frame mode. All controls must still be finite. The fhighest selector is a
 * session-wide setting (F1..F8), supplied separately from the 62 controls.
 * On invalid input, return -1 without changing *out. No text, phoneme,
 * timing-knot or host interpretation is performed here. */
int reloq_frame_adapter_map(
    const float controls[RELOQ_PARAMETER_TRACKS], long fhighest,
    struct reloq_core_voiced_frame *out);

#endif
