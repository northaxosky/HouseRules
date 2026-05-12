#include "PCH.h"

#include "Survival/HCManagerScript.h"

#include <cmath>
#include <cstdint>
#include <string_view>

namespace Survival::HCManagerScript
{
	namespace
	{
		constexpr std::uint32_t      kManagerQuestFormID = 0x0000080Eu;
		constexpr std::string_view   kScriptName{ "Hardcore:HC_ManagerScript" };

		// Cached for the duration of one save load. Reset() drops it.
		RE::BSTSmartPointer<RE::BSScript::Object> g_cachedObject;
		bool                                       g_warnedMissing = false;

		RE::TESQuest* FindHCManagerQuest()
		{
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				return nullptr;
			}
			for (auto* quest : dh->GetFormArray<RE::TESQuest>()) {
				if (quest && quest->formID == kManagerQuestFormID) {
					return quest;
				}
			}
			return nullptr;
		}

		RE::BSScript::Object* GetManagerObject()
		{
			if (g_cachedObject) {
				return g_cachedObject.get();
			}

			auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto* quest = FindHCManagerQuest();
			if (!vm || !quest) {
				return nullptr;
			}

			auto& policy = vm->GetObjectHandlePolicy();
			const auto handle = policy.GetHandleForObject(
				RE::BSScript::GetVMTypeID<RE::TESQuest>(), quest);
			if (handle == policy.EmptyHandle()) {
				return nullptr;
			}

			RE::BSTSmartPointer<RE::BSScript::Object> obj;
			if (!vm->FindBoundObject(handle, kScriptName.data(), true, obj, true) || !obj) {
				if (!g_warnedMissing) {
					REX::WARN("HCManagerScript: '{}' not bound on quest 0x{:08X}; Survival may be inactive",
						kScriptName, kManagerQuestFormID);
					g_warnedMissing = true;
				}
				return nullptr;
			}

			g_cachedObject = std::move(obj);
			g_warnedMissing = false;
			return g_cachedObject.get();
		}

		RE::BSScript::Variable* LookupProperty(const char* a_name)
		{
			auto* obj = GetManagerObject();
			if (!obj) {
				return nullptr;
			}
			RE::BSFixedString key{ a_name };
			auto* var = obj->GetProperty(key);
			if (!var) {
				REX::WARN("HCManagerScript: property '{}' not found on bound object", a_name);
			}
			return var;
		}
	}

	bool SetFloat(const char* a_property, float a_value)
	{
		auto* var = LookupProperty(a_property);
		if (!var) {
			return false;
		}
		if (var->is<float>()) {
			const float current = RE::BSScript::get<float>(*var);
			if (std::abs(current - a_value) > 1e-4f) {
				*var = a_value;
				REX::INFO("HCManagerScript: '{}' = {} (float)", a_property, a_value);
			}
			return true;
		}
		REX::WARN("HCManagerScript: '{}' is not float; skipping write", a_property);
		return false;
	}

	bool SetInt(const char* a_property, std::int32_t a_value)
	{
		auto* var = LookupProperty(a_property);
		if (!var) {
			return false;
		}
		if (var->is<std::int32_t>()) {
			const auto current = RE::BSScript::get<std::int32_t>(*var);
			if (current != a_value) {
				*var = a_value;
				REX::INFO("HCManagerScript: '{}' = {} (int)", a_property, a_value);
			}
			return true;
		}
		REX::WARN("HCManagerScript: '{}' is not int; skipping write", a_property);
		return false;
	}

	void Reset()
	{
		g_cachedObject.reset();
		g_warnedMissing = false;
	}
}
