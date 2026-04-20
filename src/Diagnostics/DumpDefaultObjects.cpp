#include "PCH.h"

#include "Diagnostics/DumpDefaultObjects.h"

#include "Settings.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

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
					// trim whitespace
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
				return true;  // no filter = pass everything
			}
			for (const auto& f : a_filters) {
				if (f == "*") return true;
				if (a_name.size() >= f.size() && a_name.compare(0, f.size(), f) == 0) {
					return true;
				}
			}
			return false;
		}

		const char* FormTypeName(RE::ENUM_FORM_ID a_type)
		{
			using T = RE::ENUM_FORM_ID;
			switch (a_type) {
				case T::kKYWD: return "KYWD";
				case T::kMGEF: return "MGEF";
				case T::kALCH: return "ALCH";
				case T::kFLST: return "FLST";
				case T::kGLOB: return "GLOB";
				case T::kSPEL: return "SPEL";
				case T::kPERK: return "PERK";
				case T::kAVIF: return "AVIF";
				case T::kMISC: return "MISC";
				case T::kWEAP: return "WEAP";
				case T::kARMO: return "ARMO";
				case T::kNPC_: return "NPC_";
				case T::kRACE: return "RACE";
				case T::kFACT: return "FACT";
				case T::kQUST: return "QUST";
				case T::kCOBJ: return "COBJ";
				case T::kFURN: return "FURN";
				default:       return "?";
			}
		}

		const char* AVEditorID(const RE::ActorValueInfo* a_av)
		{
			if (!a_av) {
				return "<none>";
			}
			const auto edid = a_av->GetFormEditorID();
			return edid ? edid : "<unnamed AV>";
		}

		const char* FormEditorID(const RE::TESForm* a_form)
		{
			if (!a_form) {
				return "<null>";
			}
			const auto edid = a_form->GetFormEditorID();
			return edid ? edid : "";
		}

		void DumpKeywordMGEFs(const RE::BGSKeyword* a_keyword)
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				REX::WARN("    (no TESDataHandler)");
				return;
			}
			auto& mgefs = dh->GetFormArray<RE::EffectSetting>();
			int matched = 0;
			for (auto* mgef : mgefs) {
				if (!mgef) continue;
				if (!mgef->HasKeyword(a_keyword)) continue;
				++matched;
				const auto& d = mgef->data;
				REX::INFO("    [MGEF] {:08X} edid='{}' baseCost={} primaryAV='{}' secondaryAV='{}' archetype={} flags=0x{:X}",
					mgef->formID,
					FormEditorID(mgef),
					d.baseCost,
					AVEditorID(d.primaryAV),
					AVEditorID(d.secondaryAV),
					static_cast<std::int32_t>(d.archetype.get()),
					static_cast<std::uint32_t>(d.flags.get()));
			}
			REX::INFO("    ({} MGEFs tagged with this keyword)", matched);
		}

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
					setting ? FormEditorID(setting) : "");
			}
		}

		void DumpFormList(const RE::BGSListForm* a_fl)
		{
			if (!a_fl) return;
			REX::INFO("    arrayOfForms.size()={}", a_fl->arrayOfForms.size());
			std::uint32_t idx = 0;
			a_fl->ForEachForm([&idx](RE::TESForm* f) {
				if (f) {
					REX::INFO("    [{}] {} {:08X} edid='{}'",
						idx,
						FormTypeName(f->GetFormType()),
						f->formID,
						FormEditorID(f));
				}
				++idx;
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}

		void DumpGlobal(const RE::TESGlobal* a_glob)
		{
			if (!a_glob) return;
			REX::INFO("    value={}", a_glob->value);
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

		auto* dos = RE::BGSDefaultObject::GetSingleton();
		if (!dos) {
			REX::WARN("BGSDefaultObject singleton not available; aborting dump");
			return;
		}

		int do_count    = 0;
		int match_count = 0;
		for (auto* entry : *dos) {
			if (!entry) continue;
			++do_count;

			const auto name = std::string_view{ entry->formEditorID };
			if (!NameMatches(name, filters)) continue;
			++match_count;

			auto* form = entry->form;
			if (!form) {
				REX::INFO("[DO] {} -> <unresolved>", name);
				continue;
			}

			REX::INFO("[DO] {} -> {} {:08X} edid='{}'",
				name,
				FormTypeName(form->GetFormType()),
				form->formID,
				FormEditorID(form));

			switch (form->GetFormType()) {
				case RE::ENUM_FORM_ID::kKYWD:
					DumpKeywordMGEFs(form->As<RE::BGSKeyword>());
					break;
				case RE::ENUM_FORM_ID::kALCH:
					DumpAlchemyEffects(form->As<RE::AlchemyItem>());
					break;
				case RE::ENUM_FORM_ID::kFLST:
					DumpFormList(form->As<RE::BGSListForm>());
					break;
				case RE::ENUM_FORM_ID::kGLOB:
					DumpGlobal(form->As<RE::TESGlobal>());
					break;
				default:
					break;
			}
		}

		REX::INFO("--- DumpDefaultObjects done ({}/{} DOs matched filter) ---",
			match_count, do_count);
	}
}
