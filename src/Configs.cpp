#include "Configs.h"

#include <Windows.h>
#include <regex>
#include <fstream>
#include <algorithm>

#include "Utils.h"

namespace Configs {
	std::unordered_map<uint32_t, BodyPartSpell>* ConfigMap::GetRaceMap(uint32_t a_weapID, uint32_t a_ammoID) {
		uint64_t key = static_cast<uint64_t>(a_weapID) << 32 | a_ammoID;

		auto map_iter = spellMap.find(key);
		if (map_iter == spellMap.end())
			return nullptr;

		return &map_iter->second;
	}

	RE::SpellItem* ConfigMap::GetSpellByFormID(uint32_t a_weapID, uint32_t a_ammoID, uint32_t a_vicRaceID, int32_t a_bodyPartType) {
		auto raceMap = GetRaceMap(a_weapID, a_ammoID);
		if (raceMap) {
			RE::SpellItem* retSpell = nullptr;

			auto raceMap_iter = raceMap->find(a_vicRaceID);
			if (raceMap_iter != raceMap->end()) {
				retSpell = GetSpellByBodyPartType(raceMap_iter->second, a_bodyPartType);
				if (retSpell)
					return retSpell;
			}

			if (a_vicRaceID != 0) {
				raceMap_iter = raceMap->find(0);
				if (raceMap_iter != raceMap->end()) {
					retSpell = GetSpellByBodyPartType(raceMap_iter->second, a_bodyPartType);
					if (retSpell)
						return retSpell;
				}
			}
		}
		
		return nullptr;
	}

	RE::SpellItem* ConfigMap::GetSpellByBodyPartType(const BodyPartSpell& bps, int32_t a_bodyPartType) {
		if (a_bodyPartType == -1)
			return nullptr;

		if (bps.Part[a_bodyPartType])
			return bps.Part[a_bodyPartType];

		if (bps.Whole)
			return bps.Whole;

		return nullptr;
	}

	RE::SpellItem* ConfigMap::GetSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType) {
		if (a_bodyPartType == -1)
			return nullptr;

		uint32_t weapFormID = a_weap ? a_weap->formID : 0;
		uint32_t ammoFormID = a_ammo ? a_ammo->formID : 0;
		uint32_t raceFormID = a_vicRace ? a_vicRace->formID : 0;

		RE::SpellItem* retSpell = nullptr;

		// 1. Weap, Ammo
		retSpell = GetSpellByFormID(weapFormID, ammoFormID, raceFormID, a_bodyPartType);
		if (retSpell)
			return retSpell;

		// a_ammo가 null인 경우(근접공격) 1과 2, 3과 4가 서로 중복되므로 불필요한 체크 제거를 위해
		if (a_ammo) {
			// 2. Weap, *
			retSpell = GetSpellByFormID(weapFormID, 0, raceFormID, a_bodyPartType);
			if (retSpell)
				return retSpell;

			// 3. *, Ammo
			retSpell = GetSpellByFormID(0, ammoFormID, raceFormID, a_bodyPartType);
			if (retSpell)
				return retSpell;
		}

		// 4. *, *
		retSpell = GetSpellByFormID(0, 0, raceFormID, a_bodyPartType);
		if (retSpell)
			return retSpell;

		return nullptr;
	}

	void ConfigMap::AddSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType, RE::SpellItem* a_spell) {
		if (!a_spell)
			return;

		uint32_t weapFormID = a_weap ? a_weap->formID : 0;
		uint32_t ammoFormID = a_ammo ? a_ammo->formID : 0;
		uint32_t raceFormID = a_vicRace ? a_vicRace->formID : 0;

		uint64_t key = static_cast<uint64_t>(weapFormID) << 32 | ammoFormID;

		auto map_iter = spellMap.find(key);
		if (map_iter == spellMap.end()) {
			BodyPartSpell nBPSpell;
			std::memset(&nBPSpell, 0, sizeof(nBPSpell));
			std::unordered_map<uint32_t, BodyPartSpell> newVicMap = { std::make_pair(raceFormID, nBPSpell) };

			auto ins_res = spellMap.insert(std::make_pair(key, newVicMap));
			if (!ins_res.second)
				return;

			map_iter = ins_res.first;
		}

		auto vr_iter = map_iter->second.find(raceFormID);
		if (vr_iter == map_iter->second.end()) {
			BodyPartSpell nBPSpell;
			std::memset(&nBPSpell, 0, sizeof(nBPSpell));

			auto ins_res = map_iter->second.insert(std::make_pair(raceFormID, nBPSpell));
			if (!ins_res.second)
				return;

			vr_iter = ins_res.first;
		}

		if (a_bodyPartType == -1)
			vr_iter->second.Whole = a_spell;
		else
			vr_iter->second.Part[a_bodyPartType] = a_spell;
	}

	std::string GetIniValue(const char* section, const char* key) {
		std::string	result;
		char resultBuf[256] = { 0 };

		static const std::string& configPath = "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + ".ini";
		GetPrivateProfileStringA(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
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

		logger::info(FMT_STRING("bEnablePlayerTarget: {}"), cMap->PlayerTargetEnabled());
		logger::info(FMT_STRING("bPlayerOnlyHitSpell: {}"), cMap->PlayerOnlyHitSpell());
	}

	uint8_t GetNextChar(const std::string& line, uint32_t& index) {
		if (index < line.length())
			return line[index++];

		return 0xFF;
	}

	std::string GetNextData(const std::string& line, uint32_t& index, char delimeter) {
		uint8_t ch;
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

	RE::TESForm* GetFormFromString(const std::string& formStr) {
		auto delimiter = formStr.find('|');
		if (delimiter == std::string::npos)
			return nullptr;

		std::string pluginName = formStr.substr(0, delimiter);
		std::string formID = formStr.substr(delimiter + 1);

		return Utils::GetFormFromIdentifier(pluginName, formID);
	}

	void LoadConfigFile(ConfigMap* cMap, const std::filesystem::path& path) {
		std::ifstream configFile(path);

		logger::info(FMT_STRING("Loading Config file: {}"), path.string());
		if (!configFile.is_open()) {
			logger::warn(FMT_STRING("Cannot open the config file: {}"), path.string());
			return;
		}

		std::string line;
		std::string weapFormStr, ammoFormStr, vicRaceFormStr, hitLocStr, spellFormStr;
		while (std::getline(configFile, line)) {
			Utils::Trim(line);
			if (line.empty() || line[0] == '#')
				continue;

			uint32_t index = 0;

			weapFormStr = GetNextData(line, index, ',');
			if (weapFormStr.empty()) {
				logger::warn(FMT_STRING("Cannot read the weapForm: {}"), line);
				continue;
			}

			ammoFormStr = GetNextData(line, index, ',');
			if (ammoFormStr.empty()) {
				logger::warn(FMT_STRING("Cannot read the ammoForm: {}"), line);
				continue;
			}

			vicRaceFormStr = GetNextData(line, index, ',');
			if (vicRaceFormStr.empty()) {
				logger::warn(FMT_STRING("Cannot read the vicRaceForm: {}"), line);
				continue;
			}

			hitLocStr = GetNextData(line, index, ',');
			if (hitLocStr.empty()) {
				logger::warn(FMT_STRING("Cannot read the hitLoc: {}"), line);
				continue;
			}

			spellFormStr = GetNextData(line, index, 0);
			if (spellFormStr.empty()) {
				logger::warn(FMT_STRING("Cannot read the spellForm: {}"), line);
				continue;
			}

			RE::TESForm* weapForm = nullptr;
			if (weapFormStr != "*") {
				weapForm = GetFormFromString(weapFormStr);
				if (!weapForm) {
					logger::warn(FMT_STRING("Failed to lookup the weapForm: {}"), line);
					continue;
				}
			}

			RE::TESAmmo* ammoForm = nullptr;
			if (ammoFormStr != "*") {
				ammoForm = GetFormFromString(ammoFormStr)->As<RE::TESAmmo>();
				if (!ammoForm) {
					logger::warn(FMT_STRING("Failed to lookup the ammoForm: {}"), line);
					continue;
				}
			}

			RE::TESRace* vicRaceForm = nullptr;
			if (vicRaceFormStr != "*") {
				vicRaceForm = GetFormFromString(vicRaceFormStr)->As<RE::TESRace>();
				if (!vicRaceForm) {
					logger::warn(FMT_STRING("Failed to lookup the vicRaceForm: {}"), line);
					continue;
				}
			}

			int32_t hitLoc = std::stol(hitLocStr);
			if (hitLoc < -1 || hitLoc >= BodyPartType::kMax) {
				logger::warn(FMT_STRING("Invalid hitLoc: {}"), line);
				continue;
			}

			RE::SpellItem* spellForm = GetFormFromString(spellFormStr)->As<RE::SpellItem>();
			if (!spellForm) {
				logger::warn(FMT_STRING("Failed to lookup the spellForm: {}"), line);
				continue;
			}

			cMap->AddSpell(weapForm, ammoForm, vicRaceForm, hitLoc, spellForm);

			logger::info(FMT_STRING("Spell Added: [{}] [{}] [{}] [{}] [{}]"), weapFormStr, ammoFormStr, vicRaceFormStr, hitLocStr, spellFormStr);
		}

		logger::info(FMT_STRING("Config file loaded: {}"), path.string());
	}

	void LoadConfigs() {
		static bool configLoaded = false;
		if (configLoaded)
			return;

		ConfigMap* g_configMap = ConfigMap::GetSingleton();
		if (!g_configMap)
			return;

		LoadIni(g_configMap);

		const std::filesystem::path configDir{ "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) };

		const std::regex filter(".*\\.cfg", std::regex_constants::icase);
		const std::filesystem::directory_iterator dir_iter(configDir);
		for (auto iter : dir_iter) {
			if (!std::filesystem::is_regular_file(iter.status()))
				continue;

			if (!std::regex_match(iter.path().filename().string(), filter))
				continue;

			LoadConfigFile(g_configMap, iter.path());
		}

		configLoaded = true;
	}
}
