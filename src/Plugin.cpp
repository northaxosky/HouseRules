#include "PCH.h"

F4SE_EXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	F4SE::PluginVersionData v{};
	v.PluginVersion({ 0, 1, 0, 0 });
	v.PluginName("SurvivalArchitect");
	v.AuthorName("Kuz");
	v.UsesAddressLibrary(true);
	v.UsesSigScanning(false);
	v.IsLayoutDependent(true);
	v.HasNoStructUse(false);
	v.CompatibleVersions({
		F4SE::RUNTIME_1_10_163,  // pre-next-gen
		F4SE::RUNTIME_1_10_984,  // next-gen
		F4SE::RUNTIME_LATEST,    // current
	});
	return v;
}();
