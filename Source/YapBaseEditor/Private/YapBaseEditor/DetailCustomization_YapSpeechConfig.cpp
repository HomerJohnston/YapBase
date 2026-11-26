#include "DetailCustomization_YapSpeechConfig.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBox.h"
#include "YapBase/YapSpeechConfig.h"
#include "YapBaseEditorTags.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

void FDetailCustomization_YapSpeechConfig::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	Config = (Objects.Num() == 1) ? Cast<UYapSpeechConfig>(Objects[0].Get()) : nullptr;
	if (!Config.IsValid())
	{
		return;
	}

	IDetailCategoryBuilder& Moods = DetailBuilder.EditCategory("Moods");
	Moods.AddCustomRow(LOCTEXT("EditMoodTags", "Edit Mood Tags"))
	.Visibility(TAttribute<EVisibility>::CreateLambda([this]()
	{
		return (Config.IsValid() && !Config->bDisableMoodTags) ? EVisibility::Visible : EVisibility::Collapsed;
	}))
	[
		SNew(SBox)
		.HAlign(HAlign_Center)
		.Padding(0, 6)
		[
			SNew(SHyperlink)
			.Text(LOCTEXT("EditMoodTags", "Edit Mood Tags"))
			.ToolTipText(LOCTEXT("EditMoodTags_ToolTip", "Open the gameplay tag manager, showing the tags under this config's Mood Tags Root (all tags if it's not set). Each child tag of the root is a mood."))
			.OnNavigate_Lambda([this]()
			{
				FGameplayTagContainer Roots;
				if (Config.IsValid() && Config->MoodTagsRoot.IsValid())
				{
					Roots.AddTag(Config->MoodTagsRoot);
				}
				YapBaseEditor::OpenGameplayTagsEditor(Roots);
			})
		]
	];
}

#undef LOCTEXT_NAMESPACE
