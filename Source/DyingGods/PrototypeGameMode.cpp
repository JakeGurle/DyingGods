

#include "DyingGods.h"
#include "PrototypeGameMode.h"

APrototypeGameMode::APrototypeGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Sets the default pawn to our blueprinted hero class
	static ConstructorHelpers::FObjectFinder<UBlueprint> PlayerPawnObject(TEXT("Blueprint'/Game/Blueprints/BP_Hero.BP_Hero'"));
	if (PlayerPawnObject.Object != NULL)
	{
		DefaultPawnClass = (UClass*)PlayerPawnObject.Object->GeneratedClass;
	}
}


