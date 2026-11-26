#include "YapCharacterThumbnailRenderer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "TextureResource.h"
#include "YapBase/YapCharacterAsset.h"
#include "YapBaseEditorColor.h"

void UYapCharacterThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	OutWidth = 128;
	OutHeight = 128;
}

void UYapCharacterThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	const UYapCharacterAsset* Character = Cast<UYapCharacterAsset>(Object);
	if (!Character || !Canvas)
	{
		return;
	}

	const UTexture2D* Texture = Character->GetCharacterPortrait(FGameplayTag::EmptyTag);
	const FVector2D Origin(X, Y);
	const FVector2D Center = Origin + FVector2D(Width * 0.5f, Height * 0.5f);

	Canvas->DrawNGon(Center, (Character->GetCharacterColor() * YapBaseColor::DimGray).ToFColor(true), 16, Width);

	FCanvasBoxItem Box(Origin, FVector2D(Width, Height));
	Box.SetColor(Character->GetCharacterColor() * YapBaseColor::Gray);
	Box.LineThickness = 0.0625f * Width;
	Canvas->DrawItem(Box);

	if (Texture && Texture->GetResource())
	{
		Canvas->DrawTile(X + Width * 0.0625f, Y + Height * 0.0625f, Width * 0.875f, Height * 0.875f, 0, 0, 1, 1, YapBaseColor::White_SemiTrans, Texture->GetResource(), true);
		return;
	}

	// One item per line: canvas text can't be centre-justified across lines. Not localized (it's a thumbnail).
	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", 14);
	const TCHAR* Lines[] = { TEXT("Default"), TEXT("portrait"), TEXT("missing") };
	for (int32 i = 0; i < 3; ++i)
	{
		FCanvasTextItem Text(Center + FVector2D(0, Height * 0.2f * (i - 1)), FText::FromString(Lines[i]), GEngine->GetLargeFont(), YapBaseColor::DimWhite);
		Text.bCentreX = true;
		Text.bCentreY = true;
		Text.SlateFontInfo = Font;
		Canvas->DrawItem(Text);
	}
}

EThumbnailRenderFrequency UYapCharacterThumbnailRenderer::GetThumbnailRenderFrequency(UObject* Object) const
{
	return EThumbnailRenderFrequency::OnAssetSave;
}

bool UYapCharacterThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Cast<UYapCharacterAsset>(Object) != nullptr;
}
