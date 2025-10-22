// Copyright G.E.FC 2023.


#include "Settings/RPGCoreSettings.h"


const URPGCoreSettings* URPGCoreSettings::GetRPGCoreSettingsInstance()
{
	const URPGCoreSettings* Setting = GetDefault<URPGCoreSettings>();

	return Setting;
}
