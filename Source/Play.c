
//============================================================================
//----------------------------------------------------------------------------
//									Play.c
//----------------------------------------------------------------------------
//============================================================================

// This (rather large) file handles all player routines while a game is in�
// progress. It gets the player's input, moves the player, tests for collisions�
// and generally handles the "main game loop".  Enemies and actually drawing�
// the graphics to the screen are handled in other files.

#include "Externs.h"


#define kFlapImpulse			48
#define kGlideImpulse			12
#define kAirResistance			2
#define kMaxHVelocity			192
#define kMaxVVelocity			512
#define kNumLightningStrikes	5


void SetUpLevel (void);
void ResetPlayer (Boolean);
void OffAMortal (void);
void DoCommandKey (void);
void GetPlayerInput (void);
void HandlePlayerIdle (void);
void HandlePlayerFlying (void);
void HandlePlayerWalking (void);
void HandlePlayerSinking (void);
void HandlePlayerFalling (void);
void HandlePlayerBones (void);
void MovePlayer (void);
void CheckTouchDownCollision (void);
void CheckPlatformCollision (void);
void KeepPlayerOnPlatform (void);
void CheckLavaRoofCollision (void);
void SetAndCheckPlayerDest (void);
void HandleLightning (void);
void FinishLightning (void);
void HandleCountDownTimer (void);
void CheckHighScore (void);


playerType	thePlayer;
enemyType	theEnemies[kMaxEnemies];
KeyMap		theKeys;
Rect		platformRects[6], touchDownRects[6], enemyRects[24];
Rect		enemyInitRects[5];
long		theScore, wasTensOfThousands;
short		numLedges, beginOnLevel, levelOn, livesLeft, lightH, lightV;
short		lightningCount, numEnemies, countDownTimer;
Boolean		playing, pausing, flapKeyDown, evenFrame;
Boolean		doEnemyFlapSound, doEnemyScrapeSound;

extern	handInfo	theHand;
extern	prefsInfo	thePrefs;
extern	Rect		playerRects[11], mainWindowRect;
extern	short		numUpdateRects1, numUpdateRects2, numOwls;
extern	Boolean		quitting, openTheScores;


//==============================================================  Functions
//--------------------------------------------------------------  InitNewGame

// This funciton sets up variables and readies for a new game.  It is called�
// only when a the user selects "New Game" - during the course of a game, it�
// is  not called again.

void InitNewGame (void)
{								// Initialize a number of game variables.
	countDownTimer = 0;			// Zero count down timer.
	numLedges = 3;				// Initial number of ledges (platforms).
	beginOnLevel = 1;			// Ledge (platform) the player is on (center ledge).
	levelOn = 0;				// Game level on (first level).
	livesLeft = kInitNumLives;	// Number of player lives remaining.
	theScore = 0L;				// Player's score (a long - can go to 2 billion).
	playing = TRUE;				// Flag playing.
	pausing = FALSE;			// Not paused.
	evenFrame = TRUE;			// Set an initial state for evenFrame.
	wasTensOfThousands = 0L;	// Used for noting when player gets an extra life.
	numOwls = 4;				// Number of "owl" enemies for this level.
	
	numUpdateRects1 = 0;		// Init number of "update" rectangles.
	numUpdateRects2 = 0;		// (see Render.c to see what these do)
	
	InitHandLocation();			// Get the mummy hand down in the lava.
	theHand.mode = kLurking;	// Flag the hand in "lurking" mode.
	
	SetUpLevel();				// Set up platforms for first level (wave).
	
	DumpBackToWorkMap();		// Copy background offscreen to "work" offscreen.
	
	UpdateLivesNumbers();		// Display number of lives remaining on screen.
	UpdateScoreNumbers();		// Display the player's score (zero at this point).
	UpdateLevelNumbers();		// Display the level (wave) the player is on.
	
	GenerateEnemies();			// Prepare all enemies for this level.
	ResetPlayer(TRUE);			// Initialize all player variables and put on ledge.
}

//--------------------------------------------------------------  SetUpLevel

// Primarily, this function is called to set up the ledges for the�
// current level (wave) the player is on.  It determines how many�
// are required and then draws these offscreen.  It also flashes�
// the obelisks and strikes the lightning.

void SetUpLevel (void)
{
	short		wasLedges, waveMultiple;
	
	KillOffEye();					// Return eye to the aether.
	
	wasLedges = numLedges;			// Remember number of ledges.
	waveMultiple = levelOn % 5;		// Waves repeat every 5th wave (but harder!).
	
	switch (waveMultiple)			// See which of the 5 we're on.
	{
		case 0:						// Waves 0, 5, 10, �
		numLedges = 5;				// have 5 ledges (platforms) on screen.
		break;
		
		case 1:						// Waves 1, 6, 11, �
		numLedges = 6;				// are up to 6 ledges (platforms) on screen.
		break;
		
		case 2:						// Waves 2, 7, 12, �
		numLedges = 5;				// return to 5 ledges (platforms) on screen.
		break;
		
		case 3:						// Waves 3, 8, 13, �
		numLedges = 3;				// drop to 3 ledges (platforms) on screen.
		break;
		
		case 4:						// Waves 4, 9, 14, �
		numLedges = 6;				// and return to 6 ledges (platforms) on screen.
		break;
	}
	
	if (wasLedges != numLedges)		// No need to redraw if platforms are unchanged.
		DrawPlatforms(numLedges);
	
	FlashObelisks(TRUE);			// Flash the obelisks.
	
	GenerateLightning(320, 429);	// Lightning strikes platform 0.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	GenerateLightning(95, 289);		// Lightning strikes platform 1.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	GenerateLightning(95, 110);		// Lightning strikes platform 3.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	GenerateLightning(320, 195);	// Lightning strikes platform 5.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	GenerateLightning(545, 110);	// Lightning strikes platform 4.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	GenerateLightning(545, 289);	// Lightning strikes platform 2.
	StrikeLightning();
	LogNextTick(2);
	WaitForNextTick();
	StrikeLightning();
	
	FlashObelisks(FALSE);			// "Unflash" obelisks (return to normal state).
									// Play lightning sound!
	PlayExternalSound(kLightningSound, kLightningPriority);
	
	UpdateLevelNumbers();			// Display the current level on screen.
}

//--------------------------------------------------------------  ResetPlayer

// This function prepares the player - it places the player and his/her mount�
// in their proper starting location (depending on which platform they are to�
// begin on), and it sets all the player's variables to their initial state.

void ResetPlayer (Boolean initialPlace)
{
	short		location;
	
	thePlayer.srcNum = 5;			// Set which graphic (frame) the player is to use.
	thePlayer.frame = 320;			// This variable will be used as a coutndown timer.
	
	if (initialPlace)				// If "initialPlace" is TRUE, �
		location = 0;				// the player is to begin on the lowest platform.
	else							// Otherwise, a random location is chosen.
		location = RandomInt(numLedges);
	
	switch (location)				// Move player horizontally and vertically to their�
	{								// proper location (based on ledge # they're on).
		case 0:
		thePlayer.h = 296 << 4;		// Bottom center ledge.
		thePlayer.v = 377 << 4;		// We're scaling by 16.
		break;
		
		case 1:
		thePlayer.h = 102 << 4;		// Lower left ledge.
		thePlayer.v = 237 << 4;
		break;
		
		case 2:
		thePlayer.h = 489 << 4;		// Lower right ledge.
		thePlayer.v = 237 << 4;
		break;
		
		case 3:
		thePlayer.h = 102 << 4;		// Top left ledge.
		thePlayer.v = 58 << 4;
		break;
		
		case 4:
		thePlayer.h = 489 << 4;		// Top right ledge.
		thePlayer.v = 58 << 4;
		break;
		
		case 5:
		thePlayer.h = 296 << 4;		// Top central ledge.
		thePlayer.v = 143 << 4;
		break;
	}
									// Assign destination rectangle.
	thePlayer.dest = playerRects[thePlayer.srcNum];
	ZeroRectCorner(&thePlayer.dest);
	OffsetRect(&thePlayer.dest, thePlayer.h >> 4, thePlayer.v >> 4);
	thePlayer.wasDest = thePlayer.dest;
	
	thePlayer.hVel = 0;				// Player initially has no velocity.
	thePlayer.vVel = 0;
	thePlayer.facingRight = TRUE;	// We're facing to the right.
	thePlayer.flapping = FALSE;		// We're not flapping our wings initially.
	thePlayer.wrapping = FALSE;		// We can't be wrapping around the edge of the screen.
	thePlayer.clutched = FALSE;		// The hand ain't got us.
	thePlayer.mode = kIdle;			// Our mode is "idle" - waiting to be "born".
	if (lightningCount == 0)		// Prepare for a lightning display to "birth" us.
	{
		lightH = thePlayer.dest.left + 24;
		lightV = thePlayer.dest.bottom - 24;
		lightningCount = kNumLightningStrikes;
	}
}

//--------------------------------------------------------------  OffAMortal

// Alas, 'tis here that a player is brought who loses a life.

void OffAMortal (void)
{
	livesLeft--;				// Decrememnt number of player lives left.
	
	if (livesLeft > 0)			// Indeed, are there lives remaining?
	{
		ResetPlayer(FALSE);		// Good, start a new one off.
		UpdateLivesNumbers();	// Make note of the number of lives remaining.
	}
	else						// Otherwise, we are at the dreaded "Game Over".
		playing = FALSE;		// Set flag to drop us out of game loop.
}

//--------------------------------------------------------------  DoCommandKey

// This function handles the case when the user has held down the command�
// key.  Note, this only applies to input when a game is in session - otherwise�
// a standard event loop handles command keys and everything else.

void DoCommandKey (void)
{
	if (BitTst(&theKeys, kEKeyMap))			// Test for "command - E"�
	{
		playing = FALSE;					// which would indicate "End Game".
	}
	else if (BitTst(&theKeys, kPKeyMap))	// Otherwise, see if it's "command - P".
	{
		pausing = TRUE;						// This means the player is pausing the game.
		MenusReflectMode();					// Gray-out menus etc.
		DumpMainToWorkMap();				// Save screen to offscreen.
	}
	else if (BitTst(&theKeys, kQKeyMap))	// Or perhaps the player hit "command - Q".
	{
		playing = FALSE;					// Set flag to drop out of game loop.
		quitting = TRUE;					// Set flag to drop out of Glypha.
	}
}

//--------------------------------------------------------------  GetPlayerInput

// This function looks for keystrokes when a game is underway.  We don't use�
// the more conventional event routines (like GetNextEvent()), because they're�
// notoriously slow, allow background tasks, introduce possible INIT problems,�
// and we don't have to.  Instead, we'll rely on GetKeys() (which has its own�
// set of problems - but we deal with them).

void GetPlayerInput (void)
{
	thePlayer.flapping = FALSE;				// Assume we're not flapping.
	thePlayer.walking = FALSE;				// Assume too we're not walking.
	
	GetKeys(theKeys);						// Get the current keyboard keymap.
	if (BitTst(&theKeys, kCommandKeyMap))	// See first if command key down�
		DoCommandKey();						// and handle those seperately.
	else									// If not command key, continue.
	{										// Look for one of the two "flap" keys.
		if ((BitTst(&theKeys, kSpaceBarMap)) || (BitTst(&theKeys, kDownArrowKeyMap)))
		{
			if (thePlayer.mode == kIdle)	// Handle special case when player is idle.
			{
				thePlayer.mode = kWalking;	// Set the player's mode now to walking.
				thePlayer.frame = 0;		// Used to note "state" of walking.
			}								// Otherwise, if player is flying or walking�
			else if ((thePlayer.mode == kFlying) || (thePlayer.mode == kWalking))
			{
				if (!flapKeyDown)			// If flap key was not down last frame�
				{							// (this is to prevent "automatic fire").
											// Give player lift.
					thePlayer.vVel -= kFlapImpulse;
					flapKeyDown = TRUE;		// Note that the flap key is down.
											// Play the "flap" sound.
					PlayExternalSound(kFlapSound, kFlapPriority);
											// Set player flag to indicate flapping.
					thePlayer.flapping = TRUE;
				}
			}
		}
		else
			flapKeyDown = FALSE;			// If flap key not down, remember this.
		
											// Test now for one of three "right" keys.
		if ((BitTst(&theKeys, kRightArrowKeyMap) || 
				BitTst(&theKeys, kSKeyMap) || 
				BitTst(&theKeys, kQuoteMap)) && 
				(thePlayer.hVel < kMaxHVelocity))
		{
			if (thePlayer.mode == kIdle)	// Handle special case when player idle.
			{								// They are to begin walking (no longer idle).
				thePlayer.mode = kWalking;
				thePlayer.frame = 0;
			}
			else if ((thePlayer.mode == kFlying) || (thePlayer.mode == kWalking))
			{								// If flying or walking, player moves right.
				if (!thePlayer.facingRight)	// If facing left, player does an about face.
				{
					thePlayer.facingRight = TRUE;
					if (thePlayer.clutched)
					{
						thePlayer.dest.left += 18;
						thePlayer.dest.right += 18;						
						thePlayer.h = thePlayer.dest.left << 4;
						thePlayer.wasH = thePlayer.h;
						thePlayer.wasDest = thePlayer.dest;
					}
				}							// Otherwise, if facing right already�
				else
				{							// If flying, add to their horizontal velocity.
					if (thePlayer.mode == kFlying)
						thePlayer.hVel += kGlideImpulse;
					else					// If walking, set flag to indicate a step.
						thePlayer.walking = TRUE;
				}
			}
		}									// Test now for one of three "left" keys.
		else if ((BitTst(&theKeys, kLeftArrowKeyMap) || 
				BitTst(&theKeys, kAKeyMap) || 
				BitTst(&theKeys, kColonMap)) && 
				(thePlayer.hVel > -kMaxHVelocity))
		{
			if (thePlayer.mode == kIdle)	// Handle special case when player idle.
			{
				thePlayer.mode = kWalking;
				thePlayer.frame = 0;
			}
			else if ((thePlayer.mode == kFlying) || (thePlayer.mode == kWalking))
			{								// If flying or walking, player moves left.
				if (thePlayer.facingRight)	// If facing right, player does an about face.
				{							// Flag player facing left.
					thePlayer.facingRight = FALSE;
					if (thePlayer.clutched)	// Handle case where player gripped by hand.
					{						// An about face handled a bit differently.
						thePlayer.dest.left -= 18;
						thePlayer.dest.right -= 18;
						thePlayer.h = thePlayer.dest.left << 4;
						thePlayer.wasH = thePlayer.h;
						thePlayer.wasDest = thePlayer.dest;
					}
				}
				else						// Otherwise, player already facing left.
				{							// So player will move left.
					if (thePlayer.mode == kFlying)
						thePlayer.hVel -= kGlideImpulse;
					else
						thePlayer.walking = TRUE;
				}
			}
		}
	}
}

//--------------------------------------------------------------  HandlePlayerIdle

// Following are a number of functions handling the player's different "modes".
// This first function handles the player when in "idle" mode.  When idle, the�
// player is standing on a platform - having just been "born".  This is when the�
// player is in a "safe" mode - meaning no enemy can kill them.  The player remains�
// in idle mode until they hit a key to flap or move or until a timer (thePlayer.frame)�
// counts down to zero.

void HandlePlayerIdle (void)
{
	thePlayer.frame--;				// Count down the timer.
	if (thePlayer.frame == 0)		// See if timer has reached zero yet.
		thePlayer.mode = kWalking;	// If so, player is no longer idle.
	
	SetAndCheckPlayerDest();		// Keep player on platform.
}

//--------------------------------------------------------------  HandlePlayerFlying

// This function handles a player in "flying" mode.  In flying mode, the player�
// is alive and not standing/walking on any platform.  A plyaer remains in flying�
// mode until the player dies (collides unfavorably with an enemy), is caught by�
// the hand, or comes near the top of a platform (in which case they land and�
// switch to walking mode).  While in flying mode, gravity pulls the player down�
// while friction acts to slow the player down.

void HandlePlayerFlying (void)
{	
	if (thePlayer.hVel > 0)					// If player has a positive hori. velocity�
	{										// subtract frictional constant from velocity.
		thePlayer.hVel -= kAirResistance;
		if (thePlayer.hVel < 0)				// Don't let it go negative (otherwise, you�
			thePlayer.hVel = 0;				// can get a "yo-yo" effect set up).
	}
	else if (thePlayer.hVel < 0)			// Otherwise, if horizontal velocity negative�
	{										// add firctional constant to hori. velocity.
		thePlayer.hVel += kAirResistance;
		if (thePlayer.hVel > 0)
			thePlayer.hVel = 0;
	}
	
	thePlayer.vVel += kGravity;				// Add gravity to player's vertical velocity.
	
	if (thePlayer.vVel > kMaxVVelocity)		// Don't allow player to fall too fast.
		thePlayer.vVel = kMaxVVelocity;
	else if (thePlayer.vVel < -kMaxVVelocity)
		thePlayer.vVel = -kMaxVVelocity;	// And don't allow player to climb too fast.
	
	thePlayer.h += thePlayer.hVel;			// Add velocities to players position.
	thePlayer.v += thePlayer.vVel;
											// Now we determine which graphic to use.
	if (thePlayer.facingRight)				// There are the set of right-facing graphics.
	{
		thePlayer.srcNum = 1;				// Assume standard right-facing graphic.
		if (thePlayer.vVel < -kDontFlapVel)	// Now we jump through a series of hoops�
		{									// simply to determine whether we'll use�
			if (thePlayer.flapping)			// the graphic of the player with the wings�
				thePlayer.srcNum = 0;		// up (srcNum = 0) or with the wings down�
			else							// (srcNum = 1).
				thePlayer.srcNum = 1;
		}
		else if (thePlayer.vVel > kDontFlapVel)
		{
			if (thePlayer.flapping)
				thePlayer.srcNum = 1;
			else
				thePlayer.srcNum = 0;
		}
		else if (thePlayer.flapping)
			thePlayer.srcNum = 0;
	}
	else									// If the player is facing left�
	{										// We jump through a similar set of hoops�
		thePlayer.srcNum = 2;				// this time choosing between srcNum = 2 �
		if (thePlayer.vVel < -kDontFlapVel)	// and srcNum = 3.
		{
			if (thePlayer.flapping)
				thePlayer.srcNum = 3;
			else
				thePlayer.srcNum = 2;
		}
		else if (thePlayer.vVel > kDontFlapVel)
		{
			if (thePlayer.flapping)
				thePlayer.srcNum = 2;
			else
				thePlayer.srcNum = 3;
		}
		else if (thePlayer.flapping)
			thePlayer.srcNum = 3;
	}
	
	SetAndCheckPlayerDest();				// Check for wrap-around, etc.
	
	CheckLavaRoofCollision();				// See if player hit top or bottom of screen.
	CheckPlayerEnemyCollision();			// See if player hit an enemy.
	CheckPlatformCollision();				// See if player collided with platform.
	CheckTouchDownCollision();				// See if player has landed on platform.
}

//--------------------------------------------------------------  HandlePlayerWalking

// This function handles a player in "walking" mode.  They remain in this mode�
// until they walk off a platform's edge, flap to lift off the platform, or�
// collide unfavorably with an enemy (die).  While in walking mode, we need only�
// determine which frame of animation to display (if the player is taking steps)�
// and check for the usual set of collisions.

void HandlePlayerWalking (void)
{
	short		desiredHVel;
	
	if (thePlayer.walking)					// This means user is actively holding down�
	{										// the left or right key.
		if (evenFrame)						// Now we jump through a number of hoops�
		{									// in order to get a semi-realistic�
			if (thePlayer.facingRight)		// "stepping" animation going.  We take steps�
			{								// only on "even frames".
				if (thePlayer.srcNum == 4)
					desiredHVel = 208;
				else
					desiredHVel = 128;
			}
			else
			{
				if (thePlayer.srcNum == 7)
					desiredHVel = -208;
				else
					desiredHVel = -128;
			}
			
			if (thePlayer.hVel < desiredHVel)
			{
				thePlayer.hVel += 80;		// Move player right.
				if (thePlayer.hVel > desiredHVel)
				{							// This is the case where player is walking.
					thePlayer.hVel = desiredHVel;
					PlayExternalSound(kWalkSound, kWalkPriority);
				}
				else						// In this case, player is skidding.
					PlayExternalSound(kScreechSound, kScreechPriority);
			}
			else
			{
				thePlayer.hVel -= 80;		// Move player to the left.
				if (thePlayer.hVel < desiredHVel)
				{							// Player is stepping to left.
					thePlayer.hVel = desiredHVel;
					PlayExternalSound(kWalkSound, kWalkPriority);
				}
				else						// Player is skidding to a stop.
					PlayExternalSound(kScreechSound, kScreechPriority);
			}
		}
	}
	else									// If user is not actively holding down the�
	{										// left or right key, bring player to a stop.
		thePlayer.hVel -= thePlayer.hVel / 4;
		if ((thePlayer.hVel < 4) && (thePlayer.hVel > -4))
			thePlayer.hVel = 0;				// If close to zero (within 4), stop player.
		else								// Othewrwise, play the skidding sound.
			PlayExternalSound(kScreechSound, kScreechPriority);
	}
	
	if (thePlayer.vVel > kMaxVVelocity)		// Keep player from moving too quickly�
		thePlayer.vVel = kMaxVVelocity;		// left or right.
	else if (thePlayer.vVel < -kMaxVVelocity)
		thePlayer.vVel = -kMaxVVelocity;
	
	thePlayer.h += thePlayer.hVel;			// Move player horizontally and vertically�
	thePlayer.v += thePlayer.vVel;			// by the corresponding velocity.
	
	if (thePlayer.walking)					// "If player holding down left or right keys�".
	{
		if (evenFrame)						// Here's where we toggle between the two�
		{									// frames of "stepping" animation.
			if (thePlayer.facingRight)
				thePlayer.srcNum = 9 - thePlayer.srcNum;
			else
				thePlayer.srcNum = 13 - thePlayer.srcNum;
		}
	}
	else									// If the player not holding down keys�
	{										// draw the player just standing there.
		if (thePlayer.facingRight)
			thePlayer.srcNum = 5;
		else
			thePlayer.srcNum = 6;
	}
	
	SetAndCheckPlayerDest();				// Check for wrap-around and all that.
	
	CheckTouchDownCollision();				// See if player still on platform.
	KeepPlayerOnPlatform();					// Don't let player "sink through" ledge.
	CheckPlayerEnemyCollision();			// See if player hit an enemy.
}

//--------------------------------------------------------------  HandlePlayerSinking

// When the player is in "sinking" mode, they are on a one-way ticket to death.
// The player is sinking into the lava.  We put the player into this mode (rather�
// than kill them outright) so that we can have a number of frames of them slowly�
// slipping beneath the surface of the lava.  When the get below the surface of�
// the lava, they will be officially "killed" and a new player will be "born",

void HandlePlayerSinking (void)
{
	thePlayer.hVel = 0;						// Don't allow horizontal motion.
	thePlayer.vVel = 16;					// They will sink at this constant rate.
	if (thePlayer.dest.top > kLavaHeight)	// See if they slipped below the surface.
		OffAMortal();						// If they did, kill 'em.
	
	thePlayer.v += thePlayer.vVel;			// Otherwise, move them down a notch.
	
	SetAndCheckPlayerDest();				// Check for wrap-around, etc.
}

//--------------------------------------------------------------  HandlePlayerFalling

// "Falling" refers to a player who is dead already but is still careening�
// down the screen as a skeleton.  If (when) the player lands on a ledge they�
// will turn into a pile of bones for a short duration.  If instead they fall�
// into the lava, they'll sink.  In any event, it is then that they are�
// officially pronounced dead and a new player is born.

void HandlePlayerFalling (void)
{
	if (thePlayer.hVel > 0)				// Handle horizontal air resistance.
	{
		thePlayer.hVel -= kAirResistance;
		if (thePlayer.hVel < 0)
			thePlayer.hVel = 0;
	}
	else if (thePlayer.hVel < 0)
	{
		thePlayer.hVel += kAirResistance;
		if (thePlayer.hVel > 0)
			thePlayer.hVel = 0;
	}
	
	thePlayer.vVel += kGravity;			// Add in effect of gravity.
	
	if (thePlayer.vVel > kMaxVVelocity)	// Keep player from falling too fast.
		thePlayer.vVel = kMaxVVelocity;
	else if (thePlayer.vVel < -kMaxVVelocity)
		thePlayer.vVel = -kMaxVVelocity;
	
	thePlayer.h += thePlayer.hVel;		// Move player's x and y (h and v)�
	thePlayer.v += thePlayer.vVel;		// by amount of velocity in each direction.
	
	SetAndCheckPlayerDest();			// Check for wrap-around, etc.
	
	CheckLavaRoofCollision();			// See if they hit roof or lava.
	CheckPlatformCollision();			// See if they crashed to a ledge.
}

//--------------------------------------------------------------  HandlePlayerBones

// This is when the player is just a static pile of bones on a platform.  They�
// have been killed by an enemy and now are waiting to slip away so that a new�
// player can be born.

void HandlePlayerBones (void)
{
	if (evenFrame)					// To slow it down a bit, action only occurs�
	{								// on the even frames.
		thePlayer.frame--;			// Drop the counter down by one.
		if (thePlayer.frame == 0)	// When counter reaches zero, player officially dies.
			OffAMortal();
		else						// Otherwise, player's bones are sinking.
			thePlayer.dest.top = thePlayer.dest.bottom - thePlayer.frame;
	}
}

//--------------------------------------------------------------  MovePlayer

// This function is the sort of "master movement" function.  It looks�
// at what mode a player is in and calls the appropriate function from�
// above.  Arcade games (at least this one) tend to be very "modal" in�
// this way.  It's the actions of the user and the enemies in the game�
// that cause the player's mode to move from one state to another.

void MovePlayer (void)
{
	switch (thePlayer.mode)		// Check the "mode" the player is in.
	{
		case kIdle:				// Invulnerable - standing there - just born.
		HandlePlayerIdle();
		break;
		
		case kFlying:			// Flapping, floating, airborne.
		HandlePlayerFlying();
		break;
		
		case kWalking:			// On terra firma.  Standing or walking on ledge.
		HandlePlayerWalking();
		break;
		
		case kSinking:			// Trapped in the lava - going down.
		HandlePlayerSinking();
		break;
		
		case kFalling:			// Dead - a skeleton falling to earth.
		HandlePlayerFalling();
		break;
		
		case kBones:			// Dead - a static pile of bones on a ledge.
		HandlePlayerBones();
		break;
	}
}

//--------------------------------------------------------------  CheckTouchDownCollision

// This function determines whether or not the player is landed on a ledge.
// It does this by doing a rectangle collision between the player's bounding�
// rectangle and an imaginary rectangle enclosing an area above the ledges.
// I call these imaginary rectangles "touchDownRects[]".  The trick was that�
// you don't want the player to have to "hit" the top of a ledge in order to�
// land on it - there is an arbitrary distance above a ledge where, if the player�
// is within this area, the legs ought to come out and the player flagged as�
// walking.  As well, this same function is used for a walking player to see�
// if they are still on the ledge (they may walk off the edge).

void CheckTouchDownCollision (void)
{
	Rect		testRect, whoCares;
	short		i, offset;
	Boolean		sected;
	
	sected = FALSE;								// Assume not on ledge.
	for (i = 0; i < numLedges; i++)				// Go through all ledges.
	{
		testRect = touchDownRects[i];			// Here's the imaginary rect.
		if (thePlayer.mode == kWalking)			// We need an offset if player walking�
			OffsetRect(&testRect, 0, 11);		// since the player graphic is taller.
		
		if (SectRect(&thePlayer.dest, &testRect, &whoCares))
		{										// Does the player's rect intersect?
			if (thePlayer.mode == kFlying)		// Okay, it does, is the player airborne?
			{
				thePlayer.mode = kWalking;		// Put player into walking mode.
				if (thePlayer.facingRight)		// Assign correct graphic for player.
					thePlayer.srcNum = 5;
				else
					thePlayer.srcNum = 6;
				if (thePlayer.vVel > 0)			// Stop player from falling further.
					thePlayer.vVel = 0;
				thePlayer.dest.bottom += 11;	// "Grow" player's bounding rect.
				thePlayer.wasDest.bottom += 11;
												// Move player so standing on top of ledge.
				offset = thePlayer.dest.bottom - testRect.bottom - 1;
				thePlayer.dest.bottom -= offset;
				thePlayer.dest.top -= offset;
				thePlayer.v = thePlayer.dest.top << 4;
												// Play brief collision sound.
				PlayExternalSound(kGrateSound, kGratePriority);
			}
			sected = TRUE;						// Make note that we've landed.
		}
	}
	
	if (!sected)								// Now, if we didn't collide�
	{											// were we walking?
		if (thePlayer.mode == kWalking)			// Did we walk off the ledge?
		{
			thePlayer.mode = kFlying;			// Set player to flying mode.
			thePlayer.dest.bottom -= 11;		// Resize player's bounding rect.
			thePlayer.wasDest.bottom -= 11;
		}
	}
}

//--------------------------------------------------------------  CheckPlatformCollision

// Unlike the above function, this one tests the player's bounding rect against�
// the bounding rect of each ledge (not an imaginary rect above the ledge).  This�
// function is primarily for (then) collisions off the bottom and sides of the�
// ledges.  In this way, the ledges are "solid" - not able to be passed through.

void CheckPlatformCollision (void)
{
	Rect		hRect, vRect, whoCares;
	short		i, offset;
	
	for (i = 0; i < numLedges; i++)					// Walk through all ledges.
	{												// Test rectangle overlap.
		if (SectRect(&thePlayer.dest, &platformRects[i], &whoCares))
		{											// If player intersecting ledge�
			hRect.left = thePlayer.dest.left;		// Create our special test rect.
			hRect.right = thePlayer.dest.right;
			hRect.top = thePlayer.wasDest.top;
			hRect.bottom = thePlayer.wasDest.bottom;
													// Determine if the player hit the�
													// top/bottom of the ledge or the�
													// sides of the ledge.
			if (SectRect(&hRect, &platformRects[i], &whoCares))
			{										// We're fairly sure the player hit�
													// the left or right edge of ledge.
				if (thePlayer.h > thePlayer.wasH)	// If player was heading right�
				{									// player will bounce to left.
					offset = thePlayer.dest.right - platformRects[i].left;
					thePlayer.dest.left -= offset;
					thePlayer.dest.right -= offset;
					thePlayer.h = thePlayer.dest.left << 4;
					if (thePlayer.hVel > 0)			// We bounce back with 1/2 our vel.
						thePlayer.hVel = -(thePlayer.hVel >> 1);
					else
						thePlayer.hVel = thePlayer.hVel >> 1;
				}									// Else if player was heading left�
				else if (thePlayer.h < thePlayer.wasH)
				{									// player will bounce right.
					offset = platformRects[i].right - thePlayer.dest.left;
					thePlayer.dest.left += offset;
					thePlayer.dest.right += offset;
					thePlayer.h = thePlayer.dest.left << 4;
					if (thePlayer.hVel < 0)			// We bounce back with 1/2 our vel.
						thePlayer.hVel = -(thePlayer.hVel >> 1);
					else
						thePlayer.hVel = thePlayer.hVel >> 1;
				}									// Play impact sound.
				PlayExternalSound(kGrateSound, kGratePriority);
			}
			else									// It doesn't look like we hit the�
			{										// the left or right edge of ledge.
				vRect.left = thePlayer.wasDest.left;
				vRect.right = thePlayer.wasDest.right;
				vRect.top = thePlayer.dest.top;
				vRect.bottom = thePlayer.dest.bottom;
													// So we'll test top/bottom collision.
				if (SectRect(&vRect, &platformRects[i], &whoCares))
				{									// We've decided we've hit top/bottom.
					if (thePlayer.wasV < thePlayer.v)
					{								// If we were heading down (hit top)�
													// keep player on top of ledge.
						offset = thePlayer.dest.bottom - platformRects[i].top;
						thePlayer.dest.top -= offset;
						thePlayer.dest.bottom -= offset;
						thePlayer.v = thePlayer.dest.top << 4;
													// Play collision sound.
						if (thePlayer.vVel > kDontFlapVel)
							PlayExternalSound(kGrateSound, kGratePriority);
													// If we were falling bones (dead)�
						if (thePlayer.mode == kFalling)
						{							// we'll bounce.
							if ((thePlayer.dest.right - 16) > platformRects[i].right)							{
								thePlayer.hVel = 16;
								if (thePlayer.vVel > 0)
									thePlayer.vVel = -(thePlayer.vVel >> 1);
								else
									thePlayer.vVel = thePlayer.vVel >> 1;
							}
							else if ((thePlayer.dest.left + 16) < platformRects[i].left)
							{
								thePlayer.hVel = -16;
								if (thePlayer.vVel > 0)
									thePlayer.vVel = -(thePlayer.vVel >> 1);
								else
									thePlayer.vVel = thePlayer.vVel >> 1;
							}
							else					// If we were nearly stopped�
							{						// turn into pile of bones.
								PlayExternalSound(kBoom1Sound, kBoom1Priority);
								thePlayer.vVel = 0;
								thePlayer.mode = kBones;
								thePlayer.frame = 22;
								thePlayer.dest.top = thePlayer.dest.bottom - 22;
								thePlayer.v = thePlayer.dest.top << 4;
								thePlayer.srcNum = 10;
							}
						}
						else						// Okay, if we weren't falling bones�
						{							// bounce the player (-1/2 vel.).
							if (thePlayer.vVel > 0)
								thePlayer.vVel = -(thePlayer.vVel >> 1);
							else
								thePlayer.vVel = thePlayer.vVel >> 1;
						}
					}								// If the player was instead moving up�
					else if (thePlayer.wasV > thePlayer.v)
					{								// the player likely hit the bottom of�
													// the ledge.  Keep player below ledge.
						offset = platformRects[i].bottom - thePlayer.dest.top;
						thePlayer.dest.top += offset;
						thePlayer.dest.bottom += offset;
						thePlayer.v = thePlayer.dest.top << 4;
													// Play collision sound.
						PlayExternalSound(kGrateSound, kGratePriority);
						if (thePlayer.vVel < 0)		// Bounce player down (-1/2 vel.).
							thePlayer.vVel = -(thePlayer.vVel >> 1);
						else
							thePlayer.vVel = thePlayer.vVel >> 1;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------  KeepPlayerOnPlatform

// This is an alignment function.  It is called only if the player is standing or�
// walking on a ledge.  It is designed to keep the player's mount's (bird's)�
// feet firmly planted on the ledge.  Consider that, with the addition of gravity�
// to a player's downward velocity, there is a problem where the player can appear�
// to slowly sink down through the ledge.  There may be any number of methods you�
// might want to try to prevent this from becoming a problem in the first place, �
// but my experience has been that all the methods I've tried have flaws - correcting�
// for those flaws points out other flaws and you start getting a messy sort of�
// patchwork.  Should you ever get it to work, the mess that is your function has come�
// to resemble the Knot of ????.

void KeepPlayerOnPlatform (void)
{
	Rect		whoCares;
	short		i, offset;
	
	for (i = 0; i < numLedges; i++)		// For each ledge for this wave�
	{									// test for a collision.
		if ((SectRect(&thePlayer.dest, &platformRects[i], &whoCares)) && (thePlayer.vVel > 0))
		{								// If collided (player sinking), force�
										// player to top of ledge.
			offset = thePlayer.dest.bottom - platformRects[i].top - 1;
			thePlayer.dest.top -= offset;
			thePlayer.dest.bottom -= offset;
			thePlayer.v = thePlayer.dest.top * 16;
		}
	}
	
	if (thePlayer.vVel > 0)				// Set player's vertical velocity to zero.
		thePlayer.vVel = 0;
}

//--------------------------------------------------------------  CheckLavaRoofCollision

// This is a simple high/low test to see if the player has either bounced off�
// the roof of the "arena" or dipped down into the lava below.

void CheckLavaRoofCollision (void)
{
	short		offset;
	
	if (thePlayer.dest.bottom > kLavaHeight)	// See if player in lava.
	{
		if (thePlayer.mode == kFalling)			// If falling (dead), "Splash!"
			PlayExternalSound(kSplashSound, kSplashPriority);
		else									// If flying (alive), "Yeow!"
			PlayExternalSound(kBirdSound, kBirdPriority);
		thePlayer.mode = kSinking;				// Irregardless, player is now sinking.
	}
	else if (thePlayer.dest.top < kRoofHeight)	// See if player hit roof.
	{											// Move player to below roof.
		offset = kRoofHeight - thePlayer.dest.top;
		thePlayer.dest.top += offset;
		thePlayer.dest.bottom += offset;
		thePlayer.v = thePlayer.dest.top * 16;
												// Play collision sound.
		PlayExternalSound(kGrateSound, kGratePriority);
		thePlayer.vVel = thePlayer.vVel / -2;	// Rebound player (-1/2 vel.).
	}
}

//--------------------------------------------------------------  SetAndCheckPlayerDest

// This function keeps our player's screen coordinates and "scaled" coordinates�
// in agreement.  As well, it checks for wrap-around and handles it.

void SetAndCheckPlayerDest (void)
{
	short		wasTall, wasWide;
										// Remember width and height of player.
	wasTall = thePlayer.dest.bottom - thePlayer.dest.top;
	wasWide = thePlayer.dest.right - thePlayer.dest.left;
										// Convert scaled coords to screen coords.
	thePlayer.dest.left = thePlayer.h >> 4;
	thePlayer.dest.right = thePlayer.dest.left + wasWide;
	thePlayer.dest.top = thePlayer.v >> 4;
	thePlayer.dest.bottom = thePlayer.dest.top + wasTall;
	
	if (thePlayer.dest.left > 640)		// Has player left right side of arena?
	{									// Wrap player back to left side of screen.
		OffsetRect(&thePlayer.dest, -640, 0);
		thePlayer.h = thePlayer.dest.left << 4;
		OffsetRect(&thePlayer.wasDest, -640, 0);
	}
	else if (thePlayer.dest.right < 0)	// Else, has player left left side of screen?
	{									// Wrap player around to right side of screen.
		OffsetRect(&thePlayer.dest, 640, 0);
		thePlayer.h = thePlayer.dest.left << 4;
		OffsetRect(&thePlayer.wasDest, 640, 0);
	}
}

//--------------------------------------------------------------  HandleLightning

// Lightning is handled here.  Obelisks are flashed, lightning is generated, �
// lighting strikes, and the lightning counter decremented.  This is pretty�
// nice - we can just set "lightningCount" to a non-zero number and this�
// function will strike lightning every fram until the counter returns to zero.

void HandleLightning (void)
{
	if (lightningCount > 0)						// Is lightning to strik this frame?
	{											// Special frame when obelisks are lit.
		if (lightningCount == kNumLightningStrikes)
			FlashObelisks(TRUE);
		GenerateLightning(lightH, lightV);		// Create new lightning "segments".
		StrikeLightning();						// Draw lightning on screen.
	}
}

//--------------------------------------------------------------  FinishLightning

// This undoes what the lightning did.  It "undraws" the lightning and returns�
// the obelisks to their "non lit" state.  I see that it is HERE where the counter�
// is decremented and not in the function above.

void FinishLightning (void)
{
	if (lightningCount > 0)
	{
		StrikeLightning();						// Undraw lightning (exclusive Or).
		lightningCount--;						// Descrement lightning counter.
		if (lightningCount == 0)				// If this is the last lightning strike�
			FlashObelisks(FALSE);				// return obelisk to normal.
												// "BOOOOM!"
		PlayExternalSound(kLightningSound, kLightningPriority);
	}
}

//--------------------------------------------------------------  HandleCountDownTimer

// This is a pretty boring function.  It is here so that when one level ends,�
// the next one does begin immediately.  It gives the player a few seconds of�
// breathing time.  Essentially, to engage it, we need merely set "countDownTimer"�
// to a positive number.  Each frame the counter gets decremented.  When it�
// reaches zero, the level is advanced to the next wave.

void HandleCountDownTimer (void)
{
	if (countDownTimer == 0)		// If already zero, do nothing.
		return;
	else							// Otherwise, if greater than zero�
	{
		countDownTimer--;			// decrememnt counter.
		if (countDownTimer == 0)	// Did it just hit zero?
		{
			countDownTimer = 0;		// Well, just to be sure (dumb line of code).
			levelOn++;				// Increment the level (wave) we're on.
			UpdateLevelNumbers();	// Display new level on screen.
			SetUpLevel();			// Set up the platforms.
			GenerateEnemies();		// Ready nemesis.
		}
	}
}

//--------------------------------------------------------------  PlayGame

// Here is the "core" of the "game loop".  When a player has elected to�
// begin a game, Glypha falls into this function and remains in a loop�
// herein until the player either quits, or loses their last "bird".
// Each pass through the main loop below constitutes one "frame" of the game.

void PlayGame (void)
{
	#define		kTicksPerFrame		2L
	Point		offsetPt;
	long		waitUntil;
	
	offsetPt.h = 0;								// Set up ShieldCursor() point.
	offsetPt.v = 20;
	ShieldCursor(&mainWindowRect, offsetPt);	// Hide the cursor. 
	waitUntil = TickCount() + kTicksPerFrame;	// Set up speed governor variable.
	
	do											// Main game loop!!!!
	{	
		MovePlayer();							// Move the player's bird.
		MoveEnemies();							// Move all sphinx enemies.
		HandleHand();							// Handle the mummy hand (may do nothing).
		HandleEye();							// Handle eye (probably will do nothing).
		DrawFrame();							// Draw the whole scene for this frame.
		HandleLightning();						// Draw lightning (is applicable).
		do										// Here is where the speed is governed.
		{
		} while (TickCount() < waitUntil);
		waitUntil = TickCount() + kTicksPerFrame;
		evenFrame = !evenFrame;					// Toggle "evenFrame" variable.
		
		GetPlayerInput();						// Get the player's input (keystrokes).
		HandleCountDownTimer();					// Handle countdown (may do nothing).
		FinishLightning();						// Undraw lightning (if it needs undoing).
	}
	while ((playing) && (!pausing));			// Stay in loop until dead, paused or quit.
	
	if ((!playing) && (!quitting))				// If the player died!
	{											// Then play some sweet music.
		PlayExternalSound(kMusicSound, kMusicPriority);
		CheckHighScore();						// And see if they're on the high scores.
	}
	
	ShowCursor();								// Before we go, restore the cursor.
	MenusReflectMode();							// Set the menus grayed-out state correctly.
	FlushEvents(everyEvent, 0);					// Flush any events in the queue.
}

//--------------------------------------------------------------  CheckHighScore

// This function handles testing to see if the player's score is in the �
// high scores.  If that is the case, the function prompts the user for�
// a name to enter, and sorts and stores off the new score list.

void CheckHighScore (void)
{
	#define		kHighNameDialogID	130
	Str255		placeStr, tempStr;
	DialogPtr	theDial;
	short		i, item;
	Boolean		leaving;
	
	if (theScore > thePrefs.highScores[9])		// To see if on high scores, we need�
	{											// merely see if the last guy is beat out.
		openTheScores = TRUE;					// Will automatically bring up high scores.
												// Play some congratulatory music.
		PlayExternalSound(kBonusSound, kMusicPriority - 1);
		i = 8;									// Find where new score fits in list.
		while ((theScore > thePrefs.highScores[i]) && (i >= 0))
		{										// We'll bump everyone down as we look.
			thePrefs.highScores[i + 1] = thePrefs.highScores[i];
			thePrefs.highLevel[i + 1] = thePrefs.highLevel[i];
			PasStringCopy(thePrefs.highNames[i], thePrefs.highNames[i + 1]);
			i--;
		}
		
		i++;									// i is our place in list (zero based).
		thePrefs.highScores[i] = theScore;		// Pop the new score in place.
		thePrefs.highLevel[i] = levelOn + 1;	// Drop in the new highest level.
		
		NumToString((long)i + 1L, placeStr);	// Convert place to a string to display�
		ParamText(placeStr, "\p", "\p", "\p");	// in the dialog (via ParamText()).
		
		InitCursor();							// Show cursor.
		CenterDialog(kHighNameDialogID);		// Center the dialog and then bring it up.
		theDial = GetNewDialog(kHighNameDialogID, 0L, kPutInFront);
		SetPort((GrafPtr)theDial);
		ShowWindow((GrafPtr)theDial);			// Make dialog visible.
		DrawDefaultButton(theDial);				// Draw outline around "Okay" button.
		FlushEvents(everyEvent, 0);				// Flush any events queued up.
												// Put a default name in text edit box.
		SetDialogString(theDial, 2, thePrefs.highName);
		SelIText(theDial, 2, 0, 1024);			// Select the whole text edit string.
		leaving = FALSE;						// Flag for noting when player hit "Okay".
		
		while (!leaving)						// Simple modal dialog loop.
		{
			ModalDialog(0L, &item);				// Use standard filtering.
			
			if (item == 1)						// If player hit the "Okay" button�
			{									// Get the name entered in text edit box.
				GetDialogString(theDial, 2, tempStr);
												// Copy the name into high score list.
				PasStringCopyNum(tempStr, thePrefs.highNames[i], 15);
				PasStringCopy(thePrefs.highNames[i], thePrefs.highName);
				leaving = TRUE;					// We're gone!
			}
		}
		
		DisposDialog(theDial);					// Clean up.
	}
	else										// But if player didn't get on high scores�
		openTheScores = FALSE;					// no need to rub their face in it.
}

