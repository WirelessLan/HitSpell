#include "HitSpell.h"
#include "Configs.h"

namespace HitSpell {
	void CastSpell(RE::TESObjectREFR* a_source, RE::SpellItem* a_spell, RE::TESObjectREFR* a_target, RE::TESForm* a_weap) {
		using func_t = decltype(&CastSpell);
		REL::Relocation<func_t> func{ REL::ID(833698) };
		func(a_source, a_spell, a_target, a_weap);
	}

	void ProcessHit(RE::TESObjectREFR* a_attacker, RE::TESObjectREFR* a_victim, RE::TESForm* a_attackSource, RE::TESAmmo* a_attackSourceAmmo, int32_t a_bodyPartType) {
		if (!a_attacker || !a_victim || !a_attackSource || a_bodyPartType == -1)
			return;

		Configs::ConfigMap* g_configMap = Configs::ConfigMap::GetSingleton();
		if (!g_configMap)
			return;

		RE::PlayerCharacter* g_player = RE::PlayerCharacter::GetSingleton();

		if (g_configMap->PlayerOnlyHitSpell() && (g_player && a_attacker != g_player))
			return;

		if (!g_configMap->PlayerTargetEnabled() && (g_player && a_victim == g_player))
			return;

		RE::SpellItem* hitSpell = g_configMap->GetSpell(a_attackSource, a_attackSourceAmmo, a_victim->GetVisualsRace(), a_bodyPartType);
		if (!hitSpell)
			return;

		CastSpell(a_attacker, hitSpell, a_victim, a_attackSource);
	}
}
