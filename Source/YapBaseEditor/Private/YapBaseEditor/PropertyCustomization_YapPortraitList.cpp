#include "PropertyCustomization_YapPortraitList.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBaseEditorColor.h"
#include "YapBaseEditorTags.h"

void FPropertyCustomization_YapPortraitList::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> MapProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapPortraitList, Map));
	if (!MapProperty.IsValid())
	{
		return;
	}

	uint32 NumElements = 0;
	MapProperty->GetNumChildren(NumElements);

	for (uint32 i = 0; i < NumElements; ++i)
	{
		TSharedPtr<IPropertyHandle> PortraitProperty = MapProperty->GetChildHandle(i);
		if (!PortraitProperty.IsValid() || !PortraitProperty->GetKeyHandle().IsValid())
		{
			continue;
		}

		FName Name;
		PortraitProperty->GetKeyHandle()->GetValue(Name);
		const FName LeafName = YapBaseEditor::GetLeafOfTag(FGameplayTag::RequestGameplayTag(Name, false));

		StructBuilder.AddCustomRow(FText::FromName(Name))
		.NameContent()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(YapBaseColor::DarkGray)
			[
				SNew(STextBlock)
				.Text(FText::FromName(LeafName.IsNone() ? Name : LeafName))
				.Font(StructCustomizationUtils.GetRegularFont())
			]
		]
		.ValueContent()
		[
			PortraitProperty->CreatePropertyValueWidget()
		];
	}
}
