#pragma once

namespace HitSpell {
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

	struct PriorityData {
		uint64_t priority;
		bool topPriorityCastOnly;
	};

	struct SpellData {
		RE::SpellItem* spell;
		PriorityData priority;

		bool operator<(const SpellData& a) const	{
			return priority.priority > a.priority.priority;
		}
	};

	struct BodyPartSpell {
		std::vector<SpellData> Whole;
		std::vector<SpellData> Part[BodyPartType::kMax];
	};

	using SpellMap = std::unordered_map <uint64_t, std::unordered_map<uint32_t, BodyPartSpell>>;

	void ProcessHit(RE::TESObjectREFR* a_attacker, RE::TESObjectREFR* a_victim, RE::TESForm* a_attackSource, RE::TESAmmo* a_attackSourceAmmo, int32_t a_bodyPartType);
}
