# Push Pawn <img align="right" width=128, height=128 src="https://github.com/Vaei/PushPawn/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **Net-predicted soft organic collisions!**
> <br>Eliminate multiplayer de-sync when moving near other pawns
> <br>Collisions that feel great for the best player experience possible!

> [!TIP]
> Suitable for both singleplayer and multiplayer games

## Features

### Net Predicted Collisions
Push Pawn leverages GAS for net prediction.

With the default Unreal implementation, if two characters touch on the server -- so you might not even see it! -- a drastic desync occurs. :dizzy_face:

But with PushPawn, no desync occurs regardless of high latency! :star_struck:

> [!NOTE]
> Push Pawn has a lightweight and highly optimized implementation that is proven in production.

### Soft Organic Collisions
The included Push Ability applies a force over a short amount of time.

Walking into another pawn is very much like walking into a human in the real world! We can push past them or push them out of the way. We can even make them get angry! :cursing_face:

Most importantly, we're not experiencing the effect of running into a brick wall :persevere: which is Unreal's default collision for Pawns.

### Custom Collision Shapes
Capsule, Box, Sphere collisions are all supported for Pawn vs Pawn collisions

> [!TIP]
> Characters are Pawns -- Push Pawn supports custom collision shapes even with Character Movement Component!
> <br>Of course, this doesn't affect their navigation nor interaction with the environment

### Push Abilities & Gameplay Events
Push Abilities can respond to gameplay events and trigger their own events. :triumph:

> [!NOTE]
> Push Pawn can drive gameplay systems, it is not merely cosmetic.

> [!TIP]
> Play a "shove" anim montage
> <br>Implement procedural skeletal physics so their body physically reacts to being bumped
> <br>Make your NPCs yell at the player when they run into them too fast!

### Pushing Each Other
When a group of NPCs attack a player, they often get in the way :skull: With Push Pawn they will push past each other with no extra work for you beyond setting up Push Pawn itself!

Is another player blocking a door you want to pass through? No problem, we can push past them also!

Or perhaps you'd like to be able to push AFK players off cliffs... :smiling_imp:

### Exceptionally Customizable
One day, someone is going to do something surprising with Push Pawn, and I look forward to it, because the possibilities to adjust behaviour really are endless. That someone might be you! :smiley_cat:

## How To Use

> [!WARNING]
> ***Due to the use of Git LFS, do not download a zip or your content will be missing***
> <br>You will need to clone this via `git clone https://github.com/Vaei/PushPawn.git`

> [!IMPORTANT]
> [Read the Wiki to Learn How to use Push Pawn](https://github.com/Vaei/PushPawn/wiki)

## Example
_Examples are based on 1.0.0_

Here is the preview from the included minimal Third Person Example project. This footage was obtained with >200ms and `p.netshowcorrections 1` - as you can see, it doesn't de-sync abnormally.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview.gif)

This was initially created for a personal project that would make the AI play a push animation and yell at the player, which is not included, however the helper functions for building this are included.

![example usage](https://github.com/Vaei/repo_files/blob/main/PushPawn/preview_isekai.gif)

## Changelog

### 2.3.0-beta
* Remove separate ability-granting task and move into scan task

### 2.2.0-beta
* Removed final from UPushPawn_Action and noted the expectation to mark yours as final
* Add UAbilityTask_PushPawnForce
	* This properly ends abilities when root motion source expires
	* Very lightweight compared to engine tasks that replicate unnecessarily (for PushPawn)
* Add helper GatherPushOptions to UPushStatics
* Output ShapeRotation from GetPusheeCollisionShape for rotated shape support
* Export FPushQuery & FPushOption
* Preliminary completion of blueprint support
	* Add UPusheeTargetComponent
	* Migrate commonalities to UPushPawnComponent

### 2.1.10-beta
* Wrap UPushStatics::GetPushPawnScanRange return statement for Linux compiler

### 2.1.9-beta
* Change LoadingPhase to PreDefault

### 2.1.8-beta
* Add support for horizontal capsule collisions

### 2.1.7-beta
* Wait Net Sync properties moved to sub-category and AdvancedDisplay
* Added advanced disable feature for Wait Net Sync

### 2.1.6-beta
* Fix not applying rotation to box scan
* Fix using default shapes without scale
* Refactor PusherRadiusScalar to PusheeRadiusScalar & Accel equivalent - oops
* Switch from ClearTimer to ClearAllTimersForObject in ability tasks

_Blueprint_
* Add K2_UpdatePawnOwner as a cache-point for PusheeComponent
* Added FromCollisionShape to FPushPawnCollisionShapeHelper
* Add K2_GetDefaultPusheeCollisionShape to UPusheeComponentHelper for BP only user convenience

### 2.1.5-beta
* Renamed curve to include `Action` keyword
* Added additional curve for scanning and assigned by default
* Default to 2D direction for scanning

### 2.1.4-beta
* Auto-Activate scan ability when granted by default
* FPushPawn namespace renamed to FPushPawnCVars
* Add debug drawing for PushPawn_Action "p.PushPawn.Action.Debug.Draw"
* Fix bug with checking diff post-normalization instead of pre-normalization

### 2.1.3-beta
* ActivatePushPawnAbility returns a bool so we can ensure Super didn't end the ability (returns false)
* Remove redundant and now deprecated SetMovementSyncPoint

### 2.1.2-beta
* PushPawn now compatible with blueprint only projects
* Added optional UPusheeComponent
	* Extended UPusheeComponentHelper to implement BP-exposed IPusheeInstigator interface
	* Added UPushStatics::GetPusheeInstigator as a central method for retrieving IPusheeInstigator
* Rename BlueprintDemo folder to Blueprints

### 2.1.1-beta
* Add PushPawnEditor module
* Add Details Customization to hide properties we removed support for in our lightweight base ability
	* Or irrelevant properties

### 2.1.0-beta
* Support non-capsule collision shapes to allow for Pawn v Pawn collisions to use custom shape types such as boxes

### 2.0.2-beta
* Remove dependency on ACharacter/UCharacterMovementComponent
	* Built-in Push ability still maintains this dependency because it uses root motion sources
	* Acceleration is always normalized with Mover's MovementIntent in mind
	* Remove searching for UCapsuleComponent* as root
* Add UPushStatic getters using IPusheeInstigator instead of APawn to reduce casting
* Add IPusheeInstigator::GetPusheeCapsuleShape() allowing non-capsule root components, as well as using a capsule that doesn't represent the collision
* Add IPusheeInstigator::GetPusheeVelocity() to remove need to casting to Pawn to get it
* Added sanity checks for 0 scan range
* Refactor ability tasks
	* AbilityTask_WaitForPushTargets ➜ UAbilityTask_PushPawnScan_Base
	* UAbilityTask_WaitForPushTargets_CapsuleTrace ➜ UAbilityTask_PushPawnScan
	* UAbilityTask_GrantNearbyPush ➜ UAbilityTask_GrantPushAbility

### 2.0.1-beta
* Ability itself pauses along with the scan ability, aimed at preventing ASC from continuing to tick, pending testing
* Possible engine crash fix for ability tasks

### 2.0.0-beta
_BREAKING CHANGES_

_Significant perf improvements_

* Overhauled the internal system
	* Created lightweight ability classes with details customization to reduce perf overhead
		* PushPawn_Ability as the parent class for all abilities
		* PushPawn_Scan_Base
		* PushPawn_Ability_Base
	* Created C++ variations of classes to remove overhead from BP VM
		* PushPawn_Scan
		* PushPawn_Ability
		* Content is included primarily for example purposes and non-C++ users
	* Previous ability classes have been removed, including the corresponding content
		* GameplayAbility_Push_Scan removed in favour of PushPawn_Scan_Base, UPushPawn_Scan_Core, and PushPawn_Scan
	* Instead of sending the direction as an FHitResult's Normal, a lighter class with only a normal vector has been added (FPushPawnAbilityTargetData)
	* Added pre-activation interface checks that don't require pusher - use to check if alive, etc. to prevent activation entirely
		* IPusheeInstigator: CanBePushed() refactored to CanBePushedBy(), and IsPushable() added
		* IPusherTarget: CanPush() refactored to CanPushPawn(), and IsPushCapable() added
	* Added TRACE_CPUPROFILER_EVENT_SCOPE throughout for profiling
	* Added IPusheeInstigator::GetPushPawnScanPausedDelegate() that can be used to conditionally disable scanning when no one is nearby

* Improved synchronization
	* Scan task awaits net sync intermittently, see UPushPawn_Scan_Base::ShouldWaitForNetSync() and ConsumeWaitForNetSync() to customize behaviour

* Quality of life changes
	* Improved overall feel of default values
	* Added FPushPawnActionParams & FPushPawnScanParams structs
	* Improved handling for runtime parameters
	* Overhauled handling of scan rate and range to allow for runtime changes based on acceleration
	* Added PushPawnTags to better define reusable native gameplay tags
	* Categories changed from Push to PushPawn
	* General refactoring to improve readability
	* Improved code comments throughout

### 1.3.1
* Add Icon

### 1.3.0
_If you're on 1.2.0 you need to update_

* Refactor GA_Push_x to GA_PushPawn_x
* Push force curve no longer normalized, better defaults set
* Fixed bug where valid direction wasn't obtained when on top of each other
* Update blueprint content with changes
* Brought forward fixes from 1.2.0 that were accidentally not fully implemented or missed - if on 1.2.0 you must update
* Enabled gravity during root motion force by default
* Added helper functions to GA_PushPawn_Action

### 1.2.0
* Fix major init order bug
* Add radius scaling under acceleration
* Add radius scaling under velocity
* Add separate scan rate under acceleration
* Remove unused scan range
* Fix starting ability before actor has begun play

### 1.1.2
* Switch to Cvar to show debug

### 1.1.1
* Add missing include

### 1.1.0
* Move static helpers from `GameplayAbility_Push_Action` to `PushStatics`
* Fix include paths and const correctness
* Add inline cpp includes

### 1.0.0
* Initial Release