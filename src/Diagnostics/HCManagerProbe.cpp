#include "PCH.h"

#include "Diagnostics/HCManagerProbe.h"

#include "Settings.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace Diagnostics::HCManagerProbe
{
	namespace
	{
		constexpr std::string_view kScriptName{ "Hardcore:HC_ManagerScript" };

		bool g_ran = false;

		std::string ToLower(std::string_view a_value)
		{
			std::string out{ a_value };
			std::ranges::transform(out, out.begin(), [](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});
			return out;
		}

		std::vector<std::string> SplitFilter(const std::string& a_raw)
		{
			std::vector<std::string> out;
			std::size_t start = 0;
			for (std::size_t i = 0; i <= a_raw.size(); ++i) {
				if (i == a_raw.size() || a_raw[i] == ',') {
					auto token = a_raw.substr(start, i - start);
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) {
						token.erase(token.begin());
					}
					while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) {
						token.pop_back();
					}
					if (!token.empty()) {
						out.push_back(ToLower(token));
					}
					start = i + 1;
				}
			}
			return out;
		}

		bool NameMatches(std::string_view a_name, const std::vector<std::string>& a_filters)
		{
			if (a_name.empty()) {
				return false;
			}
			if (a_filters.empty()) {
				return true;
			}

			const auto lower = ToLower(a_name);
			for (const auto& filter : a_filters) {
				if (filter == "*" || lower.find(filter) != std::string::npos) {
					return true;
				}
			}
			return false;
		}

		const char* SafeString(const char* a_value)
		{
			return a_value ? a_value : "";
		}

		const char* SafeEditorID(const RE::TESForm* a_form)
		{
			if (!a_form) {
				return "";
			}
			const auto edid = a_form->GetFormEditorID();
			return edid ? edid : "";
		}

		std::string Hex(std::uint64_t a_value, std::uint32_t a_width = 0)
		{
			std::ostringstream out;
			out.imbue(std::locale::classic());
			out << std::uppercase << std::hex << std::setfill('0');
			if (a_width > 0) {
				out << std::setw(static_cast<int>(a_width));
			}
			out << a_value;
			return out.str();
		}

		const char* RawTypeName(RE::BSScript::TypeInfo::RawType a_type)
		{
			using RawType = RE::BSScript::TypeInfo::RawType;
			switch (a_type) {
				case RawType::kNone: return "none";
				case RawType::kObject: return "object";
				case RawType::kString: return "string";
				case RawType::kInt: return "int";
				case RawType::kFloat: return "float";
				case RawType::kBool: return "bool";
				case RawType::kVar: return "var";
				case RawType::kStruct: return "struct";
				case RawType::kArrayObject: return "object[]";
				case RawType::kArrayString: return "string[]";
				case RawType::kArrayInt: return "int[]";
				case RawType::kArrayFloat: return "float[]";
				case RawType::kArrayBool: return "bool[]";
				case RawType::kArrayVar: return "var[]";
				case RawType::kArrayStruct: return "struct[]";
				default: return "unknown";
			}
		}

		template <class T>
		T* ResolveHandle(RE::BSScript::IObjectHandlePolicy& a_policy, std::size_t a_handle)
		{
			if (!a_policy.HandleIsType(RE::BSScript::GetVMTypeID<T>(), a_handle) ||
			    !a_policy.IsHandleObjectAvailable(a_handle)) {
				return nullptr;
			}
			return static_cast<T*>(a_policy.GetObjectForHandle(RE::BSScript::GetVMTypeID<T>(), a_handle));
		}

		std::string DescribeObjectValue(const RE::BSScript::Variable& a_var)
		{
			auto object = RE::BSScript::get<RE::BSScript::Object>(a_var);
			if (!object) {
				return "object=None";
			}

			auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			if (!vm) {
				return "object vm=unavailable";
			}

			auto& policy = vm->GetObjectHandlePolicy();
			const auto handle = object->GetHandle();

			std::ostringstream out;
			out << "object handle=0x" << Hex(handle);

			std::uint32_t handleType = 0;
			if (policy.GetHandleType(handle, handleType)) {
				out << " handleType=" << handleType;
			}

			if (auto* global = ResolveHandle<RE::TESGlobal>(policy, handle)) {
				out << " form=" << Hex(global->formID, 8)
				    << " edid='" << SafeEditorID(global) << "'"
				    << " value=" << global->value;
			} else if (auto* av = ResolveHandle<RE::ActorValueInfo>(policy, handle)) {
				out << " form=" << Hex(av->formID, 8)
				    << " edid='" << SafeEditorID(av) << "'";
			} else if (auto* quest = ResolveHandle<RE::TESQuest>(policy, handle)) {
				out << " form=" << Hex(quest->formID, 8)
				    << " edid='" << SafeEditorID(quest) << "'";
			}

			return out.str();
		}

		std::string DescribeValue(const RE::BSScript::Variable& a_var)
		{
			if (a_var.is<std::nullptr_t>()) {
				return "None";
			}
			if (a_var.is<std::int32_t>()) {
				return std::to_string(RE::BSScript::get<std::int32_t>(a_var));
			}
			if (a_var.is<float>()) {
				std::ostringstream out;
				out.imbue(std::locale::classic());
				out << RE::BSScript::get<float>(a_var);
				return out.str();
			}
			if (a_var.is<bool>()) {
				return RE::BSScript::get<bool>(a_var) ? "true" : "false";
			}
			if (a_var.is<RE::BSFixedString>()) {
				return std::string{ "'" } + SafeString(RE::BSScript::get<RE::BSFixedString>(a_var).c_str()) + "'";
			}
			if (a_var.is<RE::BSScript::Object>()) {
				return DescribeObjectValue(a_var);
			}
			if (a_var.is<RE::BSScript::Array>()) {
				const auto array = RE::BSScript::get<RE::BSScript::Array>(a_var);
				if (!array) {
					return "array=None";
				}
				std::ostringstream out;
				out << "array size=" << array->size()
				    << " elemType=" << RawTypeName(array->type_info().GetRawType());
				return out.str();
			}

			return std::string{ "rawType=" } + RawTypeName(a_var.GetType().GetRawType());
		}

		void LogVariable(
			const char* a_kind,
			std::string_view a_name,
			std::uint32_t a_index,
			const RE::BSScript::Variable& a_var)
		{
			REX::INFO("HCProbe: {} name='{}' index={} type={} value={}",
				a_kind,
				a_name,
				a_index,
				RawTypeName(a_var.GetType().GetRawType()),
				DescribeValue(a_var));
		}

		std::size_t LogProperties(RE::BSScript::Object& a_object, const std::vector<std::string>& a_filters)
		{
			std::size_t matched = 0;
			for (auto* type = a_object.GetTypeInfo(); type; type = type->GetParent()) {
				const auto props = type->GetPropertyIter();
				for (std::uint32_t i = 0; props && i < type->GetNumProperties(); ++i) {
					const auto& prop = props[i];
					const auto name = std::string_view{ SafeString(prop.name.c_str()) };
					if (!NameMatches(name, a_filters)) {
						continue;
					}
					const auto* value = a_object.GetProperty(prop.name);
					if (!value) {
						REX::WARN("HCProbe: property name='{}' index={} missing_value", name, prop.info.autoVarIndex);
						continue;
					}
					++matched;
					LogVariable("property", name, prop.info.autoVarIndex, *value);
				}
			}
			return matched;
		}

		std::size_t LogVariables(
			RE::BSScript::IVirtualMachine& a_vm,
			RE::BSScript::Object& a_object,
			const std::vector<std::string>& a_filters)
		{
			std::size_t matched = 0;
			for (auto* type = a_object.GetTypeInfo(); type; type = type->GetParent()) {
				const auto vars = type->GetVariableIter();
				for (std::uint32_t i = 0; vars && i < type->GetNumVariables(); ++i) {
					const auto& varInfo = vars[i];
					const auto name = std::string_view{ SafeString(varInfo.name.c_str()) };
					if (!NameMatches(name, a_filters)) {
						continue;
					}

					RE::BSScript::Variable value;
					if (!a_vm.GetVariableValue(a_object.GetHandle(), type->name, i, value)) {
						REX::WARN("HCProbe: variable name='{}' index={} read_failed", name, i);
						continue;
					}
					++matched;
					LogVariable("variable", name, i, value);
				}
			}
			return matched;
		}

		bool ShouldRerun(const char* a_reason)
		{
			return MCM::Settings::Diagnostic::bHCManagerProbeRerunOnPause.GetValue() &&
			       std::string_view{ SafeString(a_reason) } == "PauseMenu";
		}

		void Run(const char* a_reason)
		{
			const auto& rawFilter = MCM::Settings::Diagnostic::sHCManagerProbeFilter.GetValue();
			const auto filters = SplitFilter(rawFilter);

			auto* dh = RE::TESDataHandler::GetSingleton();
			auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			if (!dh || !vm) {
				REX::WARN("HCProbe: skipped reason='{}' tesDataHandler={} vm={}",
					SafeString(a_reason),
					dh ? 1 : 0,
					vm ? 1 : 0);
				return;
			}

			auto& quests = dh->GetFormArray<RE::TESQuest>();
			REX::INFO("HCProbe: --- begin reason='{}' filter='{}' questArray={} ---",
				SafeString(a_reason),
				rawFilter,
				quests.size());

			auto& policy = vm->GetObjectHandlePolicy();
			std::size_t scanned = 0;
			std::size_t matchedObjects = 0;
			std::size_t matchedProperties = 0;
			std::size_t matchedVariables = 0;

			for (auto* quest : quests) {
				if (!quest) {
					continue;
				}
				++scanned;

				const auto handle = policy.GetHandleForObject(RE::BSScript::GetVMTypeID<RE::TESQuest>(), quest);
				if (handle == policy.EmptyHandle()) {
					continue;
				}

				RE::BSTSmartPointer<RE::BSScript::Object> object;
				if (!vm->FindBoundObject(handle, kScriptName.data(), true, object, true) || !object) {
					continue;
				}

				++matchedObjects;
				const auto* type = object->GetTypeInfo();
				REX::INFO("HCProbe: manager quest={:08X} edid='{}' handle=0x{} script='{}' type='{}' constructed={} initialized={} valid={} state='{}'",
					quest->formID,
					SafeEditorID(quest),
					Hex(handle),
					kScriptName,
					type ? SafeString(type->GetName()) : "",
					object->IsConstructed() ? 1 : 0,
					object->IsInitialized() ? 1 : 0,
					object->IsValid() ? 1 : 0,
					SafeString(object->currentState.c_str()));

				if (type) {
					REX::INFO("HCProbe: scriptStats variables={} properties={} userFlags={} states={}",
						type->GetVariableCount(),
						type->GetNumProperties(),
						type->GetNumUserFlags(),
						type->GetNumNamedStates());
				}

				matchedProperties += LogProperties(*object, filters);
				matchedVariables += LogVariables(*vm, *object, filters);
			}

			REX::INFO("HCProbe: --- end scannedQuests={} managerObjects={} properties={} variables={} ---",
				scanned,
				matchedObjects,
				matchedProperties,
				matchedVariables);
		}
	}

	void MaybeRun(const char* a_reason)
	{
		if (!MCM::Settings::Diagnostic::bHCManagerProbe.GetValue()) {
			return;
		}

		const bool rerun = ShouldRerun(a_reason);
		if (g_ran && !rerun) {
			return;
		}

		Run(a_reason);
		g_ran = true;
	}

	void Reset()
	{
		g_ran = false;
	}
}
