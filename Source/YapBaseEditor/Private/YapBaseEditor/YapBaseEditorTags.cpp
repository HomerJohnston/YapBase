#include "YapBaseEditorTags.h"

#include "SGameplayTagPicker.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

void YapBaseEditor::OpenGameplayTagsEditor(const FGameplayTagContainer& Roots)
{
	FGameplayTagManagerWindowArgs Args;
	Args.bRestrictedTags = false;

	if (Roots.Num() > 0)
	{
		const FString RootList = FString::JoinBy(Roots, TEXT(", "), [](const FGameplayTag& Tag) { return Tag.ToString(); });
		Args.Title = FText::Format(LOCTEXT("GameplayTags_WindowTitle_Filtered", "Tags ({0})"), FText::FromString(RootList));
		Args.Filter = FString::JoinBy(Roots, TEXT(","), [](const FGameplayTag& Tag) { return Tag.ToString(); });
	}
	else
	{
		Args.Title = LOCTEXT("GameplayTags_WindowTitle", "Gameplay Tags");
	}

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
}

#undef LOCTEXT_NAMESPACE
