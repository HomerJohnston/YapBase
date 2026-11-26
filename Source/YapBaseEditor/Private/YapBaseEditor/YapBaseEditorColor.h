#pragma once

#include "Math/Color.h"

/** The few editor colours YapBase uses (taken from Yap's YapEditorColor). */
namespace YapBaseColor
{
	inline const FLinearColor White(1.0f, 1.0f, 1.0f, 1.0f);
	inline const FLinearColor White_SemiTrans(1.0f, 1.0f, 1.0f, 0.9f);
	inline const FLinearColor DimWhite(0.9f, 0.9f, 0.9f, 1.0f);
	inline const FLinearColor LightYellow(0.9f, 0.85f, 0.25f, 1.0f);
	inline const FLinearColor OrangeRed(0.92f, 0.07f, 0.02f, 1.0f);
	inline const FLinearColor Orange(0.9f, 0.4f, 0.02f, 1.0f);
	inline const FLinearColor Gray(0.2f, 0.2f, 0.2f, 1.0f);
	inline const FLinearColor DimGray(0.1f, 0.1f, 0.1f, 1.0f);
	inline const FLinearColor DarkGray(0.03f, 0.03f, 0.03f, 1.0f);
	inline const FLinearColor Noir_Trans(0.005f, 0.005f, 0.005f, 0.7f);

	/** Yap's broker blueprint colour. */
	inline const FLinearColor Broker(0.39f, 0.5f, 1.0f, 1.0f);

	/** Yap's node config colour, used for speech configs. */
	inline const FLinearColor SpeechConfig(0.3f, 0.15f, 0.4f, 1.0f);

	/** Yap's character asset colour: its broker blue, darkened by half. */
	inline const FLinearColor CharacterAsset(0.195f, 0.25f, 0.5f, 1.0f);
}
