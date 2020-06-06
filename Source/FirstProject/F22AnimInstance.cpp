
#include "F22AnimInstance.h"
#include "FirstProjectPawn.h"

void UF22AnimInstance::UpdateAnimationProperties()
{
	const APawn* character = TryGetPawnOwner();
	const AFirstProjectPawn* f22Character;

	if (character && (f22Character = Cast<AFirstProjectPawn>(character)) != nullptr)
	{
	}
}