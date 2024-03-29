#include "Configs.h"

#include <Windows.h>
#include <regex>
#include <fstream>
#include <algorithm>

#include "Utils.h"

namespace Configs {
	const HitSpell::SpellMap& ConfigMap::GetSpellMap() {
		return spellMap;
	}

	void ConfigMap::AddSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, std::int32_t a_bodyPartType, RE::SpellItem* a_spell, HitSpell::PriorityData& priority) {
		std::uint32_t weapFormID = a_weap ? a_weap->formID : 0;
		std::uint32_t ammoFormID = a_ammo ? a_ammo->formID : 0;
		std::uint32_t raceFormID = a_vicRace ? a_vicRace->formID : 0;

		std::uint64_t key = static_cast<std::uint64_t>(weapFormID) << 32 | ammoFormID;

		auto map_iter = spellMap.find(key);
		if (map_iter == spellMap.end()) {
			HitSpell::BodyPartSpell nBPSpell;
			std::unordered_map<std::uint32_t, HitSpell::BodyPartSpell> newVicMap = { std::make_pair(raceFormID, nBPSpell) };

			auto ins_res = spellMap.insert(std::make_pair(key, newVicMap));
			if (!ins_res.second)
				return;

			map_iter = ins_res.first;
		}

		auto vr_iter = map_iter->second.find(raceFormID);
		if (vr_iter == map_iter->second.end()) {
			HitSpell::BodyPartSpell nBPSpell;
			auto ins_res = map_iter->second.insert(std::make_pair(raceFormID, nBPSpell));
			if (!ins_res.second)
				return;

			vr_iter = ins_res.first;
		}

		if (a_bodyPartType == -1)
			vr_iter->second.Whole.push_back({ a_spell, priority });
		else
			vr_iter->second.Part[a_bodyPartType].push_back({ a_spell, priority });
	}

	std::string GetIniValue(std::string_view section, std::string_view key) {
		const std::string configPath("Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + ".ini");
		char resultBuf[256] = { 0 };
		GetPrivateProfileStringA(section.data(), key.data(), NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
		return resultBuf;
	}

	void LoadIni(ConfigMap* cMap) {
		std::string iniVal;
		
		iniVal = GetIniValue("Settings", "bEnablePlayerTarget");
		if (!iniVal.empty())
			cMap->SetPlayerTarget(std::stoul(iniVal) == 0 ? false : true);

		iniVal = GetIniValue("Settings", "bPlayerOnlyHitSpell");
		if (!iniVal.empty())
			cMap->SetPlayerOnlyHitSpell(std::stoul(iniVal) == 0 ? false : true);

		logger::info("bEnablePlayerTarget: {}"sv, cMap->PlayerTargetEnabled());
		logger::info("bPlayerOnlyHitSpell: {}"sv, cMap->PlayerOnlyHitSpell());
	}

	std::uint8_t GetNextChar(std::string_view line, std::uint32_t& index) {
		if (index < line.length())
			return line[index++];

		return 0xFF;
	}

	std::string GetNextData(std::string_view line, std::uint32_t& index, char delimeter) {
		std::uint8_t ch;
		std::string retVal = "";

		while ((ch = GetNextChar(line, index)) != 0xFF) {
			if (ch == '#') {
				if (index > 0) index--;
				break;
			}

			if (delimeter != 0 && ch == delimeter)
				break;

			retVal += static_cast<char>(ch);
		}

		Utils::Trim(retVal);
		return retVal;
	}

	RE::TESForm* GetFormFromString(std::string_view formStr) {
		auto delimiter = formStr.find('|');
		if (delimiter == std::string_view::npos)
			return nullptr;

		std::string_view pluginName = formStr.substr(0, delimiter);
		std::string_view formID = formStr.substr(delimiter + 1);

		return Utils::GetFormFromIdentifier(pluginName, formID);
	}

	void LoadConfigFile(ConfigMap* cMap, std::string_view path) {
		std::ifstream configFile(path.data());

		logger::info("Loading Config file: {}"sv, path);
		if (!configFile.is_open()) {
			logger::warn("Cannot open the config file: {}"sv, path);
			return;
		}

		std::string line;
		std::string weapFormStr, ammoFormStr, vicRaceFormStr, hitLocStr, spellFormStr, priorityStr, topPriorityOnlyCastStr;
		while (std::getline(configFile, line)) {
			Utils::Trim(line);
			if (line.empty() || line[0] == '#')
				continue;

			std::uint32_t index = 0;

			weapFormStr = GetNextData(line, index, ',');
			if (weapFormStr.empty()) {
				logger::warn("Cannot read the weapForm: {}"sv, line);
				continue;
			}

			ammoFormStr = GetNextData(line, index, ',');
			if (ammoFormStr.empty()) {
				logger::warn("Cannot read the ammoForm: {}"sv, line);
				continue;
			}

			vicRaceFormStr = GetNextData(line, index, ',');
			if (vicRaceFormStr.empty()) {
				logger::warn("Cannot read the vicRaceForm: {}"sv, line);
				continue;
			}

			hitLocStr = GetNextData(line, index, ',');
			if (hitLocStr.empty()) {
				logger::warn("Cannot read the hitLoc: {}"sv, line);
				continue;
			}

			spellFormStr = GetNextData(line, index, ',');
			if (spellFormStr.empty()) {
				logger::warn("Cannot read the spellForm: {}"sv, line);
				continue;
			}

			priorityStr = GetNextData(line, index, ',');
			if (priorityStr.empty()) {
				logger::warn("Cannot read the priority: {}"sv, line);
				continue;
			}
			
			topPriorityOnlyCastStr = GetNextData(line, index, 0);
			if (topPriorityOnlyCastStr.empty()) {
				logger::warn("Cannot read the topPriorityOnlyCast: {}"sv, line);
				continue;
			}

			RE::TESForm* weapForm = nullptr;
			if (weapFormStr != "*") {
				weapForm = GetFormFromString(weapFormStr);
				if (!weapForm) {
					logger::warn("Failed to lookup the weapForm: {}"sv, line);
					continue;
				}
			}

			RE::TESAmmo* ammoForm = nullptr;
			if (ammoFormStr != "*") {
				ammoForm = GetFormFromString(ammoFormStr)->As<RE::TESAmmo>();
				if (!ammoForm) {
					logger::warn("Failed to lookup the ammoForm: {}"sv, line);
					continue;
				}
			}

			RE::TESRace* vicRaceForm = nullptr;
			if (vicRaceFormStr != "*") {
				vicRaceForm = GetFormFromString(vicRaceFormStr)->As<RE::TESRace>();
				if (!vicRaceForm) {
					logger::warn("Failed to lookup the vicRaceForm: {}"sv, line);
					continue;
				}
			}

			std::int32_t hitLoc = -1;
			try {
				hitLoc = std::stol(hitLocStr);
			}
			catch (...) {
				logger::warn("Failed to parse the hitLoc: {}"sv, line);
				continue;
			}

			if (hitLoc < -1 || hitLoc >= HitSpell::BodyPartType::kMax) {
				logger::warn("Invalid hitLoc: {}"sv, line);
				continue;
			}

			RE::SpellItem* spellForm = GetFormFromString(spellFormStr)->As<RE::SpellItem>();
			if (!spellForm) {
				logger::warn("Failed to lookup the spellForm: {}"sv, line);
				continue;
			}

			std::uint64_t priority = 0;
			try {
				priority = std::stoll(priorityStr);
			}
			catch (...) {
				logger::warn("Failed to parse the priority: {}"sv, line);
				continue;
			}

			bool topPriorityOnlyCast = false;
			try {
				topPriorityOnlyCast = std::stol(topPriorityOnlyCastStr) == 1;
			}
			catch (...) {
				logger::warn("Failed to parse the topPriorityOnlyCast: {}"sv, line);
				continue;
			}

			HitSpell::PriorityData priorityData = { priority, topPriorityOnlyCast };

			cMap->AddSpell(weapForm, ammoForm, vicRaceForm, hitLoc, spellForm, priorityData);

			logger::info("Spell Added: Weapon[{}] Ammo[{}] Race[{}] HitLoc[{}] Spell[{}] Priority[{}] TopPriorityCastOnly[{}]"sv,
				weapFormStr, ammoFormStr, vicRaceFormStr, hitLocStr, spellFormStr, priority, topPriorityOnlyCast);
		}

		logger::info("Config file loaded: {}"sv, path);
	}

	void LoadConfigs() {
		static bool configLoaded = false;
		if (configLoaded)
			return;

		configLoaded = true;

		ConfigMap* g_configMap = ConfigMap::GetSingleton();

		LoadIni(g_configMap);

		const std::filesystem::path configDir("Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT));

		const std::regex filter(".*\\.cfg", std::regex_constants::icase);
		const std::filesystem::directory_iterator dir_iter(configDir);
		for (auto iter : dir_iter) {
			if (!std::filesystem::is_regular_file(iter.status()))
				continue;

			if (!std::regex_match(iter.path().filename().string(), filter))
				continue;

			LoadConfigFile(g_configMap, iter.path().string());
		}
	}
}
