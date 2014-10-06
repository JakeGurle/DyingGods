/*
* Implements the Hero actor class's behavior (camera, movement, attacks)
*/

#include "DyingGods.h"
#include "Hero.h"

/*
* Constructor
* @PCIP: Initializes default actor properties
*/
AHero::AHero(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Print a message to verify that we are using the Hero class as a pawn
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("We are using Hero!"));
	}
	//Create a camera component
	FirstPersonCameraComponent = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	//Attach the camera component to our capsule (the hero)
	FirstPersonCameraComponent->AttachParent = CapsuleComponent;
	//Adjust the height of the camera so it's in a logical place relative to the model
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 25.0f + BaseEyeHeight);
	//Create first person mesh
	FirstPersonMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->AttachParent = FirstPersonCameraComponent;
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->CastShadow = false;
	//The third person body mesh is hidden from the owner
	Mesh->SetOwnerNoSee(true);
}

/*
* Bind our input components that are set in the editor to the Move functions
* "MoveForward", "MoveRight" and others are input types that are set using Project Settings -> Input in the editor
* @InputComponent: UE defined input processor
*/
void AHero::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	InputComponent->BindAxis("MoveForward", this, &AHero::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHero::MoveRight);
	InputComponent->BindAxis("Turn", this, &AHero::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &AHero::AddControllerPitchInput);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AHero::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AHero::OnStopJump);
	InputComponent->BindAction("Attack", IE_Pressed, this, &AHero::StartDoingDamage);
	InputComponent->BindAction("Attack", IE_Released, this, &AHero::StopDoingDamage);
}

/*
* Controls the forward movement of the character.
* @value: defines how much the character moves forward
*/
void AHero::MoveForward(float value)
{
	//If the controller exists 
	if ((Controller != NULL) && (value != 0.0f))
	{
		//Figure out which way is forward
		FRotator Rotation = Controller->GetControlRotation();
		//Limit pitch when walking or falling
		if (CharacterMovement->IsMovingOnGround() || CharacterMovement->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		//Add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}
}

/*
* Controls the strafing of the character.
* @Params: value defines how much the character moves forward
*/
void AHero::MoveRight(float value)
{
	if ((Controller != NULL) && (value != 0.0f))
	{
		//Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		//add movement in that direction
		AddMovementInput(Direction, value);
	}
}

/* 
* Start the jump...
*/
void AHero::OnStartJump()
{
	bPressedJump = true;
}

/*
* ...End the jump
*/
void AHero::OnStopJump()
{
	bPressedJump = false;
}

/*
* Start doing damage...
*/
void AHero::StartDoingDamage()
{
	bDoingDamage = true;
	//Check if the attack is starting correctly
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("attack start"));
}

/*
* ...Stop doing damage
*/
void AHero::StopDoingDamage()
{
	//Check if the attack is ending correctly
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("attack end"));
	bDoingDamage = false;
	//Empty the array, because we're done with doing damage now
	HitActors.Empty();
}

/*
* Checks on every frame to see if we're doing damage; if so, do the stuff
* @DeltaSeconds: Time passed
*/
void AHero::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDoingDamage)
	{
		AttackTrace();
	}
}

/*
* Spawns the hitbox and processes the actors within it
*/
void AHero::AttackTrace()
{
	//Actors that overlap the box stored in this array
	TArray<struct FOverlapResult> OutOverlaps;
	//Orient the box in the direction of the character
	FQuat Rotation = Instigator->GetTransform().GetRotation();
	FVector Start = Instigator->GetTransform().GetLocation() + Rotation.Rotator().Vector() * 100.0f;

	FCollisionShape CollisionHitShape;
	FCollisionQueryParams CollisionParams;
	//Have the hitbox ignore the player
	CollisionParams.AddIgnoredActor(Instigator);

	//Set what will respond to the collision
	FCollisionObjectQueryParams CollisionObjectTypes;
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	//Create the hitbox and get the actors within
	CollisionHitShape = FCollisionShape::MakeBox(FVector(100.0f, 60.0f, 0.5f));
	GetWorld()->OverlapMulti(OutOverlaps, Start, Rotation, CollisionHitShape, CollisionParams, CollisionObjectTypes);

	//Process all hit actors
	for (int i = 0; i < OutOverlaps.Num(); i++)
	{
		if (OutOverlaps[i].GetActor() && !HitActors.Contains(OutOverlaps[i].GetActor()))
		{
			//Check if hit registered
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("hit"));
			//Now call the function that does something to our unfortunate actor...
			ProcessHitActor(OutOverlaps[i].GetActor());
		}
	}
}

//Now, the function to despawn the actor
void AHero::ProcessHitActor(AActor* ActorToProcess)
{
	//If we're done, we're done
	if (!ActorToProcess)
	{
		return;
	}

	HitActors.AddUnique(ActorToProcess);
	FHitResult AttackHitResult;
	FDamageEvent AttackDamageEvent;
	AHero* Hero = Cast<AHero>(ActorToProcess);
	//Check if the function executed correctly
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("damage done"));
	//Despawn the actor we're processing
	ActorToProcess->Destroy();
}