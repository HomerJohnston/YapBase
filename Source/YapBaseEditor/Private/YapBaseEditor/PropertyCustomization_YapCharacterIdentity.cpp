#include "PropertyCustomization_YapCharacterIdentity.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"
#include "YapBase/YapCharacterComponent.h"

#define LOCTEXT_NAMESPACE "YapBaseEditor"

void FPropertyCustomization_YapCharacterIdentity::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	CharacterProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Character));
	NameProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Name));
	AutoNameProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Auto));

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(this, &FPropertyCustomization_YapCharacterIdentity::Text_Kind)
		.Font(IDetailLayoutBuilder::GetDetailFontItalic())
	];
}

void FPropertyCustomization_YapCharacterIdentity::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (CharacterProperty.IsValid())
	{
		StructBuilder.AddProperty(CharacterProperty.ToSharedRef());
	}

	if (NameProperty.IsValid())
	{
		StructBuilder.AddProperty(NameProperty.ToSharedRef())
		.IsEnabled(TAttribute<bool>::CreateLambda([this]() { return !UsesProjectCharacter(); }));
	}

	if (AutoNameProperty.IsValid())
	{
		StructBuilder.AddProperty(AutoNameProperty.ToSharedRef())
		.Visibility(TAttribute<EVisibility>::CreateLambda([]()
		{
			return (GEditor && GEditor->IsPlaySessionInProgress()) ? EVisibility::Visible : EVisibility::Collapsed;
		}));
	}
}

bool FPropertyCustomization_YapCharacterIdentity::UsesProjectCharacter() const
{
	UObject* Character = nullptr;
	return CharacterProperty.IsValid() && CharacterProperty->GetValue(Character) == FPropertyAccess::Success && Character != nullptr;
}

FText FPropertyCustomization_YapCharacterIdentity::Text_Kind() const
{
	if (UsesProjectCharacter())
	{
		return LOCTEXT("CharacterIdentity_ProjectCharacter", "Project Character");
	}

	FName Name;
	if (NameProperty.IsValid() && NameProperty->GetValue(Name) == FPropertyAccess::Success && !Name.IsNone())
	{
		return FText::Format(LOCTEXT("CharacterIdentity_CustomID", "Custom ID: {0}"), FText::FromName(Name));
	}

	return LOCTEXT("CharacterIdentity_Auto", "Automatic ID (scripts can't name it)");
}

#undef LOCTEXT_NAMESPACE
