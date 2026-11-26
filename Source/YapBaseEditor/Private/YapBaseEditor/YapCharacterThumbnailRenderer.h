#pragma once

#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "YapCharacterThumbnailRenderer.generated.h"

/** A Yap character's thumbnail: its default portrait in a frame of its colour ("Default portrait missing" without one). */
UCLASS()
class UYapCharacterThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	virtual EThumbnailRenderFrequency GetThumbnailRenderFrequency(UObject* Object) const override;
	virtual bool CanVisualizeAsset(UObject* Object) override;
};
