ProjectSettings:

1.Assetmanager->+GamefeatureData

2.Map->GameInstance

3.Common Input Settings->InputData

4.Common Input Settings->Windows->KBControllerData/GPControllerData

5.Common Loading Screen

6.GeneralSettings->SoundMix/ Save/Input

7.Enhanced Input->UserSettingsClass

8.GeneralSettings->GameViewportClientClass/LocalPlayer

9.GameFeatures->Game Feature Project Policy Class:RPGGameFeaturePolicy

10.GameplayAbilities Settings->Ability System Globals Class:RPGAbilitySystemGlobals/

11.GameplayAbilities Settings->Global GameplayCue Manager Class:RPGGameplayCueManager

12.InputComponent->GASInputComponent

13.Add GameplayTagDataTable

14.Collision->TraceChannel:+TraceChannel\_Damage/   CharacterMesh

15.Collision->Object:+Interaction  /New Preset:Interaction



DefaultGame.ini:

\[/Script/RPGCoreRuntime.LyraUIManagerSubsystem]

DefaultUIPolicyClass=/RPGCore/UI/B\_RPGUIPolicy.B\_RPGUIPolicy\_C



DefaultEngine.ini:

\[SystemSettings]

net.SubObjects.DefaultUseSubObjectReplicationList=1

;net.Iris.UseIrisReplication=0  //5.7 close

