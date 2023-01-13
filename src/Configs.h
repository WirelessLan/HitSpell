#pragma once

#include "HitSpell.h"

namespace Configs {
	class ConfigMap {
	public:
		ConfigMap() : bEnablePlayerTarget(false), bPlayerOnlyHitSpell(false) {}

		const HitSpell::SpellMap& GetSpellMap();

		void AddSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType, RE::SpellItem* a_spell, HitSpell::PriorityData& priority);

		bool PlayerTargetEnabled() { return bEnablePlayerTarget; }
		bool PlayerOnlyHitSpell() { return bPlayerOnlyHitSpell; }

		void SetPlayerTarget(bool enabled) { bEnablePlayerTarget = enabled; }
		void SetPlayerOnlyHitSpell(bool enabled) { bPlayerOnlyHitSpell = enabled; }

		static ConfigMap* GetSingleton() {
			static ConfigMap configMap;
			return &configMap;
		}

	protected:
		HitSpell::SpellMap spellMap;
		bool bEnablePlayerTarget;
		bool bPlayerOnlyHitSpell;
	};

	void LoadConfigs();
}
