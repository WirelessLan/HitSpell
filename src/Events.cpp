#include "Events.h"
#include "HitSpell.h"

namespace Events {
	struct DamageImpactData {
		RE::NiPoint4 hitPos;
		RE::NiPoint4 hitDirection;
		RE::NiPoint4 projectileDir;
		RE::bhkNPCollisionObject* collisionObj;
	};
	static_assert(sizeof(DamageImpactData) == 0x38);

	struct VATSCommand;

	class HitData {
	public:
		DamageImpactData impactData;
		int8_t gap38[8];
		RE::ObjectRefHandle attackerHandle;
		RE::ObjectRefHandle victimHandle;
		RE::ObjectRefHandle sourceHandle;
		int8_t gap4C[4];
		RE::BGSAttackData* attackData;
		RE::BGSObjectInstance source;
		RE::MagicItem* effect;
		RE::SpellItem* spellItem;
		VATSCommand* VATSCommand;
		RE::TESAmmo* ammo;
		RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>* damageTypes;
		float calculatedBaseDamage;
		float baseDamage;
		float totalDamage;
		float blockedDamage;
		float blockMult;
		float reducedDamage;
		float field_A8;
		float blockStaggerMult;
		float sneakAttackMult;
		float field_B4;
		float field_B8;
		float field_BC;
		float criticalDamageMult;
		uint32_t flags;
		RE::BGSEquipIndex equipIndex;
		uint32_t materialType;
		int32_t bodypartType;
		int8_t gapD4[4];
	};
	static_assert(sizeof(HitData) == 0xD8);

	class TESHitEvent {
	public:
		HitData hitdata;
		int8_t gapD8[8];
		RE::TESObjectREFR* victim;
		RE::TESObjectREFR* attacker;
		RE::BSFixedString matName;
		uint32_t sourceFormID;
		uint32_t projFormID;
		bool hasHitData;
		int8_t gapD1[7];

		static RE::BSTEventSource<TESHitEvent>* GetEventSource() {
			REL::Relocation<RE::BSTEventSource<TESHitEvent>*> evnSrc{ REL::ID(989868) };
			return evnSrc.get();
		}
	};
	static_assert(sizeof(TESHitEvent) == 0x108);

	RE::TESObjectREFR* GetObjectRefFromHandle(const RE::ObjectRefHandle& a_handle) {
		auto ptr = a_handle.get();
		if (!ptr) return nullptr;
		return ptr.get();
	}

	class HitEventHandler : public RE::BSTEventSink<TESHitEvent> {
	public:
		virtual RE::BSEventNotifyControl ProcessEvent(const TESHitEvent& a_evn, RE::BSTEventSource<TESHitEvent>*) {
			if (a_evn.hasHitData) {
				RE::TESObjectREFR* attacker = a_evn.attacker ? a_evn.attacker : GetObjectRefFromHandle(a_evn.hitdata.attackerHandle);
				RE::TESObjectREFR* victim = a_evn.victim ? a_evn.victim : GetObjectRefFromHandle(a_evn.hitdata.victimHandle);
				HitSpell::ProcessHit(attacker, victim, a_evn.hitdata.source.object, a_evn.hitdata.ammo, a_evn.hitdata.bodypartType);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void Install() {
		auto evtSrc = TESHitEvent::GetEventSource();
		if (!evtSrc) {
			logger::critical("Failed to get the HitEvent source...");
			return;
		}
		
		evtSrc->RegisterSink(new HitEventHandler());
		logger::info("HitEventHandler registered...");
	}
}
