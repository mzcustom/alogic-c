#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "raylib.h" 

typedef int64_t i64;
typedef int32_t i32; 
typedef double f64;  
typedef float f32;  
typedef uint8_t u8;   
typedef Vector2 Vec2;

#define SWAP(A, B)  do { typeof(A) temp = A; A = B; B = temp; } while(0)

// Constants
#define FPS                      60
#define WINDOW_WIDTH             560
#define WINDOW_HEIGHT            800
#define UPPER_LAND_HEIGHT        WINDOW_WIDTH
#define MARGIN_HEIGHT            WINDOW_HEIGHT / 40
#define MARGIN_WIDTH             MARGIN_HEIGHT
#define TITLE_WIDTH              WINDOW_WIDTH * 0.75f
#define TITLE_HEIGHT             WINDOW_HEIGHT * 0.2f
#define MIN_TITLE_HEIGHT         TITLE_HEIGHT * 0.5f
#define TITLE_LANDING_Y          MARGIN_HEIGHT + ROW_HEIGHT
#define NUM_ROW                  4
#define NUM_COL                  4
#define ROW_HEIGHT               (UPPER_LAND_HEIGHT - 2*MARGIN_HEIGHT) / NUM_ROW
#define COL_WIDTH                (WINDOW_WIDTH - 2*MARGIN_WIDTH) / NUM_COL
#define ANIM_SIZE                ROW_HEIGHT * 0.65f
#define DUST_IMAGE_WIDTH         320
#define DUST_IMAGE_HEIGHT        256
#define MIN_ANIM_HEIGHT          WINDOW_HEIGHT / 160
#define MIN_JUMP_HEIGHT          MIN_ANIM_HEIGHT * 3
#define JUMP_SCALE_INC_RATE      0.075f
#define MAX_DUST_DURATION        FPS / 3
#define FRONT_ROW_Y              MARGIN_HEIGHT + (NUM_ROW-1)*ROW_HEIGHT + ROW_HEIGHT/2
#define RESQUE_SPOT_X            MARGIN_WIDTH + (WINDOW_WIDTH-2*MARGIN_WIDTH)/2
#define RESQUE_SPOT_Y            (UPPER_LAND_HEIGHT + 7*MARGIN_HEIGHT) + (WINDOW_HEIGHT-(UPPER_LAND_HEIGHT+7*MARGIN_HEIGHT))/2
#define DEFAULT_FONT_SIZE        MARGIN_WIDTH * 1.2f
#define MAX_MSG_LEN              DEFAULT_FONT_SIZE * 2
#define MSG_POS_Y                UPPER_LAND_HEIGHT - MARGIN_HEIGHT
#define BOARD_SIZE               NUM_ROW * NUM_COL
#define FRONT_ROW_BASEINDEX      BOARD_SIZE - NUM_COL
#define NUM_COLOR                4
#define NUM_KIND                 4
#define NUM_GAME_MODE            5

#define TOTAL_BIG_JUMP           2
#define INDEFINITE              -1

// Raylib input int32 map
#define KEY_A        65
#define KEY_S        83
#define KEY_D        68
#define KEY_F        70
#define KEY_G        71
#define KEY_Q        81
#define KEY_SPACE    32
#define MOUSE_LEFT   0
#define MOUSE_RIGHT  1
#define KEY_RIGHT    262
#define KEY_LEFT     263
#define KEY_DOWN     264
#define KEY_UP       265

// GameMode for each game scene
typedef enum {
	TITLE_MODE = 1,
	OPENING_MODE = 2,
	GAME_PLAY_MODE = 3,
	GAME_CLEAR_MODE = 4,
	GAME_OVER_MODE = 5 
} GameMode;

// Assets
typedef enum {
    TITLE_TEXTURE,
    GROUND_TEXTURE,
    ANIMALS_TEXTURE,
    DUST_TEXTURE,
    TEXTURE_COUNT
} TextureType;

// typedef struct {
// 	Texture2D TitleTexture;   
// 	Texture2D GroundTexture;  
// 	Texture2D AnimalsTexture; 
// 	Texture2D DustTexture;    
// } Textures;

typedef enum {
    TITLE_JUMP_SOUND,
    TITLE_LAND_SOUND,
    START_SOUND,
    JUMP_SOUND,
    BIG_JUMP_SOUND,
    LAND_SOUND,
    BIG_LAND_SOUND,
    SUCCESS_SOUND,
    FAIL_SOUND,
    YAY_SOUND,
    SOUND_COUNT
} SoundType;

// typedef struct {
// 	Sound TitleJump;    
// 	Sound TitleLand;    
// 	Sound Start;        
// 	Sound Jump;         
// 	Sound BigJump;      
// 	Sound Land;         
// 	Sound BigLand;      
// 	Sound Success;      
// 	Sound Fail;         
// 	Sound Yay;         
// } Sounds;

// Message board system
typedef struct {
	char* l1, l2;
	int duration, frames;
	bool displayed;
	u8 alpha;
	GameMode gameMode;
} Message; 

//typedef struct {
//	msgs [NUM_GAME_MODE][]Message
//} Scripts; 

// TITLE GameMode states
typedef struct {
	Vec2 destForOpening[3];
	int titleFrame;          
	int animToDrop;          
	int titleDropFrame;      
	int firstCompressFrame;  
	int lastAnimDropFrame;   
	int titlePressFrame;     
	int lastAnimJumpFrame;   
	int secondCompressFrame; 
	int fallOutFrame;        
	int titleMessageShown;   
	int sceneEnd;            
} TitleState;

// Title logo - accel, veloc and press in pixels/frame.
typedef struct {
	Vec2 pos;    
	Vec2 dest;   
	Vec2 accel;  
	Vec2 veloc;  
	f32 height; 
	f32 press;  
} TitleLogo;

// animType - Upper 4 bits are color in the order of Blue, Green, Red, Yellow
//            Lower 4 bits are kind in the order of Cat, Giraffe, Owl, Panda.
//            ex) 0100 0010 - Green Owl, 0010 1000 - Red Cat
//            In the Animal texture atlas, color maps to the row(y), and kind does to the column(x)
//            ex) 0100 0010 - The 2nd row from the top, 3rd column from the left 
typedef struct {
	Vec2 pos;             
	Vec2 dest;            
	Vec2 accel;           
	Vec2 veloc;           
	f32 scale;           
	f32 height;          
	f32 press;           
	f32 scaleDecRate;    
	u8 dustDuration;    
	u8 totalJumpFrames; 
	u8 ascFrames;       
	u8 currJumpFrame;   
	u8 animType;        
} Animal;

// Global Variables
static Texture textures[TEXTURE_COUNT]; 
static Sound sounds[SOUND_COUNT]; 
static GameMode gameMode;
static Message msg; 
//Scripts scripts 

// Vec2 math
static inline bool IsVec2Zero(Vec2 v)              { return v.x == 0 || v.y == 0; }
static inline Vec2 Vec2Add(Vec2 v1, Vec2 v2)       { return (Vec2){v1.x + v2.x, v1.y + v2.y}; }
static inline Vec2 Vec2Sub(Vec2 v1, Vec2 v2)       { return (Vec2){v1.x - v2.x, v1.y - v2.y}; }
static inline f32 Vec2LenSq(Vec2 v)                { return v.x*v.x + v.y*v.y; }
static inline f32 Vec2DistSq(Vec2 v1, Vec2 v2)     { return Vec2LenSq(Vec2Sub(v1, v2)); }

// return -1 if target is 0(no 1 bit). The Lowest bit is order 0.
int findFirst1Bit(u8 target) {
    if (target == 0) return -1;

	int order = 0;
	u8 b = 1; 
	while ((target&b) == 0) {
		b <<= 1;
		order++;
	}
	return order;
}

void setTitleLogo(TitleLogo *title) {
	title->pos.x = (WINDOW_WIDTH - TITLE_WIDTH) * 0.5f;
	title->pos.y = -TITLE_HEIGHT;
	title->height = TITLE_HEIGHT;
	title->dest = (Vec2){title->pos.x, TITLE_LANDING_Y};
}

bool updateTitle(TitleLogo *title) {
	bool isUpdated = true;

	if (title->press > 0) {
		isUpdated = false;
		title->height -= title->press;
		if (title->height < MIN_TITLE_HEIGHT) {
			title->height = MIN_TITLE_HEIGHT;
			title->press = -1;
		} else {
			title->press /= 1.75f;
			if (title->press < 0.0001f) {
				title->press = -1;
			}
		}
	}
	if (title->press < 0) {
		isUpdated = false;
		title->height -= title->press;
		if (title->height >= TITLE_HEIGHT) {
			title->height = TITLE_HEIGHT;
			title->press = 0;
		} else {
			title->press *= 1.75f;
		}
	}

	// Update Pos and Veloc if it's moving or has accel
	if ((!IsVec2Zero(title->veloc) || !IsVec2Zero(title->accel)) && (title->press == 0)) {
		isUpdated = false;
		title->pos = Vec2Add(title->pos, title->veloc);
		title->veloc = Vec2Add(title->veloc, title->accel);

		// take care of landing
		if (title->veloc.y > 0 && Vec2DistSq(title->pos, title->dest) < Vec2LenSq(title->veloc)/2) {
			title->press = title->veloc.y/2;
			title->pos = title->dest;
			if (title->veloc.y >= 2) {
				title->veloc = (Vec2){0, -title->veloc.y/2};
			} else {
				title->press = 0;
				title->veloc = (Vec2){};
				title->accel = (Vec2){};
			}
		}
	}
	return isUpdated;
}

void drawTitle(TitleLogo *title) {
	Rectangle srcRect = (Rectangle){0, 0, TITLE_WIDTH, TITLE_HEIGHT};
	Vec2 desPos = (Vec2){title->pos.x, title->pos.y + TITLE_HEIGHT - title->height};
	Rectangle desRect = (Rectangle){desPos.x, desPos.y, TITLE_WIDTH, title->height};
	DrawTexturePro(textures[TITLE_TEXTURE], srcRect, desRect, (Vec2){}, 0, RAYWHITE);
}

void setAnimals(Animal *animal) {
	u8 color = 1; 
    u8 kind = 1;

	for (int row = 0; row < NUM_ROW; row++) {
		u8 colorBit = color << NUM_KIND;
		for (int col = 0; col < NUM_COL; col++) {
			int animalIndex = row*NUM_COL + col;
			animal[animalIndex].height = ANIM_SIZE;
			animal[animalIndex].animType = colorBit | kind;
			animal[animalIndex].scale = 1;
			kind <<= 1;
		}
		kind = 1;
		color <<= 1;
	}
}

void shuffleBoard(Animal **board, Vec2 *frontRowPos) {
	for (int i = 0; i < BOARD_SIZE-2; i++) {
		int indexToSwap = GetRandomValue(i+1, BOARD_SIZE-1);
		// SWAP macro doens't compile to WASM with following error:
        //                  call to undeclared function 'typeof'; ISO C99 and later do not support      
        //                  implicit function declarations [-Wimplicit-function-declaration]
        Animal *temp = board[i]; board[i] = board[indexToSwap]; board[indexToSwap] = temp;
	}
    Animal *temp = board[0]; board[0] = board[BOARD_SIZE-1]; board[BOARD_SIZE-1] = temp;

    // Set the position(up high out of screen for the each stage) and landing destination for animals
	for (int row = 0; row < NUM_COL; row++) {
		f32 posY = (f32)(MARGIN_HEIGHT + (row * ROW_HEIGHT) + (ROW_HEIGHT / 2));
		for (int col = 0; col < NUM_ROW; col++) {
			int boardIndex = row*NUM_COL + col;
			f32 posX = (f32)(MARGIN_WIDTH + (col * COL_WIDTH) + (COL_WIDTH / 2));
			board[boardIndex]->dest = (Vec2){posX, posY};
			board[boardIndex]->pos = (Vec2){posX, posY - (f32)(4*ROW_HEIGHT)};
		}
	}
    // Set separate front row position array
	for (int i = 0; i < NUM_COL; i++) {
		int frontRowIndex = i + BOARD_SIZE - NUM_COL;
		frontRowPos[i] = board[frontRowIndex]->dest;
	}
}

// Returns the number of animals that could be chosen from the front row(last NUM_COL elements of
// the board) and fills the nextAnimIndexes array with the indexes of them
int findResquables(Animal **board, u8 mostRecentRescuedType, int *nextAnimIndexes) {
	int nextAnimNum = 0;
	int frontRowOffset = BOARD_SIZE - NUM_COL;

	for (int i = 0; i < NUM_COL; i++) {
		if (board[i+frontRowOffset] && (board[i+frontRowOffset]->animType&mostRecentRescuedType)) {
			nextAnimIndexes[i] = i + frontRowOffset;
			nextAnimNum++;
		}
	}

	return nextAnimNum;
}

// totalFrames: the total duration of the jump in frames.
// ascFrames: the duration of animal moving upward ing in frames.
// if dest.Y is greater than anim.pos.Y, ascFrames has to be less than totalFrames/2
// and dest.Y is less than anim.pos.Y, ascFrames has to be greater than totalFrmaes/2
// TotalFrames CANNOT be exactly 2*ascFrames otherwise divided by 0 occurs
void jumpAnimal(Animal *anim, Vec2 dest, f32 totalFrames, f32 ascFrames) {

	// desFrames: frames left at the its original position while desending to dest
	f32 desFrames = totalFrames - ascFrames;

	//veloc = -accel * ascFrames (accel is positive)
	//leastAlt = anim.pos.Y + 0.5*ascFrames*veloc (veloc is negative)
	//dest.Y = leastAlt + 0.5*accel*(desFrame)^2
	f32 diff = 0;
	if (anim->pos.y == dest.y) {
		diff = -ANIM_SIZE;
	} else {
		diff = dest.y - anim->pos.y;
	}
	anim->accel = (Vec2){0, 2 * diff / (desFrames*desFrames - ascFrames*ascFrames)};
	anim->veloc = (Vec2){(dest.x - anim->pos.x) / totalFrames, -anim->accel.y * ascFrames};
	anim->dest = dest;
	anim->totalJumpFrames = (u8)totalFrames;
	anim->ascFrames = (u8)ascFrames;
	anim->currJumpFrame = 0;

	if (anim->height < ANIM_SIZE) {
		anim->press = -anim->height / 10;
	}

	if (anim->dest.x == RESQUE_SPOT_X && anim->dest.y == RESQUE_SPOT_Y) {
		if (gameMode == GAME_PLAY_MODE && anim->height <= MIN_JUMP_HEIGHT) {
			PlaySound(sounds[BIG_JUMP_SOUND]);
		} else if (gameMode != GAME_CLEAR_MODE) {
			PlaySound(sounds[JUMP_SOUND]);
		}
	}
}


void resqueAt(Animal **board, Animal **resqued, int resqueIndex, int numAnimalLeft) {
	Animal *anim = board[resqueIndex];
	assert(anim->animType);

	resqued[BOARD_SIZE-numAnimalLeft] = anim;

    // Jump the resqued animal across the river
    // Animal.Height depends on the how long the user holds the button(squeeze) on the selected animal
    // the more squeezed, the higher it jumps with a greater ascFrames.
	f32 ascFrames = 2.5f * ANIM_SIZE / anim->height;
	f32 totalFrames = FPS/3 + ascFrames;
	if (ascFrames < FPS/15) {
		ascFrames = FPS/15;
		totalFrames = FPS/3;
	}
	jumpAnimal(anim, (Vec2){RESQUE_SPOT_X, RESQUE_SPOT_Y}, totalFrames, ascFrames);

	// Advance the row of each animal from the column where the rescued animal was
	int currIndex = resqueIndex;
	while ((currIndex >= 0) && board[currIndex]) {
		if (currIndex < NUM_COL || board[currIndex-NUM_COL]) {
			board[currIndex] = 0; 
			break;
		} else {
			board[currIndex] = board[currIndex-NUM_COL];
			anim = board[currIndex];
			jumpAnimal(anim, (Vec2){anim->pos.x, anim->pos.y + ROW_HEIGHT}, FPS/3, FPS/10);
		}
		currIndex -= NUM_COL;
	}
}

void drawAnimal(Animal *anim) {
	u8 colorBitfield = anim->animType >> NUM_KIND;
	u8 kindBitfield = anim->animType & 0b1111;
	u8 colorOffset = NUM_COLOR - 1 - findFirst1Bit(colorBitfield);
	u8 kindOffset = NUM_KIND - 1 - findFirst1Bit(kindBitfield);
	Rectangle srcRect = (Rectangle){(f32)(kindOffset) * ANIM_SIZE, (f32)(colorOffset) * ANIM_SIZE,
		                          ANIM_SIZE, ANIM_SIZE};

	if (anim->totalJumpFrames > 0) {
		if (anim->currJumpFrame <= anim->ascFrames) {
			anim->scale += JUMP_SCALE_INC_RATE;
			if (anim->currJumpFrame == anim->ascFrames) {
				anim->scaleDecRate = (anim->scale - 1) / (f32)(anim->totalJumpFrames - anim->ascFrames);
			}
		} else {
			anim->scale -= anim->scaleDecRate;
			if (anim->scale < 1) {
				anim->scale = 1;
			}
		}
	}

	f32 sc = anim->scale;
	Vec2 animOrigin = Vec2Sub(anim->pos, (Vec2){sc*ANIM_SIZE*0.5f, sc*ANIM_SIZE*0.5f});
	Vec2 desPos = (Vec2){animOrigin.x, animOrigin.y + sc*ANIM_SIZE - sc*anim->height};
	Rectangle desRect = (Rectangle){desPos.x, desPos.y, sc*ANIM_SIZE, sc*anim->height};

	DrawTexturePro(textures[ANIMALS_TEXTURE], srcRect, desRect, (Vec2){}, 0, RAYWHITE);

	// Draw dust
	if (anim->dustDuration != 0) {
		Rectangle srcRect = (Rectangle){0, 0, DUST_IMAGE_WIDTH, DUST_IMAGE_HEIGHT};
		Rectangle desRect = (Rectangle){anim->pos.x - ANIM_SIZE*0.7, anim->pos.y + ANIM_SIZE*0.4,
			                ANIM_SIZE * 0.5, ANIM_SIZE * 0.15};
		DrawTexturePro(textures[DUST_TEXTURE], srcRect, desRect, (Vec2){}, 0, RAYWHITE);

		desRect = (Rectangle){anim->pos.x + ANIM_SIZE*0.2, anim->pos.y + ANIM_SIZE*0.4,
			                ANIM_SIZE * 0.5, ANIM_SIZE * 0.15};
		DrawTexturePro(textures[DUST_TEXTURE], srcRect, desRect, (Vec2){}, 0, RAYWHITE);
		anim->dustDuration -= 1;
	}
}

bool isAnimRectClicked(Animal *anim) {
	if (!anim) return false;

	f32 mouseX = (f32)(GetMouseX());
	f32 mouseY = (f32)(GetMouseY());
	f32 animPosX = anim->pos.x;
	f32 animPosY = anim->pos.y;
	f32 halfLength = ANIM_SIZE+0.5f;

	//if DEBUG {
	//	printf("Mouse Clicked at : %d, %d\n", mouseX, mouseY);
	//	printf("AnimPos : %d, %d\n", animPosX, animPosY);
	//}

	return (mouseX >= animPosX-halfLength) && (mouseX <= animPosX+halfLength) &&
		(mouseY >= animPosY-halfLength) && (mouseY <= animPosY+halfLength);
}


void loadAssets() {
	Image titleImage = LoadImage("assets/textures/title.png");
	Image groundImage = LoadImage("assets/textures/background.png");
	Image animalsImage = LoadImage("assets/textures/animals.png");
	Image dustImage = LoadImage("assets/textures/dust.png");

	ImageResize(&titleImage, TITLE_WIDTH, TITLE_HEIGHT);
	ImageResize(&groundImage, WINDOW_WIDTH, WINDOW_HEIGHT);
	ImageResize(&animalsImage, (i32)(ANIM_SIZE*NUM_COL), (i32)(ANIM_SIZE*NUM_ROW));

	textures[TITLE_TEXTURE] = LoadTextureFromImage(titleImage);
	textures[GROUND_TEXTURE] = LoadTextureFromImage(groundImage);
	textures[ANIMALS_TEXTURE] = LoadTextureFromImage(animalsImage);
	textures[DUST_TEXTURE] = LoadTextureFromImage(dustImage);

	UnloadImage(titleImage);
	UnloadImage(groundImage);
	UnloadImage(animalsImage);
	UnloadImage(dustImage);

	sounds[TITLE_JUMP_SOUND] = LoadSound("assets/sounds/titlejump.mp3");
	sounds[TITLE_LAND_SOUND] = LoadSound("assets/sounds/titleland.mp3");
	sounds[START_SOUND] = LoadSound("assets/sounds/start.mp3");
	sounds[JUMP_SOUND] = LoadSound("assets/sounds/jump.wav");
	sounds[BIG_JUMP_SOUND] = LoadSound("assets/sounds/bigjump.mp3");
	sounds[LAND_SOUND] = LoadSound("assets/sounds/land.mp3");
	sounds[BIG_LAND_SOUND] = LoadSound("assets/sounds/bigland.mp3");
	sounds[SUCCESS_SOUND] = LoadSound("assets/sounds/success.mp3");
	sounds[FAIL_SOUND] = LoadSound("assets/sounds/fail.mp3");
	sounds[YAY_SOUND] = LoadSound("assets/sounds/yay.mp3");
}

void unloadSounds() {
	for (int i = 0; i < SOUND_COUNT; i++) {
		UnloadSound(sounds[i]);
	}
}

int findLastRowEmpties(Animal **board, bool *empties) {
	int emptyColCount = 0;
	for (int i = 0; i < NUM_COL; i++) {
		if (!board[i]) {
			empties[i] = true;
			emptyColCount++;
		}
	}
	return emptyColCount;
}

void moveResquedToBoard(Animal **board, Animal **resqued, int boardIndex, int resquedIndex,
	                    int *numAnimalLeft, bool *resquedChanged) {
	Animal *animToPush = resqued[resquedIndex];
	resqued[resquedIndex] = 0;

	int currIndex = boardIndex;
	while (currIndex >= 0) {
		Animal *nextAnimToPush = board[currIndex];
		board[currIndex] = animToPush;
		if (nextAnimToPush) {
			jumpAnimal(nextAnimToPush, (Vec2){nextAnimToPush->pos.x, nextAnimToPush->pos.y - ROW_HEIGHT},
				       20, 12);
			animToPush = nextAnimToPush;
			currIndex -= NUM_COL;
		} else {
			break;
		}
	}

	*numAnimalLeft += 1;
	*resquedChanged = true;
}

void scatterResqued(Animal **board, Animal **resqued, int maxIndexToScatter,
	                Vec2 *frontRowPos, int *numAnimalLeft, bool *resquedChanged) {
	bool lastRowEmptyIndices[NUM_COL] = {};
	int emptyCount = findLastRowEmpties(board, lastRowEmptyIndices);
	int indexToMoveToBoard = maxIndexToScatter;

	for (int i = 0; i < NUM_COL; i++) {
		if (lastRowEmptyIndices[i]) {
			jumpAnimal(resqued[indexToMoveToBoard], frontRowPos[i], 24, 16);
			int frontRowIndexToJump = BOARD_SIZE - NUM_COL + i;
			moveResquedToBoard(board, resqued, frontRowIndexToJump, indexToMoveToBoard,
				               numAnimalLeft, resquedChanged);
			indexToMoveToBoard--;
			emptyCount--;
			if ((indexToMoveToBoard < 0) || (emptyCount < 1)) {
				break;
			}
		}
	}

	resqued[indexToMoveToBoard+1] = resqued[maxIndexToScatter+1];
	resqued[maxIndexToScatter+1] = 0;
}

bool updateAnimState(Animal *animals, Animal **board, Animal **resqued, Vec2 *frontRowPos, 
                     int *numAnimalLeft, bool *resquedChanged, bool *bigJumpMade, int *bigJumpLeft, 
                     bool lastMsgShown) {
	bool isAllUpdated = true;

	for (int i = 0; i < BOARD_SIZE; ++i) {
		Animal *anim = &animals[i];
		// Update Press and Height
		if (anim->press > 0) {
			anim->height -= anim->press;
			if (anim->height < MIN_ANIM_HEIGHT) {
				anim->height = MIN_ANIM_HEIGHT;
			}
			anim->press *= 0.5f;
			if (anim->press < 0.0001f) {
				anim->press = -1;
			}
		}
		if (anim->press < 0) {
			if (anim->height-anim->press >= ANIM_SIZE) {
				anim->height = ANIM_SIZE;
				anim->press = 0;
			} else {
				anim->height -= anim->press;
				anim->press *= 2;
			}
		}

		// Update Pos and Veloc if it's moving or has accel
		if ((!IsVec2Zero(anim->veloc)) || (!IsVec2Zero(anim->accel))) {
			isAllUpdated = false;
			Vec2 pos = anim->pos;
			Vec2 veloc = anim->veloc;
			anim->pos = Vec2Add(pos, veloc);
			anim->veloc = Vec2Add(anim->veloc, anim->accel);
			if (anim->totalJumpFrames > 0) {
				anim->currJumpFrame++;
			}

			// Take care of landing
			if (Vec2DistSq(anim->pos, anim->dest) < Vec2LenSq(anim->veloc)/2) {
				if ((gameMode == GAME_PLAY_MODE) &&
					(anim->ascFrames > anim->totalJumpFrames/2) && (anim->dest.y == FRONT_ROW_Y)) {
					PlaySound(sounds[YAY_SOUND]);
				}
				if (anim->veloc.y <= FPS) {
					if (anim->veloc.y > FPS/2) {
						anim->press = anim->veloc.y / 3;
						if (gameMode == GAME_PLAY_MODE) {
							PlaySound(sounds[LAND_SOUND]);
						}
					}
				} else {
					anim->press = anim->veloc.y / 3;
					anim->dustDuration = MAX_DUST_DURATION;
					if ((anim->dest.x == RESQUE_SPOT_X) && (anim->dest.y == RESQUE_SPOT_Y)) {
						PlaySound(sounds[BIG_LAND_SOUND]);
					} else {
						PlaySound(sounds[LAND_SOUND]);
					}
				}
				// if the landing animal is the last resqued(the one crossing the bridge)
				int lastResquedIndex = BOARD_SIZE - 1 - *numAnimalLeft;
				if ((gameMode == GAME_PLAY_MODE) && (lastResquedIndex > 0) && 
                    (anim == resqued[lastResquedIndex])) {
					// when a big jump is made, send previously resqued animals back to the land
					if ((anim->veloc.y > FPS) && (*bigJumpLeft > 0)) {
						*bigJumpMade = true;
						scatterResqued(board, resqued, lastResquedIndex-1, frontRowPos,
							           numAnimalLeft, resquedChanged);
						*bigJumpLeft -= 1;
//						if lastMsgShown && *bigJumpLeft == 1 {
//							setMsg(GAME_PLAY, 3);
						//}
						//if *bigJumpLeft == 0 {
						//	setMsg(GAME_PLAY, 4);
						//}
					} else {
						// For regular jumps, compress and move the previously resqued sideway
						int prevAnimIndex = lastResquedIndex - 1;
						Animal *prevAnim = resqued[prevAnimIndex];
						prevAnim->press = ANIM_SIZE;
						f32 pushFactor = (f32)(prevAnimIndex*0.5f + 1);
						if (prevAnimIndex%2 == 0) {
							prevAnim->dest = Vec2Sub(prevAnim->pos,
								                    (Vec2){pushFactor * ANIM_SIZE * 0.25f, 0});
							prevAnim->accel = (Vec2){pushFactor * ANIM_SIZE * 0.075f, 0};
							prevAnim->veloc = (Vec2){pushFactor * -ANIM_SIZE * 0.15f, 0};
						} else {
							prevAnim->dest = Vec2Add(prevAnim->pos,
								                    (Vec2){pushFactor * ANIM_SIZE * 0.25, 0});
							prevAnim->accel = (Vec2){pushFactor * -ANIM_SIZE * 0.075f, 0};
							prevAnim->veloc = (Vec2){pushFactor * ANIM_SIZE * 0.15f, 0};
						}
					}
				}

				anim->pos = anim->dest;
				anim->veloc = (Vec2){}; 
                anim->accel = (Vec2){};
				anim->scale = 1;
				anim->scaleDecRate = 0;
				anim->totalJumpFrames = 0;
				anim->ascFrames = 0;
				anim->currJumpFrame = 0;
			}
		}
	}

	return isAllUpdated;
}

void resetState(Animal *animals, Animal **board, Animal **resqued, Vec2 *frontRowPos) {
	memset(animals, 0, sizeof(Animal)*BOARD_SIZE);
	setAnimals(animals);
	memset(resqued, 0, sizeof(animals)*BOARD_SIZE);

	for (int i = 0; i < BOARD_SIZE; i++) {
		board[i] = &animals[i];
	}
	shuffleBoard(board, frontRowPos);
}

void processKeyDown(Animal *anim) {
	anim->press = anim->height*0.05f;
	if (anim->height < MIN_JUMP_HEIGHT) {
		anim->height = MIN_JUMP_HEIGHT;
	}
}

void setTitleAnims(Animal **titleAnims, TitleState *tstate) {
	for (int i = 0; i < 3; i++) {
		tstate->destForOpening[i] = titleAnims[i]->dest;
	}

	titleAnims[0]->dest = (Vec2){WINDOW_WIDTH / 4 * 3, TITLE_LANDING_Y + TITLE_HEIGHT - ANIM_SIZE/3};
	titleAnims[1]->dest = (Vec2){WINDOW_WIDTH / 4 * 1.5f, TITLE_LANDING_Y + TITLE_HEIGHT - ANIM_SIZE/3};
	titleAnims[2]->dest = (Vec2){WINDOW_WIDTH / 2, TITLE_LANDING_Y - TITLE_HEIGHT/4 + ANIM_SIZE/2};
}

int main() {
    TitleLogo title = (TitleLogo){};
	setTitleLogo(&title);

	Animal animals[BOARD_SIZE] = {};
	Animal *board[BOARD_SIZE] = {}; 
	Animal *resqued[BOARD_SIZE] = {}; 
	Vec2 frontRowPos[NUM_COL] = {}; 
	resetState(animals, board, resqued, frontRowPos);

	TitleState tstate = {};
	int firstRow = BOARD_SIZE - NUM_COL;
	Animal *titleAnims[3] = {board[firstRow], board[firstRow+2], board[firstRow+1]};
	setTitleAnims(titleAnims, &tstate);

	int numAnimalLeft = BOARD_SIZE;
	int resquableIndex[NUM_COL] = {};
	bool isTitleUpdated = true;
	bool isAllAnimUpdated = true;
	bool isQuitting = false;
	gameMode = TITLE_MODE;
	int openingFrame = 0;
	int gameClearFrame = 0;
	int bigJumpLeft = TOTAL_BIG_JUMP;
	bool willReplay = false;
	bool firstMoveMade = false;
    bool bigJumpMade = false;
    bool lastMsgShown = false;
    
	bool resquedChanged = true;
	bool mostRecentResqueType = (u8)(0xFF); // initially, all front row animals can be resqued.
	int numPossibleMoves = findResquables(board, mostRecentResqueType, resquableIndex);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Animal Logic");
    SetTargetFPS(FPS);
    InitAudioDevice();
    loadAssets();

    while (!isQuitting && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Let's port port port!", 20, 20, 20, BLACK);
        EndDrawing();
    }
    
    CloseWindow();
    unloadSounds();

    return 0;
}
