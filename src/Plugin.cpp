#include "PCH.h"

// Runtime versions we support.
namespace
{
	constexpr auto kRuntimeOG  = REL::Version{ 1, 10, 163, 0 };   // pre-Next-Gen
	constexpr auto kRuntimeNG  = REL::Version{ 1, 10, 984, 0 };   // Next-Gen

	constexpr std::uint32_t kPluginVersion = (0u << 24) | (2u << 16) | (0u << 4) | 0u;  // 0.2.0.0
}

// New-API plugin declaration (Next-Gen runtimes, 1.10.980+).
F4SE_EXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	F4SE::PluginVersionData v{};
	v.PluginVersion({ 0, 2, 0, 0 });
	v.PluginName("HouseRules");
	v.AuthorName("Kuz");
	v.UsesAddressLibrary(true);   // 1.11.137+ AddressLib
	v.UsesSigScanning(false);
	v.IsLayoutDependent(true);    // 1.11.137+ layout
	v.HasNoStructUse(false);
	v.CompatibleVersions({ kRuntimeNG, F4SE::RUNTIME_LATEST });
	return v;
}();

// Old-API plugin declaration (pre-NG runtime 1.10.163).
F4SE_EXPORT bool F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
	if (!a_f4se || !a_info) {
		return false;
	}

	if (a_f4se->RuntimeVersion() != kRuntimeOG) {
		return false;
	}

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name        = "HouseRules";
	a_info->version     = kPluginVersion;
	return true;
}
