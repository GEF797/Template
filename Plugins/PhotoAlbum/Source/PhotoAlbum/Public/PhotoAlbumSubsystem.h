// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PhotoAlbumSubsystem.generated.h"

/**
 *  UPhotoAlbumSubsystem
 *  Local Player subsystem to handle photo file management
 */
UCLASS()
class PHOTOALBUM_API UPhotoAlbumSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/** Gets the filenames for all photos in the album path */
	UFUNCTION(BlueprintCallable, Category="Titan|Album")
	void GetAllPhotoFilenames(const FString& AlbumFolder, TArray<FString>& Files);

	/** Attempts to load a photo with the given filename and returns it as a Texture2D */
	UFUNCTION(BlueprintCallable, Category="Titan|Album")
	bool LoadPhoto(const FString& Filename, const FString& AlbumFolder, UTexture2D*& PhotoTexture);

	/** Attempts to save a render target as a photo in the album path. Filename will be date-time coded */
	UFUNCTION(BlueprintCallable, Category="Titan|Album")
	bool SavePhotoFromRenderTarget(UTextureRenderTarget2D* RenderTarget, const FString& AlbumFolder, const FString& FilePrefix, FString& SavedFileName);
	
	/** Attempts to delete a photo file on the given path. */
	UFUNCTION(BlueprintCallable, Category="Titan|Album")
	bool DeletePhotoFromAlbum(const FString& Filename, const FString& AlbumFolder);

private:

	/** Utility function to build an album path string */
	FString BuildFolderPath(const FString& AlbumFolder) const;

	/** Utility function to build a full path from a filename and an album folder path */
	FString BuildFullPath(const FString& Filename, const FString& AlbumFolder) const;
	
};
