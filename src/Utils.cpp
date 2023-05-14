#include "Utils.h"

namespace Utils {
	void Trim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	RE::TESForm* GetFormFromIdentifier(std::string_view pluginName, std::uint32_t formId) {
		RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
		if (!g_dataHandler)
			return nullptr;

		return g_dataHandler->LookupForm(formId, pluginName);
	}

	RE::TESForm* GetFormFromIdentifier(std::string_view pluginName, std::string_view formIdStr) {
		std::uint32_t formID = std::stoul(std::string(formIdStr), nullptr, 16) & 0xFFFFFF;
		return GetFormFromIdentifier(pluginName, formID);
	}
}
