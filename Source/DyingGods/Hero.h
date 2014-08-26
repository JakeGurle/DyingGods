

#pragma once

#include "GameFramework/Character.h"
#include "Hero.generated.h"

/**
 * 
 */
UCLASS()
class DYINGGODS_API AHero : public ACharacter
{
	GENERATED_UCLASS_BODY()
	//Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TSubobjectPtr<UCameraComponent> FirstPersonCameraComponent;
	//1st person model
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	TSubobjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	//Start doing damage
	UFUNCTION(BlueprintCallable, Category = "Game Combat")
	void StartDoingDamage();
	//Stop doing damage
	UFUNCTION(BlueprintCallable, Category = "Game Combat")
	void StopDoingDamage();
	//Creates the melee hitbox
	UFUNCTION()
	void AttackTrace();
	//Process actors in hitbox
	UFUNCTION()
	void ProcessHitActor(AActor* ActorToProcess);
	//Override Tick
	virtual void Tick(float DeltaSeconds) override;
	//Is the player doing damage?
	bool bDoingDamage;
	//Array of actors in the hitbox
	TArray<AActor*> HitActors;
	
protected:
	//Handles player input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	//Handles moving forwards and backwards
	UFUNCTION()
	void MoveForward(float val);
	//Handles strafing
	UFUNCTION()
	void MoveRight(float val);
	//Sets jump flag when key is pressed
	UFUNCTION()
	void OnStartJump();
	//Clears jump flag when key is released
	UFUNCTION()
	void OnStopJump();
};
