#include "DetailCustomization_YapCharacterAsset.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "GameplayTagsManager.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBase/YapMoodTags.h"
#include "YapBaseEditorColor.h"
#include "YapBaseEditorTags.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

FDetailCustomization_YapCharacterAsset::FDetailCustomization_YapCharacterAsset()
{
	TagTreeChangedHandle = UGameplayTagsManager::OnEditorRefreshGameplayTagTree.AddRaw(this, &FDetailCustomization_YapCharacterAsset::RefreshDetails);

	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}
}

FDetailCustomization_YapCharacterAsset::~FDetailCustomization_YapCharacterAsset()
{
	UGameplayTagsManager::OnEditorRefreshGameplayTagTree.Remove(TagTreeChangedHandle);

	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

void FDetailCustomization_YapCharacterAsset::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	CachedDetailBuilder = DetailBuilder;
	CustomizeDetails(*DetailBuilder);
}

void FDetailCustomization_YapCharacterAsset::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (!CacheEditedCharacterAsset(DetailBuilder))
	{
		return;
	}

	DefaultPortraitProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, Portrait));
	DefaultPortraitProperty->MarkHiddenByCustomization();

	PortraitsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacterAsset, PortraitsMap));
	PortraitsProperty->MarkHiddenByCustomization();

	SortCategories(DetailBuilder);

	IDetailCategoryBuilder& CharacterCategory = DetailBuilder.EditCategory("Character");
	CharacterCategory.AddProperty(DefaultPortraitProperty);

	IDetailCategoryBuilder& PortraitsCategory = DetailBuilder.EditCategory("Portraits", LOCTEXT("PortraitsCategory_Title", "Portraits"));

	TSharedPtr<IPropertyHandleMap> PortraitsMap = PortraitsProperty->AsMap();
	uint32 NumMoodRoots = 0;
	PortraitsMap->GetNumElements(NumMoodRoots);

	for (uint32 i = 0; i < NumMoodRoots; ++i)
	{
		TSharedPtr<IPropertyHandle> ListElement = PortraitsProperty->GetChildHandle(i);
		if (!ListElement.IsValid())
		{
			continue;
		}

		FName ListKeyName;
		ListElement->GetKeyHandle()->GetValue(ListKeyName);
		const FGameplayTag MoodRoot = FGameplayTag::RequestGameplayTag(ListKeyName, false);

		const FText ListHeading = MoodRoot.IsValid()
			? FText::FromName(ListKeyName)
			: FText::Format(LOCTEXT("InvalidMoodTag", "INVALID: {0}"), FText::FromName(ListKeyName));

		IDetailGroup& Group = PortraitsCategory.AddGroup(ListKeyName, ListHeading, false, true);
		Group.HeaderRow()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SColorBlock)
				.Color(YapBaseColor::Noir_Trans)
			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.Padding(8)
			[
				SNew(STextBlock)
				.Text(ListHeading)
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
			]
		];

		// FYapPortraitList only has one property: its map.
		TSharedPtr<IPropertyHandle> MoodMap = ListElement->GetChildHandle(0);
		if (!MoodMap.IsValid())
		{
			continue;
		}

		uint32 NumMoods = 0;
		MoodMap->GetNumChildren(NumMoods);

		for (uint32 j = 0; j < NumMoods; ++j)
		{
			TSharedPtr<IPropertyHandle> MoodElement = MoodMap->GetChildHandle(j);
			if (!MoodElement.IsValid())
			{
				continue;
			}

			FName MoodName;
			MoodElement->GetKeyHandle()->GetValue(MoodName);
			const FGameplayTag MoodTag = FGameplayTag::RequestGameplayTag(MoodName, false);

			TSharedRef<STextBlock> NameWidget = MoodTag.IsValid()
				? SNew(STextBlock)
					.Text(FText::FromName(YapBaseEditor::GetLeafOfTag(MoodTag)))
					.ToolTipText(FText::FromName(MoodName))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				: SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("InvalidMoodTag", "INVALID: {0}"), FText::FromName(MoodName)))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(YapBaseColor::Orange);

			Group.AddWidgetRow()
			.NameContent()
			[
				NameWidget
			]
			.ValueContent()
			.MinDesiredWidth(250)
			[
				MoodElement->CreatePropertyValueWidget()
			];
		}
	}

	AddBottomControls(PortraitsCategory);
}

bool FDetailCustomization_YapCharacterAsset::CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	CharacterBeingCustomized = (Objects.Num() == 1) ? Cast<UYapCharacterAsset>(Objects[0].Get()) : nullptr;
	return CharacterBeingCustomized.IsValid();
}

void FDetailCustomization_YapCharacterAsset::SortCategories(IDetailLayoutBuilder& DetailBuilder) const
{
	DetailBuilder.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& CategoryMap)
	{
		const TMap<FName, int32> SortOrder =
		{
			{ TEXT("Character"), 0 },
			{ TEXT("Portraits"), 1 },
		};

		for (const TPair<FName, int32>& SortPair : SortOrder)
		{
			if (IDetailCategoryBuilder* const* Category = CategoryMap.Find(SortPair.Key))
			{
				(*Category)->SetSortOrder(SortPair.Value);
			}
		}
	});
}

EVisibility FDetailCustomization_YapCharacterAsset::Visibility_MoodTagsOutOfDateWarning() const
{
	return (CharacterBeingCustomized.IsValid() && CharacterBeingCustomized->GetPortraitsOutOfDate()) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FDetailCustomization_YapCharacterAsset::Text_PortraitsListHint() const
{
	const FGameplayTagContainer MoodTagRoots = Yap::GetMoodTagRoots();
	if (MoodTagRoots.Num() == 0)
	{
		return LOCTEXT("CharacterPortraits_MoodTagRootsEmpty", "To use mood portraits, set a Mood Tags Root in a Yap Speech Config asset.");
	}

	if (Yap::GetAllMoodTagsUnder(MoodTagRoots).Num() == 0)
	{
		return LOCTEXT("CharacterPortraits_MoodTagsEmpty", "Add gameplay tags under your mood tags root for each mood.");
	}

	return FText::GetEmpty();
}

void FDetailCustomization_YapCharacterAsset::OnClicked_RefreshMoodTagsButton() const
{
	if (!CharacterBeingCustomized.IsValid())
	{
		return;
	}

	{
		FScopedTransaction Transaction(LOCTEXT("RefreshCharacterPortraitList_Transaction", "Refresh character portrait list"));
		CharacterBeingCustomized->Modify();

		TSharedPtr<IPropertyHandleMap> PortraitsMap = PortraitsProperty->AsMap();
		const FGameplayTagContainer MoodRoots = Yap::GetMoodTagRoots();
		FGameplayTagContainer ExistingMoodRoots;

		// 1. Remove lists whose mood root is gone (or is no longer used by any speech config). Restart after each removal.
		bool bLoopAgain = true;
		while (bLoopAgain)
		{
			bLoopAgain = false;

			uint32 NumLists = 0;
			PortraitsMap->GetNumElements(NumLists);
			for (uint32 i = 0; i < NumLists; ++i)
			{
				TSharedPtr<IPropertyHandle> ListElement = PortraitsProperty->GetChildHandle(i);
				TSharedPtr<IPropertyHandle> Key = ListElement.IsValid() ? ListElement->GetKeyHandle() : nullptr;
				if (!Key.IsValid())
				{
					continue;
				}

				FName ListKey;
				Key->GetValue(ListKey);
				const FGameplayTag ListTag = FGameplayTag::RequestGameplayTag(ListKey, false);

				if (!ListTag.IsValid() || !MoodRoots.HasTagExact(ListTag))
				{
					PortraitsMap->DeleteItem(i);
					bLoopAgain = true;
					break;
				}

				ExistingMoodRoots.AddTag(ListTag);
			}
		}

		// 2. Add lists for new mood roots.
		for (const FGameplayTag& Root : MoodRoots)
		{
			if (!ExistingMoodRoots.HasTagExact(Root))
			{
				AddEntryToMap(PortraitsMap, Root);
			}
		}

		// 3. Bring each list's moods up to date.
		uint32 NumLists = 0;
		PortraitsMap->GetNumElements(NumLists);
		for (uint32 i = 0; i < NumLists; ++i)
		{
			TSharedPtr<IPropertyHandle> ListElement = PortraitsProperty->GetChildHandle(i);
			if (ListElement.IsValid() && ListElement->GetKeyHandle().IsValid())
			{
				// FYapPortraitList only has one property: its map.
				RefreshList(ListElement->GetKeyHandle(), ListElement->GetChildHandle(0));
			}
		}

		auto SortNames = [](const FName& A, const FName& B) { return A.Compare(B) < 0; };
		CharacterBeingCustomized->PortraitsMap.KeySort(SortNames);
		for (TPair<FName, FYapPortraitList>& Pair : CharacterBeingCustomized->PortraitsMap)
		{
			Pair.Value.Map.KeySort(SortNames);
		}
	}

	RefreshDetails();
}

void FDetailCustomization_YapCharacterAsset::RefreshList(TSharedPtr<IPropertyHandle> MoodRootProperty, TSharedPtr<IPropertyHandle> ListProperty) const
{
	if (!ListProperty.IsValid())
	{
		return;
	}

	TSharedPtr<IPropertyHandleMap> Map = ListProperty->AsMap();
	if (!Map.IsValid())
	{
		return;
	}

	FName MoodRootName;
	MoodRootProperty->GetValue(MoodRootName);
	const FGameplayTag MoodRoot = FGameplayTag::RequestGameplayTag(MoodRootName, false);
	if (!MoodRoot.IsValid())
	{
		return;
	}

	const FGameplayTagContainer RequiredMoodTags = Yap::GetAllMoodTagsUnder(MoodRoot);

	TSet<FName> ExistingMoodNames;
	uint32 NumMoods = 0;
	ListProperty->GetNumChildren(NumMoods);
	for (uint32 j = 0; j < NumMoods; ++j)
	{
		TSharedPtr<IPropertyHandle> MoodElement = ListProperty->GetChildHandle(j);
		if (MoodElement.IsValid() && MoodElement->GetKeyHandle().IsValid())
		{
			FName MoodName;
			MoodElement->GetKeyHandle()->GetValue(MoodName);
			ExistingMoodNames.Add(MoodName);
		}
	}

	// Remove moods that are gone.
	for (const FName& ExistingName : ExistingMoodNames)
	{
		if (!RequiredMoodTags.HasTagExact(FGameplayTag::RequestGameplayTag(ExistingName, false)))
		{
			RemoveEntryFromMap(Map, ExistingName);
		}
	}

	// Add new moods.
	for (const FGameplayTag& Required : RequiredMoodTags)
	{
		if (!ExistingMoodNames.Contains(Required.GetTagName()))
		{
			AddEntryToMap(Map, Required);
		}
	}
}

void FDetailCustomization_YapCharacterAsset::AddEntryToMap(TSharedPtr<IPropertyHandleMap> Map, const FGameplayTag& NewKey) const
{
	// Adding makes a "None" entry; find it and give it the key.
	Map->AddItem();

	uint32 NumElements = 0;
	Map->GetNumElements(NumElements);
	for (uint32 i = 0; i < NumElements; ++i)
	{
		TSharedPtr<IPropertyHandle> Element = Map->GetElement(i);
		TSharedPtr<IPropertyHandle> Key = Element.IsValid() ? Element->GetKeyHandle() : nullptr;
		if (!Key.IsValid())
		{
			continue;
		}

		FName KeyName;
		Key->GetValue(KeyName);
		if (!FGameplayTag::RequestGameplayTag(KeyName, false).IsValid())
		{
			Key->SetValueFromFormattedString(NewKey.GetTagName().ToString());
			break;
		}
	}
}

void FDetailCustomization_YapCharacterAsset::RemoveEntryFromMap(TSharedPtr<IPropertyHandleMap> Map, const FName& DeadKey) const
{
	uint32 NumElements = 0;
	Map->GetNumElements(NumElements);
	for (uint32 i = 0; i < NumElements; ++i)
	{
		TSharedPtr<IPropertyHandle> Element = Map->GetElement(i);
		TSharedPtr<IPropertyHandle> Key = Element.IsValid() ? Element->GetKeyHandle() : nullptr;
		if (!Key.IsValid())
		{
			continue;
		}

		FName KeyName;
		Key->GetValue(KeyName);
		if (KeyName == DeadKey)
		{
			Map->DeleteItem(i);
			break;
		}
	}
}

void FDetailCustomization_YapCharacterAsset::RefreshDetails() const
{
	if (TSharedPtr<IDetailLayoutBuilder> Layout = CachedDetailBuilder.Pin())
	{
		Layout->ForceRefreshDetails();
	}
}

void FDetailCustomization_YapCharacterAsset::AddBottomControls(IDetailCategoryBuilder& Category) const
{
	Category.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 12, 0, 8)
		[
			SNew(SBox)
			.Visibility_Lambda([]() { return Yap::GetMoodTagRoots().Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Visibility(this, &FDetailCustomization_YapCharacterAsset::Visibility_MoodTagsOutOfDateWarning)
					.Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh", "Portraits list is out of date. "))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(YapBaseColor::OrangeRed)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SHyperlink)
					.Text(LOCTEXT("CharacterPortraits_PerformMoodTagsRefresh", "Click to refresh."))
					.ToolTipText(LOCTEXT("RefreshMoodTags_ToolTip", "Removes portrait entries for mood tags the project's speech configs no longer use, adds missing ones, and sorts the lists."))
					.OnNavigate(this, &FDetailCustomization_YapCharacterAsset::OnClicked_RefreshMoodTagsButton)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(4)
		[
			SNew(STextBlock)
			.Text(this, &FDetailCustomization_YapCharacterAsset::Text_PortraitsListHint)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(YapBaseColor::LightYellow)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(4, 4, 4, 12)
		[
			SNew(SHyperlink)
			.Visibility_Lambda([]() { return Yap::GetMoodTagRoots().Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; })
			.Text(LOCTEXT("CharacterPortraits_EditMoodTags", "Edit Mood Tags"))
			.OnNavigate_Lambda([]() { YapBaseEditor::OpenGameplayTagsEditor(Yap::GetMoodTagRoots()); })
		]
	];
}

void FDetailCustomization_YapCharacterAsset::PostUndo(bool bSuccess)
{
	RefreshDetails();
}

void FDetailCustomization_YapCharacterAsset::PostRedo(bool bSuccess)
{
	RefreshDetails();
}

#undef LOCTEXT_NAMESPACE
