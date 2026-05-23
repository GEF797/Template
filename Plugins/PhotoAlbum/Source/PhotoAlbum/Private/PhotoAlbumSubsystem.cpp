// Copyright Epic Games, Inc. All Rights Reserved.


#include "PhotoAlbumSubsystem.h"
#include "ImageUtils.h"
#include "ImageCore.h"
#include "HAL/FileManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Misc/Paths.h"

static FString PhotoFileExtension = "png";

void UPhotoAlbumSubsystem::GetAllPhotoFilenames(const FString& AlbumFolder, TArray<FString>& Files)
{
	// concatenate the full path to the photo directory
	const FString FullDir = BuildFolderPath(AlbumFolder);

	UE_LOG(LogTemp, Log, TEXT("Finding %s album files in path: %s"), *PhotoFileExtension, *FullDir);

	// get the file manager and find the files
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFiles(Files, *FullDir, *PhotoFileExtension);

	for (const FString& Filename : Files)
	{
		UE_LOG(LogTemp, Log, TEXT("Found: %s"), *Filename);
	}
}

bool UPhotoAlbumSubsystem::LoadPhoto(const FString& Filename, const FString& AlbumFolder, UTexture2D*& PhotoTexture)
{
	// concatenate the full path to the photo directory
	const FString FullDir = BuildFullPath(Filename, AlbumFolder);

	UE_LOG(LogTemp, Log, TEXT("Loading album photo: %s"), *FullDir);

	// import the file
	PhotoTexture = FImageUtils::ImportFileAsTexture2D(FullDir);

	// check if the texture is valid
	if (PhotoTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("Photo loaded successfully!"));

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Photo not found: %s"), *FullDir);

	// photo not found
	return false;
}

bool UPhotoAlbumSubsystem::SavePhotoFromRenderTarget(UTextureRenderTarget2D* RenderTarget, const FString& AlbumFolder, const FString& FilePrefix, FString& SavedFileName)
{
	// ensure the render target is valid
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid render target."));

		return false;
	}

	// get the render target image
	FImage RTImage;

	if (!FImageUtils::GetRenderTargetImage(RenderTarget, RTImage))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not get the render target image."));

		return false;
	}

	// ensure the image is fully opaque
	FImageCore::SetAlphaOpaque(RTImage);

	// wrap into an ImageView
	FImageView RTImageView(RTImage);

	// build the image filename from the current date and time
	SavedFileName = BuildFullPath(FilePrefix + "_" + FDateTime::Now().ToString() + "." + PhotoFileExtension, AlbumFolder);

	// attempt to save the image
	if (FImageUtils::SaveImageAutoFormat(*SavedFileName, RTImageView))
	{
		UE_LOG(LogTemp, Log, TEXT("Saved photo album file: %s"), *SavedFileName);

		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Could not save album file: %s"), *SavedFileName);

	// save failed
	return false;
}

bool UPhotoAlbumSubsystem::DeletePhotoFromAlbum(const FString& Filename, const FString& AlbumFolder)
{
	// get the full path
	FString FullPath = BuildFullPath(Filename, AlbumFolder);

	// get the file manager
	IFileManager& FileManager = IFileManager::Get();

	// ensure the file exists
	if (FileManager.FileExists(*FullPath))
	{
		// attempt to delete the file
		if (FileManager.Delete(*FullPath, true, true))
		{
			UE_LOG(LogTemp, Log, TEXT("Album File deleted: %s"), *FullPath);

			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Album File queried for deletion doesn't exist: %s"), *FullPath);

	// delete failed
	return false;
}

FString UPhotoAlbumSubsystem::BuildFolderPath(const FString& AlbumFolder) const
{
	return FPaths::ProjectSavedDir() + AlbumFolder + "/";
}

FString UPhotoAlbumSubsystem::BuildFullPath(const FString& Filename, const FString& AlbumFolder) const
{
	return BuildFolderPath(AlbumFolder) + Filename;
}