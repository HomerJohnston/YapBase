#pragma once

#include "IDetailCustomization.h"

class UYapSpeechConfig;

/** A speech config's details: adds "Edit Mood Tags" (the gameplay tag manager, filtered to its mood root) under Moods. */
class FDetailCustomization_YapSpeechConfig : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FDetailCustomization_YapSpeechConfig()); }

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<UYapSpeechConfig> Config;
};
