#pragma once

#include "EditorUndoClient.h"
#include "IDetailCustomization.h"
#include "Layout/Visibility.h"

class IDetailCategoryBuilder;
class IPropertyHandle;
class IPropertyHandleMap;
class UYapCharacterAsset;
struct FGameplayTag;

/**
 * A Yap character's details: the default portrait with the character's other settings, then one group of portraits
 * per mood root (from the project's speech configs), with "Click to refresh" to bring the lists up to date when mood
 * roots or tags change.
 */
class FDetailCustomization_YapCharacterAsset : public IDetailCustomization, public FEditorUndoClient
{
public:
	FDetailCustomization_YapCharacterAsset();
	virtual ~FDetailCustomization_YapCharacterAsset() override;

	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FDetailCustomization_YapCharacterAsset()); }

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	bool CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder);
	void SortCategories(IDetailLayoutBuilder& DetailBuilder) const;
	void AddBottomControls(IDetailCategoryBuilder& Category) const;

	EVisibility Visibility_MoodTagsOutOfDateWarning() const;
	FText Text_PortraitsListHint() const;
	void OnClicked_RefreshMoodTagsButton() const;

	void RefreshList(TSharedPtr<IPropertyHandle> MoodRootProperty, TSharedPtr<IPropertyHandle> ListProperty) const;
	void AddEntryToMap(TSharedPtr<IPropertyHandleMap> Map, const FGameplayTag& NewKey) const;
	void RemoveEntryFromMap(TSharedPtr<IPropertyHandleMap> Map, const FName& DeadKey) const;
	void RefreshDetails() const;

	TWeakObjectPtr<UYapCharacterAsset> CharacterBeingCustomized;
	TWeakPtr<IDetailLayoutBuilder> CachedDetailBuilder;
	TSharedPtr<IPropertyHandle> DefaultPortraitProperty;
	TSharedPtr<IPropertyHandle> PortraitsProperty;
	FDelegateHandle TagTreeChangedHandle;
};
