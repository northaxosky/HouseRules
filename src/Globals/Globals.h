#pragma once

namespace Globals
{
	// Write a TESGlobal looked up by FormID; for vanilla Fallout4.esm masters where the FormID is stable.
	void WriteByFormID(std::uint32_t a_formID, float a_value);
}
