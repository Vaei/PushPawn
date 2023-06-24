# PushPawn
***Due to the use of Git LFS, do NOT download a zip or your content will be missing.*** You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git` .

Allows pawns to push each other, uses GAS to prevent desyncs that often occur when colliding with AI due to lack of net prediction between Characters.

The primary purpose is to overcome desync when players collide with AI, but does provide a nice "soft collision" result instead of the rigid "hard collision" that most games have.

Initially created based on the 5.1 LyraShooter interaction system. Please note that some of the code comments are left-overs from LyraShooter's interaction system and may not make sense in this context.

Summary: When a pawn that can be pushed (pushee) finds a nearby pawn that can push them (pusher), the pushee will request the push options from the pusher, and apply those to itself via a `UGameplayAbility`.

Tested >200ms without problematic desync.

## Example
This was initially created for the upcoming project with the working title "Isekai" and is offered freely under the MIT License. This game also makes the AI play a push animation and yell at the player, which is not included, however the helper functions for building this are included.
![example usage](https://github.com/Vaei/PlayWidgetAnimation/blob/examples/Example1.png)

## Difficulty
This is very advanced C++ use, if you are a beginner you will struggle with this and dedicated support isn't available. It will benefit you if you understand how LyraShooter's interaction system works though it isn't a requirement. There may be some redundancy that came from duplicating that system as a starting point which may lead to confusion.

## Terminology
*The most confusing yet important aspect to wrap your head around is this: The ability `Instigator` is the one who gets pushed, and not the one who does the pushing.*

### AICharacter
An AI controlled `ACharacter` (possesed by `AAIController`)

### PlayerCharacter
A player controlled `ACharacter` (possesed by `APlayerController`)

### Standard Use Case
Refers to the use-case where an AICharacter is using this system specifically to push a PlayerCharacter.

### Pushee
This is the Pawn that is getting pushed by another Pawn. Implementing `IPusheeInstigator`. Under the Standard Use Case this is a PlayerCharacter.

This Pawn is the ability Instigator. Due to the inversion, this term was avoided because it could cause confusion.

This Pawn searches for anyone who should push them, effectively a psuedo collision detection, and then "Instigates" by handing the colliding Pawn the ability that is used to push them back.

This means that if a massive world boss needs to push a player back, they can push a player back in their own distinct way especially factoring their unique collision properties, instead of a single universal method for pushing back.

### Pusher
This is the Pawn that is pushing the other player. Implementing `IPusherTarget`. Under the Standard Use Case this is an AI Character.

## Warning
This has only been tested for a single use case: An AI controlled ACharacter pushing a player controlled ACharacter. No other use-case has currently been tested as of the time of writing this.

The content is only supported by Unreal Engine 5.2 and up. However, there is probably nothing stopping you from mimicking the blueprints and only making use of the source code.

## Limitations
If the capsule dimensions change between prediction frames it can desync. For most of us sending this data is an unnecessary cost, but if you need to do it, add the capsule `HalfHeight` and/or `Radius` to `FPushOption` and send it along with `IPusheeInstigator::GatherPushOptions`, however this may not be sufficient on it's own! Check where the `UCapsuleComponent` getters are being used and replace these too. Any data NOT send through the `FPushOption` is very unlikely to be predicted.

This may also not be sufficient as it remains untested.

## LyraShooter
*Or anyone not using `UGameplayAbility` as an allowed base class*

*This has been tested and is confirmed working with LyraShooter's framework*

LyraShooter requires that `GameplayAbilities` extend `ULyraGameplayAbility`; you will likely need to duplicate `UGameplayAbility_Push_Scan` into your project and have it inherit `ULyraGameplayAbility` and then duplicate the `GA_Push_Scan` ability in the content folder and reparent it under this one.

Add to the ctor of duplicated ability: `ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;` and add to the appropriate attribute sets so that it always runs. Don't forget to change the `PUSHPAWN_API` macro to your own!

## How to Use
*This how to was written from the viewpoint of a brand new 5.2 "Third Person" template. Many of these steps may be unnecessary for your project.*

### Obtain Plugin
Clone to your project's plugin folder and build from source. Due to the use of Git LFS, do NOT download a zip or your content will be missing. You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git` .

### Prerequisite Setup
1. Launch Unreal 5.2 and create C++ Third Person template
1. Add a navmesh
1. 

### Optional
Duplicate the gameplay ability from the content folder to your project and extend as required. If you don't need to extend it, no need to do this step.

Any character that can be pushed must implement the IPusheeTarget interface (C++ only)
Any character that can do the pushing must implement the IPusherTarget interface (C++ only)

In `GA_Push_Action`, assign a trace channel. You probably do not want to leave this as `ECC_Visibility`, but instead setup a channel that only valid Pushers will block.

## Changelog
### 1.0.0.0
Initial Release