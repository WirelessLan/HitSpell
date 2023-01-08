#pragma once

namespace Configs {
	enum BodyPartType : int32_t {
		kNone = -1,
		kTorse,
		kHead1,
		kEye,
		kLookAt,
		kFlyGrab,
		kHead2,
		kLeftArm1,
		kLeftArm2,
		kRightArm1,
		kRightArm2,
		kLeftLeg1,
		kLeftLeg2,
		kLeftLeg3,
		kRightLeg1,
		kRightLeg2,
		kRightLeg3,
		kBrain,
		kWeapon,
		kRoot,
		kCOM,
		kPelvis,
		kCamera,
		kOffsetRoot,
		kLeftFoot,
		kRightFoot,
		kFaceTargetSource,

		kMax
	};

	struct BodyPartSpell {
		RE::SpellItem* Whole;
		RE::SpellItem* Part[BodyPartType::kMax];
	};

	class ConfigMap {
	public:
		ConfigMap() : bEnablePlayerTarget(false), bPlayerOnlyHitSpell(false) {}

		RE::SpellItem* GetSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType);

		void AddSpell(RE::TESForm* a_weap, RE::TESAmmo* a_ammo, RE::TESRace* a_vicRace, int32_t a_bodyPartType, RE::SpellItem* a_spell);

		bool PlayerTargetEnabled() { return bEnablePlayerTarget; }
		bool PlayerOnlyHitSpell() { return bPlayerOnlyHitSpell; }

		void SetPlayerTarget(bool enabled) { bEnablePlayerTarget = enabled; }
		void SetPlayerOnlyHitSpell(bool enabled) { bPlayerOnlyHitSpell = enabled; }

		static ConfigMap* GetSingleton() {
			static ConfigMap configMap;
			return &configMap;
		}

	protected:
		std::unordered_map <uint64_t, std::unordered_map<uint32_t, BodyPartSpell>> spellMap;
		bool bEnablePlayerTarget;
		bool bPlayerOnlyHitSpell;

	private:
		std::unordered_map<uint32_t, BodyPartSpell>* GetRaceMap(uint32_t a_weapID, uint32_t a_ammoID);
		RE::SpellItem* GetSpellByFormID(uint32_t a_weapID, uint32_t a_ammoID, uint32_t a_vicRaceID, int32_t a_bodyPartType);
		RE::SpellItem* GetSpellByBodyPartType(const BodyPartSpell& bps, int32_t a_bodyPartType);
	};

	void LoadConfigs();
}
