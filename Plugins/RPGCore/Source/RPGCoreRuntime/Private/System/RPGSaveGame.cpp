// Copyright G.E.FC 2023.


#include "System/RPGSaveGame.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MaxHealth, "SetByCaller.Attribute.MaxHealth");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Health, "SetByCaller.Attribute.Health");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MaxMana, "SetByCaller.Attribute.MaxMana");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Mana, "SetByCaller.Attribute.Mana");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MaxEP, "SetByCaller.Attribute.MaxEP");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_EP, "SetByCaller.Attribute.EP");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_UpgradeExp, "SetByCaller.Attribute.UpgradeExp");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Exp, "SetByCaller.Attribute.Exp");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_CostAbilityPoints, "SetByCaller.Attribute.CostAbilityPoints");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_AbilityPoints, "SetByCaller.Attribute.AbilityPoints");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Attack, "SetByCaller.Attribute.Attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Defense, "SetByCaller.Attribute.Defense");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MagicAttack, "SetByCaller.Attribute.MagicAttack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MagicDefense, "SetByCaller.Attribute.MagicDefense");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Critical, "SetByCaller.Attribute.Critical");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_CriticalRate, "SetByCaller.Attribute.CriticalRate");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Penetrated, "SetByCaller.Attribute.Penetrated");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_MagicPenetrated, "SetByCaller.Attribute.MagicPenetrated");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attribute_Speed, "SetByCaller.Attribute.Speed");

void URPGSaveGame::SaveWorld(AActor* Context)
{
	if (Context->GetWorld())
	{
		Level = MakeSoftObjectPtr<UWorld>(Context->GetWorld());
	}
	

}

