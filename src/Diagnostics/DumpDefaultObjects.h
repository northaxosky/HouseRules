#pragma once

// One-shot diagnostic: walks TESDataHandler form arrays and logs records whose
// editor ID matches one of the comma-separated prefixes in [Diagnostic]
// sDumpFilter. For each match, dumps type-appropriate detail (MGEFs tagged with
// a keyword, ALCH effects arrays, FormList contents, Global values).
//
// Gated behind [Diagnostic] bDumpOnLoad — off by default; dev-only.
namespace Diagnostics::DumpDefaultObjects
{
	void MaybeRun();
}
