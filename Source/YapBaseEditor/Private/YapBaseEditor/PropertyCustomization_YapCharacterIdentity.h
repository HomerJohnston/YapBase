#pragma once

#include "IPropertyTypeCustomization.h"

/**
 * A character component's Identity: the header says which kind it is (project character, custom ID, automatic), and
 * the custom ID is only editable without a project character. The automatic ID is only shown while playing.
 */
class FPropertyCustomization_YapCharacterIdentity : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapCharacterIdentity>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	bool UsesProjectCharacter() const;
	FText Text_Kind() const;

	TSharedPtr<IPropertyHandle> CharacterProperty;
	TSharedPtr<IPropertyHandle> NameProperty;
	TSharedPtr<IPropertyHandle> AutoNameProperty;
};
