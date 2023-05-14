#pragma once

namespace Utils {
	void Trim(std::string& s);
	RE::TESForm* GetFormFromIdentifier(std::string_view pluginName, std::uint32_t formId);
	RE::TESForm* GetFormFromIdentifier(std::string_view pluginName, std::string_view formIdStr);
}
