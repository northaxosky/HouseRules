#pragma once

// Difficulty II: per-difficulty effect-duration and effect-magnitude GMST
// sliders. Uses the shared Tweaks::GameSettings helper (snapshot once,
// write baseline * MCM mult). Skips legacy SV; current Survival is TSV.
//
// Survival baselines are unusual: fDiffMultEffectDuration_TSV is much longer
// than Normal and fDiffMultEffectMagnitude_TSV is much smaller than Normal.
// 1.00 in MCM means "vanilla Survival baseline", not "Normal baseline".
namespace Tweaks::DifficultyEffects
{
	void Apply();
}
