#include "PCH.h"

#include "Diagnostics/DumpDefaultObjects.h"

#include "Settings.h"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

// Iterates TESDataHandler form arrays for the types we care about and logs
// any whose editor ID starts with a filter token. The BGSDefaultObject
// singleton-array approach is unusable on OG (ID::BGSDefaultObject::Singleton
// is runtime-0 there), so we filter forms directly by their editorID instead.
// The DO-suffixed names (HardcoreHungerEffectKeyword_DO etc.) are just engine
// lookup aliases — the underlying keywords have their own editor IDs like
// HardcoreHungerEffect that this dump finds via prefix match.
namespace Diagnostics::DumpDefaultObjects
{
	namespace
	{
		std::vector<std::string> SplitFilter(const std::string& a_raw)
		{
			std::vector<std::string> out;
			std::size_t start = 0;
			for (std::size_t i = 0; i <= a_raw.size(); ++i) {
				if (i == a_raw.size() || a_raw[i] == ',') {
					auto token = a_raw.substr(start, i - start);
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.erase(token.begin());
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))  token.pop_back();
					if (!token.empty()) {
						out.push_back(std::move(token));
					}
					start = i + 1;
				}
			}
			return out;
		}

		bool NameMatches(std::string_view a_name, const std::vector<std::string>& a_filters)
		{
			if (a_filters.empty()) {
				return true;
			}
			for (const auto& f : a_filters) {
				if (f == "*") return true;
				if (a_name.find(f) != std::string_view::npos) {
					return true;
				}
			}
			return false;
		}

		const char* SafeEditorID(const RE::TESForm* a_form)
		{
			if (!a_form) return "<null>";
			const auto edid = a_form->GetFormEditorID();
			return edid ? edid : "";
		}

		void LogMGEFDetail(const RE::EffectSetting* a_mgef, const char* a_indent)
		{
			if (!a_mgef) return;
			const auto& d = a_mgef->data;
			REX::INFO("{}baseCost={} primaryAV='{}' secondaryAV='{}' archetype={} flags=0x{:X}",
				a_indent,
				d.baseCost,
				SafeEditorID(d.primaryAV),
				SafeEditorID(d.secondaryAV),
				static_cast<std::int32_t>(d.archetype.get()),
				static_cast<std::uint32_t>(d.flags.get()));
		}

		void DumpKeywordMGEFs(const RE::BGSKeyword* a_keyword)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) return;
			auto& mgefs = dh->GetFormArray<RE::EffectSetting>();
			int matched = 0;
			for (auto* mgef : mgefs) {
				if (!mgef) continue;
				if (!mgef->HasKeyword(a_keyword)) continue;
				++matched;
				REX::INFO("    [MGEF] {:08X} edid='{}'", mgef->formID, SafeEditorID(mgef));
				LogMGEFDetail(mgef, "        ");
			}
			REX::INFO("    ({} MGEFs tagged with this keyword)", matched);
		}

		void DetailMGEF(const RE::EffectSetting* m) { LogMGEFDetail(m, "    "); }

		void DumpAlchemyEffects(const RE::AlchemyItem* a_alch)
		{
			if (!a_alch) return;
			const auto& effs = a_alch->listOfEffects;
			REX::INFO("    listOfEffects.size()={}", effs.size());
			for (std::uint32_t i = 0; i < effs.size(); ++i) {
				const auto* eff = effs[i];
				if (!eff) continue;
				const auto* setting = eff->effectSetting;
				REX::INFO("    [{}] magnitude={} area={} duration={} rawCost={} setting={:08X} edid='{}'",
					i,
					eff->data.magnitude,
					eff->data.area,
					eff->data.duration,
					eff->rawCost,
					setting ? setting->formID : 0u,
					setting ? SafeEditorID(setting) : "");
			}
		}

		void DumpFormList(const RE::BGSListForm* a_fl)
		{
			if (!a_fl) return;
			REX::INFO("    arrayOfForms.size()={}", a_fl->arrayOfForms.size());
			std::uint32_t idx = 0;
			a_fl->ForEachForm([&idx](RE::TESForm* f) {
				if (f) {
					REX::INFO("    [{}] formType={} {:08X} edid='{}'",
						idx,
						static_cast<std::uint32_t>(f->GetFormType()),
						f->formID,
						SafeEditorID(f));
				}
				++idx;
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}

		template <class T>
		int DumpByEditorID(const char* a_label, const std::vector<std::string>& a_filters,
			void (*a_detail)(const T*))
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) return 0;
			auto& arr = dh->GetFormArray<T>();
			int matched = 0;
			for (auto* form : arr) {
				if (!form) continue;
				const auto edid = form->GetFormEditorID();
				if (!edid) continue;
				if (!NameMatches(edid, a_filters)) continue;
				++matched;
				REX::INFO("[{}] {:08X} edid='{}'", a_label, form->formID, edid);
				if (a_detail) a_detail(form);
			}
			return matched;
		}

		void DetailGlobal(const RE::TESGlobal* a_g) {
			if (a_g) REX::INFO("    value={}", a_g->value);
		}
	}

	void MaybeRun()
	{
		if (!MCM::Settings::Diagnostic::bDumpOnLoad.GetValue()) {
			return;
		}

		const auto& raw_filter = MCM::Settings::Diagnostic::sDumpFilter.GetValue();
		const auto  filters    = SplitFilter(raw_filter);
		REX::INFO("--- DumpDefaultObjects start (filter='{}' -> {} token(s)) ---",
			raw_filter, filters.size());

		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			REX::WARN("TESDataHandler singleton not available; aborting dump");
			return;
		}

		int kw    = DumpByEditorID<RE::BGSKeyword>   ("KYWD", filters, &DumpKeywordMGEFs);
		int alch  = DumpByEditorID<RE::AlchemyItem>  ("ALCH", filters, &DumpAlchemyEffects);
		int flst  = DumpByEditorID<RE::BGSListForm>  ("FLST", filters, &DumpFormList);
		int glob  = DumpByEditorID<RE::TESGlobal>    ("GLOB", filters, &DetailGlobal);
		int mgef  = DumpByEditorID<RE::EffectSetting>("MGEF", filters, &DetailMGEF);

		REX::INFO("--- DumpDefaultObjects done (KYWD={} ALCH={} FLST={} GLOB={} MGEF={}) ---",
			kw, alch, flst, glob, mgef);
	}
}
