#include "HitSpell.h"
#include "Configs.h"

namespace HitSpell {
	void CastSpell(RE::TESObjectREFR* a_source, RE::SpellItem* a_spell, RE::TESObjectREFR* a_target, RE::TESForm* a_weap) {
		using func_t = decltype(&CastSpell);
		REL::Relocation<func_t> func{ REL::ID(833698) };
		func(a_source, a_spell, a_target, a_weap);
	}

	const std::unordered_map<uint32_t, BodyPartSpell>* GetRaceMap(const SpellMap& a_spellMap, uint32_t a_weapID, uint32_t a_ammoID) {
		uint64_t key = static_cast<uint64_t>(a_weapID) << 32 | a_ammoID;

		auto map_iter = a_spellMap.find(key);
		if (map_iter == a_spellMap.end())
			return nullptr;

		return &map_iter->second;
	}

	std::vector<SpellData> GetSpellByBodyPartType(const BodyPartSpell& bps, int32_t a_bodyPartType) {
		std::vector<SpellData> retVec;

		if (a_bodyPartType != -1 && !bps.Part[a_bodyPartType].empty())
			retVec.insert(retVec.end(), bps.Part[a_bodyPartType].begin(), bps.Part[a_bodyPartType].end());

		if (!bps.Whole.empty())
			retVec.insert(retVec.end(), bps.Whole.begin(), bps.Whole.end());

		return retVec;
	}

	std::vector<SpellData> GetSpellByFormID(const SpellMap& a_spellMap, uint32_t a_weapID, uint32_t a_ammoID, uint32_t a_vicRaceID, int32_t a_bodyPartType) {
		std::vector<SpellData> retVec;

		auto raceMap = GetRaceMap(a_spellMap, a_weapID, a_ammoID);
		if (raceMap) {
			auto raceMap_iter = raceMap->find(a_vicRaceID);
			if (raceMap_iter != raceMap->end()) {
				auto spellVec = GetSpellByBodyPartType(raceMap_iter->second, a_bodyPartType);
				if (!spellVec.empty())
					retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());
			}

			if (a_vicRaceID != 0) {
				raceMap_iter = raceMap->find(0);
				if (raceMap_iter != raceMap->end()) {
					auto spellVec = GetSpellByBodyPartType(raceMap_iter->second, a_bodyPartType);
					if (!spellVec.empty())
						retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());
				}
			}
		}

		return retVec;
	}

	std::vector<SpellData> GetSpells(const SpellMap& a_spellMap, RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType) {
		std::vector<SpellData> retVec;

		uint32_t weapFormID = a_weap ? a_weap->formID : 0;
		uint32_t ammoFormID = a_ammo ? a_ammo->formID : 0;
		uint32_t raceFormID = a_vicRace ? a_vicRace->formID : 0;

		// 1. Weap, Ammo
		auto spellVec = GetSpellByFormID(a_spellMap, weapFormID, ammoFormID, raceFormID, a_bodyPartType);
		if (!spellVec.empty())
			retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());

		// a_ammo가 null인 경우(근접공격) 1과 2, 3과 4가 서로 중복되므로 불필요한 체크 제거를 위해
		if (a_ammo) {
			// 2. Weap, *
			spellVec = GetSpellByFormID(a_spellMap, weapFormID, 0, raceFormID, a_bodyPartType);
			if (!spellVec.empty())
				retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());

			// 3. *, Ammo
			spellVec = GetSpellByFormID(a_spellMap, 0, ammoFormID, raceFormID, a_bodyPartType);
			if (!spellVec.empty())
				retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());
		}

		// 4. *, *
		spellVec = GetSpellByFormID(a_spellMap, 0, 0, raceFormID, a_bodyPartType);
		if (!spellVec.empty())
			retVec.insert(retVec.end(), spellVec.begin(), spellVec.end());

		return retVec;
	}

	class HitSpellProcessor : public F4SE::ITaskDelegate {
	public:
		HitSpellProcessor(RE::TESObjectREFR* a_attacker, RE::TESObjectREFR* a_victim, RE::TESForm* a_attackSource, RE::TESAmmo* a_attackSourceAmmo, int32_t a_bodyPartType) :
			attacker(a_attacker), victim(a_victim), attackSource(a_attackSource), attckSourceAmmo(a_attackSourceAmmo), bodyPartType(a_bodyPartType) {}

		virtual void Run() {
			Configs::ConfigMap* g_configMap = Configs::ConfigMap::GetSingleton();
			if (!g_configMap)
				return;

			RE::PlayerCharacter* g_player = RE::PlayerCharacter::GetSingleton();

			if (g_configMap->PlayerOnlyHitSpell() && (g_player && attacker != g_player))
				return;

			if (!g_configMap->PlayerTargetEnabled() && (g_player && victim == g_player))
				return;

			auto spellMap = g_configMap->GetSpellMap();

			std::vector<SpellData> hitSpells = GetSpells(spellMap, attackSource, attckSourceAmmo, victim->GetVisualsRace(), bodyPartType);
			if (hitSpells.empty())
				return;

			std::sort(hitSpells.begin(), hitSpells.end());

			bool isTopPriority = true;
			uint64_t topPriority = 0;
			for (SpellData& spellData : hitSpells) {
				if (!isTopPriority && spellData.priority.priority != topPriority && spellData.priority.topPriorityCastOnly)
					continue;

				if (isTopPriority) {
					isTopPriority = false;
					topPriority = spellData.priority.priority;
				}

				CastSpell(attacker, spellData.spell, victim, attackSource);
			}
		}

	protected:
		RE::TESObjectREFR* attacker;
		RE::TESObjectREFR* victim;
		RE::TESForm* attackSource;
		RE::TESAmmo* attckSourceAmmo;
		int32_t bodyPartType;
	};

	void ProcessHit(RE::TESObjectREFR* a_attacker, RE::TESObjectREFR* a_victim, RE::TESForm* a_attackSource, RE::TESAmmo* a_attackSourceAmmo, int32_t a_bodyPartType) {
		if (!a_attacker || !a_victim || !a_attackSource)
			return;

		const F4SE::TaskInterface* task = F4SE::GetTaskInterface();
		if (task)
			task->AddTask(new HitSpellProcessor(a_attacker, a_victim, a_attackSource, a_attackSourceAmmo, a_bodyPartType));
	}
}
