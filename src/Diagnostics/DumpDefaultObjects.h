#pragma once

// One-shot diagnostic: walks BGSDefaultObject::GetSingleton() and logs every
// DO whose name matches one of the comma-separated prefixes in the INI filter
// [Diagnostic] sDumpFilter. For each match, resolves the form and dumps
// type-appropriate detail (MGEFs tagged with a keyword, ALCH effects arrays,
// FormList contents, Global values).
//
// Gated behind [Diagnostic] bDumpOnLoad — off by default; dev-only.
namespace Diagnostics::DumpDefaultObjects
{
	void MaybeRun();
}
