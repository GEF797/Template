// Fill out your copyright notice in the Description page of Project Settings.


#include "UISettings.h"

const UUISettings* UUISettings::GetUISettingsInstance()
{
	const UUISettings* Setting = GetDefault<UUISettings>();

	return Setting;
}
