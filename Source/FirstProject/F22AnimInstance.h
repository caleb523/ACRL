#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "F22AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API UF22AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RRUp_;

	UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	void UpdateAnimationProperties();
};
