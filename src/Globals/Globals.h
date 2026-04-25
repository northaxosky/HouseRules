#pragma once

namespace Globals
{
	// Resolve an HR_ GLOB (shipped in HouseRules.esp) by EditorID and write
	// a_value into its ->value. No-op (with a one-shot warn) if the global
	// cannot be found, which is the state when the user is running the DLL
	// without the companion ESP.
	//
	// Safe to call from Settings::Update / PauseMenu-close sinks. Uses a
	// linear scan of TESDataHandler::GetFormArray<TESGlobal>() to avoid the
	// OG BSStaticTriShapeDB preload trap that TESForm::GetFormByID /
	// TESDataHandler::LookupForm hit on cold meshes.
	void Write(const char* a_editorID, float a_value);

	// Convenience wrapper for boolean-flavored knobs. Writes 1.0f/0.0f.
	inline void WriteBool(const char* a_editorID, bool a_value)
	{
		Write(a_editorID, a_value ? 1.0f : 0.0f);
	}
}
