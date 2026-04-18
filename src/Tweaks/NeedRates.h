#pragma once

// Applies MCM need-rate multipliers (hunger/thirst/sleep) to the corresponding
// GMSTs. Must be invoked after settings are loaded and the game data is ready.
namespace Tweaks::NeedRates
{
	void Apply();
}
