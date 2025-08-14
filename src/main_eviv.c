#include "global.h"
#include "menu.h"
#include "task.h"
#include "constants/songs.h"
#include "bg.h"
#include "malloc.h"
#include "palette.h"
#include "sprite.h"
#include "window.h"
#include "string_util.h"
#include "sprite.h"
#include "event_data.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/moves.h"
#include "party_menu.h"
#include "scanline_effect.h"
#include "gpu_regs.h"
#include "new_menu_helpers.h"
#include "field_effect.h"
#include "text_window.h"
#include "overworld.h"
#include "field_weather.h"
#include "trainer_pokemon_sprites.h"

#include "pokemon_summary_screen.h"
#include "menu_helpers.h"
#include "link.h"

#include "main_eviv.h"


typedef struct Evolution EvolutionTableT[EVOS_PER_MON];
#define gEvolutionTable ((EvolutionTableT*) *((u32*) 0x8042F6C))

#define FLAG_BRENDAN_FINAL 0x991

#define FLAG_GIOVANNI_BOSS 0x982
#define FLAG_ARCHER_ARIANA_B2B 0x981

#define FLAG_MAY_BOSS 0x998


#define FLAG_ARCHER_ARIANA_TAG 0x955
#define FLAG_ARCHER_MT_MOON 0x201
#define FLAG_HARDCORE_MODE 0x1034
#define FLAG_EASY_MODE 0x1033
#define FLAG_CHRIS_KAIZO 0x10D1
enum EvolutionMethods
{
	EVO_NONE = 0,
	EVO_FRIENDSHIP,
	EVO_FRIENDSHIP_DAY,
	EVO_FRIENDSHIP_NIGHT,
	EVO_LEVEL,
	EVO_TRADE,
	EVO_TRADE_ITEM,
	EVO_ITEM,		// for dawn stone, add MON_MALE(0x0) or MON_FEMALE(0xFE) to .unknown in evo table entry
	EVO_LEVEL_ATK_GT_DEF,
	EVO_LEVEL_ATK_EQ_DEF,
	EVO_LEVEL_ATK_LT_DEF,
	EVO_LEVEL_SILCOON,
	EVO_LEVEL_CASCOON,
	EVO_LEVEL_NINJASK,
	EVO_LEVEL_SHEDINJA,
	EVO_BEAUTY,
	// new evolutions
	EVO_RAINY_FOGGY_OW,		// raining or foggy in overworld
	EVO_MOVE_TYPE,	// knows a move with a specific type (eg. sylveon: fairy type move). Param is the move type
	EVO_TYPE_IN_PARTY,	//specific type (unknown) in party after given level (param).
	EVO_MAP, 	// specific map evolution. bank in param, map in unknown
	EVO_MALE_LEVEL,		// above given level if male
	EVO_FEMALE_LEVEL,	// above given level if female
	EVO_LEVEL_NIGHT,	// above given level at night
	EVO_LEVEL_DAY,		// above given level during day
	EVO_HOLD_ITEM_NIGHT,	// level up holding item at night (eg. sneasel)
	EVO_HOLD_ITEM_DAY,	// level up while holding a specific item during the day (eg. happiny)
	EVO_MOVE,	// knows a given move
	EVO_OTHER_PARTY_MON,	//another poke in the party, arg is a specific species
	EVO_LEVEL_SPECIFIC_TIME_RANGE, // above given level with a range (unknown is [start][end]. eg lycanroc -> 1700-1800 hrs -> 0x1112)
	EVO_FLAG_SET, //If a certain flag is set. Can be used for touching the Mossy/Icy Rock for Leafeon/Glaceon evolutions
    EVO_NATURE_TOXTRICITY,
    EVO_NATURE_LOWKEY,
};

// #define SPECIES_EEVEE 0x85
// #define SPECIES_GLOOM 0x2C
#define SPECIES_APPLIN 0x46C
// #define SPECIES_SLOWPOKE 0x4F
#define SPECIES_SLOWPOKE_G 0x4BB
#define SPECIES_BASCULIN_BLUE 0x2E0
#define SPECIES_BASCULIN_RED 0x25B
#define SPECIES_ESPURR 0x311
#define SPECIES_BURMY 0x1D1
#define SPECIES_BURMY_TRASH 0x2C4 
#define SPECIES_BURMY_SANDY 0x2C3
#define ITEM_LINK_CABLE 87
#define ITEM_SUN_STONE 93
#define ITEM_MOON_STONE 94
#define ITEM_FIRE_STONE 95
#define ITEM_THUNDER_STONE 96
#define ITEM_WATER_STONE 97
#define ITEM_LEAF_STONE 98
#define ITEM_SHINY_STONE 99
#define ITEM_DUSK_STONE 100
#define ITEM_DAWN_STONE 101
#define ITEM_ICE_STONE 102
#define ITEM_UP_GRADE 218
#define MOVE_ANCIENTPOWER 0xF6
#define MOVE_DOUBLEHIT 0x1B6
// #define MOVE_MIMIC 0x66
// #define MOVE_STOMP 0x17
#define MOVE_DRAGONPULSE 0x172
// #define MOVE_TAUNT 0x10D
// #define MOVE_ROLLOUT 0xCD
#define MOVE_SHADOWPUNCH 0x145
#define MOVE_HYPERDRILL 0x300
#define MOVE_TWINBEAM 0x30F
#define ITEM_METAL_COAT 199
#define ITEM_PRISM_SCALE 176
#define ITEM_KINGS_ROCK 187
// #define SPECIES_SCYTHER 0x7B
// #define SPECIES_PICHU 0xAC
// #define SPECIES_KIRLIA 0x189
// #define SPECIES_SNORUNT 0x15A
// #define SPECIES_CLAMPERL 0x175
#define SPECIES_KUBFU 0x49F
#define SPECIES_CHARCADET 0x3AA
#define SPECIES_COSMOEM 0x3EF
#define SPECIES_PETILIL 0x259
// #define SPECIES_QUILAVA 0x9C
#define SPECIES_DARTRIX 0x3AC 
#define SPECIES_DEWOTT 0x22B
#define SPECIES_RUFFLET 0x2A8
#define SPECIES_BERGMITE 0x334
#define SPECIES_GOOMY 0x32C
// #define SPECIES_CUBONE 0x68
// #define SPECIES_EXEGGCUTE 0x66
#define SPECIES_MIME_JR 0x1EC
#define SPECIES_ROCKRUFF 0x3C1
#define SPECIES_LECHONK 0x34E
// LANGUAGE_SPANISH o 7 para usar los textos en español
// LANGUAGE_ENGLISH or 2 to use the english text
#define EV_IV_TEXT              LANGUAGE_ENGLISH

// Cambiar a TRUE si la potencia base de Poder Oculto es fija.
// Set TRUE if Hidden Power's Base Power is fixed
#define HIDDEN_POWER_STATIC     TRUE

// Cambie esto a la potencia base de Poder Oculto si está fijo en cualquier número que no sea 60.
// Change this to the base power of Hidden Power if it's fixed to any number other than 60.
#define HIDDEN_POWER_BASE_POWER 60

// TRUE = ACTIVADO,     FALSE = DESACTIVADO.    Activa o desactiva el salto de sprite
// TRUE = ON,           FALSE = OFF.            Activates or deactivates the sprite jump
#define SPRITE_JUMP             TRUE

// 1 = DE DERECHA A IZQUIERDA,  0 = EN EL CENTRO,       -1 = DE IZQUIERDA A DERECHA.
// 1 = FROM RIGHT TO LEFT,      0 = IN THE CENTER,      -1 = FROM LEFT TO RIGHT.
#define SPRITE_JUMP_DIRECTION   1

// 1 = A LA DERECHA,    0 = A LA IZQUIERDA.
// 1 = RIGHT,           0 = LEFT.
#define SPRITE_VIEW_DIRECTION   0


//coordenada x del sprite pokémon, se mide en tiles de 8 pixeles
//x coordinate of the pokémon sprite, measured in tiles of 8 pixels
#define PICMON_X    19

//coordenada y del sprite pokémon, se mide en tiles de 8 pixeles
//y coordinate of the pokémon sprite, measured in tiles of 8 pixels
#define PICMON_Y     5
#define FLAG_MINIMAL_GRINDING_MODE 0x1032
// ------------------------------------------------------------------ 
//                           FIRE RED/ROJOFUEGO     EMERALD/ESMERALDA
// FLAG_SYS_POKEMON_GET      0x828                  0x860
// FLAG_SYS_POKEDEX_GET      0x829                  0x861
//
// [ESP] ------------------------------------------------------------ 
// Cambiar FLAG_SYS_POKEMON_GET por la flag que quieras usar.
// ejemplo: FLAG_EV_IV  0x200
//
//  [ENG] ------------------------------------------------------------ 
// Change FLAG_SYS_POKEMON_GET to the flag you want to use.
// example: FLAG_EV_IV  0x200
#define FLAG_EV_IV FLAG_SYS_POKEMON_GET


static void Task_EvIvInit(u8);
static u8 EvIvLoadGfx(void);
static void EvIvVblankHandler(void);
static void UpdateCursorSpritePos(u16 spriteId, u8 stat, bool8 goingUp, bool8 resetY); 

static void Task_WaitForExit(u8);
static void ChangeSelectedStat(u8 stat, u8 ev, bool8 increase);
static void UpdateCurrentMon(void);
static void Task_EvIvReturn(u8);
static void BufferMonData(struct Pokemon * mon);
static s8 AdvanceMultiBattleMonIndex(s8 direction);
static void ShowSprite(struct Pokemon *mon);
static void MiniEvIvPrintText(struct Pokemon *mon, bool8 ev, u8 stat, u8 newValue, u8 stat2, bool8 fixev);

static void EvIvPrintText(struct Pokemon *mon);
static void ShowPokemonPic2(u16 species, u32 otId, u32 personality, u8 x, u8 y);
static void Task_ScriptShowMonPic(u8 taskId);
static void HidePokemonPic2(u8 taskId);

static void PrintStat(u8 nature, u8 stat);
static u8 GetDigitsDec(u32 num);
static u8 GetDigitsHex(u32 num);
static u8 GetColorByNature(u8 nature, u8 statIndex);
static u8 GetCurrentLevelCap(void); 
static void FixOddEVs(void);
#define EVS             0
#define IVS             1
#define ABILITY_EDIT    2
#define GENDER          3


const u32 gBgEvIvGfx[] = INCBIN_U32("graphics/bgEvIv.4bpp.lz");
const u32 gBgEvIvTilemap[] = INCBIN_U32("graphics/bgEvIv.bin.lz");
const u16 gBgEvIvPal[] = INCBIN_U16("graphics/bgEvIv.gbapal");

enum
{
    WIN_POKEMON_NAME,
    WIN_STATS,
    WIN_BOTTOM_BOX,
    WIN_HIDDEN_POWER,
    WIN_TYPE,
    WIN_TOP_BOX
};

extern const struct BaseStats *gBaseStatsPtr;

static const struct BgTemplate sBg_Templates[] = {
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 22,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 1,
    }, {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 19,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0,
    }
};

//window 0 = pokémon name
#define WINDOW0_WIDTH   29
#define WINDOW0_HEIGTH  2 //2

//window 1 = stats
#define WINDOW1_WIDTH   16
#define WINDOW1_HEIGTH  11
#define WINDOW1_BASEBLOCK  (WINDOW0_WIDTH * WINDOW0_HEIGTH)

//window 2 = text in the bottom bar / texto en la barra inferior
#define WINDOW2_WIDTH   29
#define WINDOW2_HEIGTH  5
#define WINDOW2_BASEBLOCK WINDOW1_WIDTH * WINDOW1_HEIGTH + WINDOW1_BASEBLOCK

//windows 3 = text hidden power
#define WINDOW3_WIDTH   11
#define WINDOW3_HEIGTH  4
#define WINDOW3_BASEBLOCK (WINDOW2_WIDTH * WINDOW2_HEIGTH) + WINDOW2_BASEBLOCK

//window 4 = type hidden power
#define WINDOW4_WIDTH   5
#define WINDOW4_HEIGTH  2
#define WINDOW4_BASEBLOCK (WINDOW3_WIDTH * WINDOW3_HEIGTH) + WINDOW3_BASEBLOCK

//window 5 = POKéMON EV-IV          +SEL. (A)(B)EXIT
#define WINDOW5_WIDTH   29
#define WINDOW5_HEIGTH  2
#define WINDOW5_BASEBLOCK (WINDOW4_WIDTH * WINDOW4_HEIGTH) + WINDOW4_BASEBLOCK

static const struct WindowTemplate sWindows_templates[] =
{
    [WIN_POKEMON_NAME] = 
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 2,
        .width = WINDOW0_WIDTH,
        .height = WINDOW0_HEIGTH,
        .paletteNum = 15,
        .baseBlock = 0x000
    },
    [WIN_STATS] = 
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 4,
        .width = WINDOW1_WIDTH,
        .height = WINDOW1_HEIGTH,
        .paletteNum = 15,
        .baseBlock = WINDOW1_BASEBLOCK
    },
    [WIN_BOTTOM_BOX] = 
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 15,
        .width = WINDOW2_WIDTH,
        .height = WINDOW2_HEIGTH,
        .paletteNum = 15,
        .baseBlock = WINDOW2_BASEBLOCK
    },
    [WIN_HIDDEN_POWER] = 
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 15,
        .width = WINDOW3_WIDTH,
        .height = WINDOW3_HEIGTH,
        .paletteNum = 15,
        .baseBlock = WINDOW3_BASEBLOCK
    },
    // [WIN_TYPE] = 
    // {
    //     .bg = 0,
    //     .tilemapLeft = 25,
    //     .tilemapTop = 15,
    //     .width = WINDOW4_WIDTH,
    //     .height = WINDOW4_HEIGTH,
    //     .paletteNum = 14,
    //     .baseBlock = WINDOW4_BASEBLOCK
    // },
    [WIN_TOP_BOX] = 
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 0,
        .width = WINDOW5_WIDTH,
        .height = WINDOW5_HEIGTH,
        .paletteNum = 15,
        .baseBlock = WINDOW5_BASEBLOCK
    },
    DUMMY_WIN_TEMPLATE
};


//                                     fondo                    fuente                  sombra
//                                     highlight                font                    shadow
static const u8 sBlackTextColor[3]  = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_DARK_GRAY,   TEXT_COLOR_LIGHT_GRAY};
static const u8 sBlueTextColor[3]   = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_BLUE,        TEXT_COLOR_LIGHT_GRAY};
static const u8 sRedTextColor[3]    = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_RED,         TEXT_COLOR_LIGHT_GRAY};
static const u8 sGrayTextColor[3]   = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_LIGHT_GRAY,  TEXT_COLOR_DARK_GRAY};
static const u8 sWhiteTextColor[3]  = {TEXT_COLOR_TRANSPARENT,  TEXT_COLOR_WHITE,       TEXT_COLOR_TRANSPARENT};

static const u8 *const sTextColorByNature[] = 
{
    [STAT_DECREASE + 1] = sBlueTextColor,
    [STAT_NEUTRAL + 1]  = sBlackTextColor,
    [STAT_INCREASE + 1] = sRedTextColor,
};

const u8 gText_eviv_Slash[] = _("/");
const u8 gText_CensorEgg[]  = _("{CLEAR_TO 12}?{CLEAR_TO 42}?{CLEAR_TO 66}?");

const u8 gText_BsEvIv[] = _("BS{CLEAR_TO 30}EV{CLEAR_TO 54}IV");

const u8 gText_eviv_Total[] = _("TOTAL:");
const u8 gText_Percent[] = _("% ");
const u8 gText_PokeSum_EvIv[] = _("EV-IV");
const u8 gText_eviv_Tittle[] = _("Pokémon EV-IV");

const u8 gText_eviv_Buttons[] = _("{DPAD_UPDOWN}SEL. {A_BUTTON}{B_BUTTON}EXIT");
const u8 gText_PokeSum_PageName_PokemonSkills[] = _("Pokémon Skills");
const u8 gText_PokeSum_Controls_Page[] = _("{DPAD_LEFTRIGHT}PAGE");
const u8 gText_PokeSum_Controls_Page_EvIv[] = _("{DPAD_LEFTRIGHT}PAGE {A_BUTTON}EV-IV");
const u8 gText_PokeSum_NoData[] = _("No data");
const u8 gText_Cancel2[] = _("CANCEL");
const u8 gText_eviv_Hp[]     = _("HP");
const u8 gText_eviv_Atk[]    = _("ATTACK");
const u8 gText_eviv_Def[]    = _("DEFENSE");
const u8 gText_eviv_SpAtk[]  = _("SP.ATK.");
const u8 gText_eviv_SpDef[]  = _("SP.DEF.");
const u8 gText_eviv_Speed[]  = _("SPEED");

const u8 gText_Your[]   = _("Your ");
const u8 gText_Is[]     = _(" is ");
const u8 gText_Happy[]  = _("Happiness: ");
const u8 gText_Lv[]    = _(" Lv");
const u8 gText_HiddenPower[] = _("Hidden power");
const u8 gText_Power[]  = _("Power: ");

const u8 gText_Steps_to_hatching[]  = _("Steps to\nhatch: ");


const u8 gText_EvolvesLevel[] = _("Evolves at Lv ");
const u8 gText_DependingOnTimeOfDay[] = _(" depending on time of day.");

const u8 gText_EvosFriendship[] = _("Evolves with high happiness.");
const u8 gText_EvolvesFriendshipDay[] = _("Evolves with high happiness at daytime.");
const u8 gText_EvolvesFriendshipNight[] = _("Evolves with high happiness at night.");
const u8 gText_inRain[] = _(" in Rain.");
const u8 gText_NoEvolution[] = _("No evolution.");

const u8 gText_EvolvesVariousWays[]  = _("Evolves in various ways.");
const u8 gText_EvoItem[]  = _("Evolves by using a ");
const u8 gText_EvoApplin[]  = _("Evolves by Leaf Stone, Sun Stone, or at Lv30.");
const u8 gText_EvoItemLinkCable[]  = _("Link Cable");

const u8 gText_EvoItemSunStone[] = _("Sun Stone");
const u8 gText_EvoItemMoonStone[] = _("Moon Stone");
const u8 gText_EvoItemFireStone[] = _("Fire Stone");
const u8 gText_EvoItemThunderStone[] = _("Thunder Stone");
const u8 gText_EvoItemWaterStone[] = _("Water Stone");

const u8 gText_EvoItemLeafStone[] = _("Leaf Stone");

const u8 gText_EvoItemShinyStone[] = _("Shiny Stone");

const u8 gText_EvoItemDuskStone[] = _("Dusk Stone");

const u8 gText_EvoItemDawnStone[] = _("Dawn Stone");
const u8 gText_EvoItemKingsRock[] = _("King's Rock");
const u8 gText_EvoItemMetalCoat[] = _("Metal Coat");
const u8 gText_EvoIfFemale[] = _(" if female");
const u8 gText_EvoIfMale[] = _(" if male");
const u8 gText_EvoGenderDepend[] = _(" depending on gender");
const u8 gText_EvoItemIceStone[] = _("Ice Stone");

const u8 gText_EvoItemUpgrade[] = _("Upgrade");
const u8 gText_EvoItemPrismScale[] = _("Prism Scale");

const u8 gText_EvoMethodIf[] = _(" if ");

const u8 gText_EvoMethodDark[] = _("Dark ");

const u8 gText_EvoTypeInParty[] = _(" type in party");

const u8 gText_EvoWhenKnowing[] = _("Evolves when knowing ");

const u8 gText_EvoAncientPower[] = _("Ancient Power");
const u8 gText_EvoDoubleHit[] = _("Double Hit");
const u8 gText_EvoMimic[] = _("Mimic");
const u8 gText_EvoStomp[] = _("Stomp");
const u8 gText_EvoDragonPulse[] = _("Dragon Pulse");
const u8 gText_EvoTaunt[] = _("Taunt");
const u8 gText_EvoRollout[] = _("Rollout");
const u8 gText_EvoShadowPunch[] = _("Shadow Punch");
const u8 gText_EvoHyperDrill [] = _("Hyper Drill");
const u8 gText_EvoTwinBeam[] = _("Twin Beam");
const u8 gText_EvoComma[] = _(", ");


const u8 gText_EvoOr[] = _(" or ");
const u8 gText_EvoSlowpokeStuff[] = _("37 or King's Rock.");
const u8 gText_EvoSnoruntStuff[] = _("42 or Dawn Stone if Female.");
const u8 gText_EvoKirliaStuff[] = _("30 or Dawn Stone if Male.");
const u8 gText_PikachuStuff[] = _("Thunder or Shiny Stone.");
const u8 gText_EvoAvaluggStuff[] = _("36 or Ice Stone.");
const u8 gText_EvoGoomyStuff[] = _("40 or Metal Coat.");

const u8 gText_EvoPoliStuff[] = _("Water Stone or King's Rock.");
const u8 gText_EvoIfDay[] = _(" if daytime");
const u8 gText_EvoIfNight[] = _(" if nighttime");

const u8 gText_EvoMimeJrStuff[] = _(" or Ice Stone.");
const u8 gText_Tu[] = _("");
const u8 gText_Period[]   = _(".");

u8 LevelCap_Badges2[18] = {
	15, //Before Brock
	22, //before Mt Moon Archer
	27, //Before Misty
	34, //Before Surge
	44, //Before Erika
	47, //Before Celadon Giovanni
	56, //Before Archer/Ariana Tag battle
	57, //Before Giovanni Saffron
	59, //Before Sabrina 
	68, //Before Koga
	73, //Before May
	76, //Before Blaine
	79, //Before Archer/Ariana 
	80, //Before Giovanni Final
	81, //Before Claire
	82, //Before Brendan
	85, //Before E4
	100}; //added here

u8 LevelCap_BadgesHardcoreMode2[18] = {
	16, //Before Brock
	23, //before Mt Moon Archer
	28, //Before Misty
	36, //Before Surge
	44, //Before Erika
	47, //Before Celadon Giovanni
	56, //Before Archer/Ariana Tag battle
	57, //Before Giovanni Saffron
	59, //Before Sabrina 
	68, //Before Koga
	73, //Before May
	76, //Before Blaine
	79, //Before Archer/Ariana 
	80, //Before Giovanni Final
	81, //Before Claire
	82, //Before Brendan
	85, //Before E4
	100}; //added here

static u8 GetBadgeCount(void) //added this here
{
	u8 badgeCount = 0;

	if (FlagGet(FLAG_SYS_GAME_CLEAR)) //0x82C
		return 17;
	if (FlagGet(FLAG_BRENDAN_FINAL))
		++badgeCount;
	if (FlagGet(FLAG_BADGE08_GET))
		++badgeCount;
	if (FlagGet(FLAG_GIOVANNI_BOSS))
	    ++badgeCount;
	if (FlagGet(FLAG_ARCHER_ARIANA_B2B))
		++badgeCount;
	if (FlagGet(FLAG_BADGE07_GET))
		++badgeCount;
	if (FlagGet(FLAG_MAY_BOSS))
		++badgeCount;
	if (FlagGet(FLAG_BADGE06_GET))
		++badgeCount;
	if (FlagGet(FLAG_BADGE05_GET))
		++badgeCount;
	if (FlagGet(FLAG_HIDE_SILPH_ROCKETS))
		++badgeCount;
	if (FlagGet(FLAG_ARCHER_ARIANA_TAG))
		++badgeCount;
	if (FlagGet(FLAG_HIDE_HIDEOUT_GIOVANNI))
		++badgeCount; 
	if (FlagGet(FLAG_BADGE04_GET))
		++badgeCount;
	if (FlagGet(FLAG_BADGE03_GET))
		++badgeCount;
	if (FlagGet(FLAG_BADGE02_GET))
		++badgeCount;
	if (FlagGet(FLAG_ARCHER_MT_MOON))
		++badgeCount; 
	if (FlagGet(FLAG_BADGE01_GET))
		++badgeCount;

	return badgeCount;
}

static u32 MathMin(u32 num1, u32 num2)
{
	if (num1 < num2)
		return num1;

	return num2;
}

static u8 GetCurrentLevelCap(void) 
{
	u8 badgeCount = GetBadgeCount(); //added
	u8 lvlCap = LevelCap_Badges2[badgeCount]; //added
	if (FlagGet(FLAG_HARDCORE_MODE)){
		lvlCap = LevelCap_BadgesHardcoreMode2[badgeCount];
	}
	else if (FlagGet(FLAG_EASY_MODE)) {
		lvlCap = lvlCap + 3;
		lvlCap = MathMin(100, lvlCap);
	}
	return lvlCap;
}

#define SELECTION_CURSOR_TAG 0x200

static void SpriteCB_SandboxCursor(struct Sprite* sprite);
static const struct OamData sCursorOam =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 0, //Above other sprites
};
static const union AnimCmd sAnimCmdHandCursor[] =
{
	ANIMCMD_FRAME(0, 30),
	ANIMCMD_FRAME(16, 30),
	ANIMCMD_JUMP(0)
};

static const union AnimCmd sAnimCmdHandCursorPointed[] =
{
	ANIMCMD_FRAME(16, 30),
	ANIMCMD_END
};

static const union AnimCmd *const sAnimCmdTable_HandCursor[] =
{
	sAnimCmdHandCursor,
	sAnimCmdHandCursorPointed,
};
static const struct SpriteTemplate sGUICursorTemplate =
{
	.tileTag = SELECTION_CURSOR_TAG,
	.paletteTag = SELECTION_CURSOR_TAG,
	.oam = &sCursorOam,
	.anims = sAnimCmdTable_HandCursor,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCB_SandboxCursor,
};

static const struct SpriteSheet sCursorSpriteSheet = {(void*) 0x83D2BEC, (32 * 32 * 4) / 2, SELECTION_CURSOR_TAG};
static const struct SpritePalette sCursorSpritePalette = {(void*) 0x83CE7F0, SELECTION_CURSOR_TAG};

static void SpriteCB_SandboxCursor(struct Sprite* sprite)
{

}


static void CreateSandboxCursor(void)
{
	LoadSpriteSheet(&sCursorSpriteSheet);
	LoadSpritePalette(&sCursorSpritePalette);
    u8 x = 0;
    u8 y = 0;
    switch(gSelectedColumn)
    {
        case EVS:
            x = 112;
            y = 28 + (14 * gSelectedStat); 
            break;
        case IVS:
            x = 138;
            y = 28 + (14 * gSelectedStat); 
            break;
        case ABILITY_EDIT:
            x = 180;
            y = 121;
            break;
        case GENDER:
            x = 164;
            y = 24;
            break;
    }
	gCursorSpriteId = CreateSprite(&sGUICursorTemplate, x, y, 1);
}

static void EvIvBgInit(void)
{
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetTasks();
    ScanlineEffect_Stop();
}

static void CB2_EvIv(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

/**
 * 2022-03-03
 * ACIMUT:
 * Callback summary screen
*/

#define FREE_AND_SET_NULL_IF_SET(ptr) \
{                                     \
    if (ptr != NULL)                  \
    {                                 \
        free(ptr);                    \
        (ptr) = NULL;                 \
    }                                 \
}

void CB2_ShowEvIv_SummaryScreen(void)
{
    if (!gPaletteFade.active)
#ifdef FIRERED
        Show_EvIv(sMonSummaryScreen->monList.mons,
                sLastViewedMonIndex,
                sMonSummaryScreen->lastIndex,
                sMonSummaryScreen->savedCallback,
                sMonSummaryScreen->isBoxMon,
                TRUE);
#else//EMERALD
        Show_EvIv(sMonSummaryScreen->monList.mons,
                sMonSummaryScreen->curMonIndex,//gLastViewedMonIndex,
                sMonSummaryScreen->maxMonIndex,
                sMonSummaryScreen->callback,
                sMonSummaryScreen->isBoxMon,
                TRUE);
#endif
}

/**
 * 2022-03-03
 * ACIMUT:
 * Esta función configura el ev-iv summary screen por 
 * defecto, muestra nuestro equipo pokémon.
*/

void CB2_ShowEvIv_PlayerParty(void)
{
    u8 lastIdx;
    if (gPlayerPartyCount == 0)
        lastIdx = 0;
    else
        lastIdx = gPlayerPartyCount - 1;
#ifdef FIRERED
    Show_EvIv(gPlayerParty, 0, lastIdx, CB2_ReturnToFieldFromDiploma, FALSE, FALSE);
#else//EMERALD
    Show_EvIv(gPlayerParty, 0, lastIdx, CB2_ReturnToFieldFadeFromBlack, FALSE, FALSE);
#endif
}

void Show_EvIv(struct Pokemon * party, u8 cursorPos, u8 lastIdx, MainCallback savedCallback, bool8 isboxMon, bool8 return_summary_screen)
{
    gEvIv = AllocZeroed(sizeof(*gEvIv));

    if (gEvIv == NULL)
    {
        SetMainCallback2(savedCallback);
        return;
    }
    
    gEvIv->state = 0;
    gEvIv->gfxStep = 0;
    gEvIv->callbackStep = 0;
    
    gEvIv->monList.mons = party;
    gEvIv->cursorPos = cursorPos;
    gEvIv->lastIdx = lastIdx;
    gEvIv->savedCallback = savedCallback;
    gEvIv->isBoxMon = isboxMon;
    gEvIv->return_summary_screen = return_summary_screen;

    BufferMonData(&gEvIv->currentMon);

    EvIvBgInit();
    CreateTask(Task_EvIvInit, 80);
    SetMainCallback2(CB2_EvIv);
}

static void VCBC_EvIvOam(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Task_EvIvInit(u8 taskId)
{
    switch (gEvIv->callbackStep)
    {
    case 0:
        SetVBlankCallback(NULL);
        break;
    case 1:
        EvIvVblankHandler();
        break;
    case 2:
        if (!EvIvLoadGfx())
        {
            return;
        }
        break;
    case 3:
        CopyToBgTilemapBuffer(1, gBgEvIvTilemap, 0, 0);
        break;
    case 4:
        // FillWindowPixelBuffer(WIN_TOP_BOX, 0);
        // AddTextPrinterParameterized3(WIN_TOP_BOX, 2, 0x10, 2, sWhiteTextColor, 0, gText_eviv_Tittle);
        // AddTextPrinterParameterized3(WIN_TOP_BOX, 0, 0x98, 1, sWhiteTextColor, 0, gText_eviv_Buttons);
        break;
    case 5:
        PutWindowTilemap(WIN_TOP_BOX);
        ShowSprite(&gEvIv->currentMon);
        EvIvPrintText(&gEvIv->currentMon);
        break;
    case 6:
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        break;
    case 7:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        break;
    case 8:
        SetVBlankCallback(VCBC_EvIvOam);
        break;
    default:
        if (gPaletteFade.active)
        {
            break;
        }
        gTasks[taskId].func = Task_WaitForExit;
    }
    gEvIv->callbackStep++;
}

static void Task_WaitForExit(u8 taskId)
{
    s8 monId = -1;
    bool8 update_mon = TRUE;

    switch (gEvIv->state)
    {
    case 0:
        gEvIv->state++;
        break;
    case 1:
        if (gEvIv->lastIdx)
        {
            if (JOY_REPT(DPAD_DOWN))
            {
                if (gEvIv->isBoxMon)
                {
                    monId =  SeekToNextMonInBox(gEvIv->monList.boxMons, gEvIv->cursorPos, gEvIv->lastIdx, DIR_DOWN_2);
                    if (monId == -1)//si llega al final, revise el primer elemento.
                        monId = SeekToNextMonInBox(gEvIv->monList.boxMons, 1, gEvIv->lastIdx, DIR_UP_2);
                    if (monId == -1)//si el primer elemento no tiene boxmon, revise desde el segundo en adelante.
                        monId = SeekToNextMonInBox(gEvIv->monList.boxMons, 0, gEvIv->lastIdx, DIR_DOWN_2);
                    if (gEvIv->cursorPos == monId)
                        update_mon = FALSE;
                    else
                        gEvIv->cursorPos = monId;
                }
            #ifdef FIRERED
                else if (IsUpdateLinkStateCBActive() == FALSE
                    && gReceivedRemoteLinkPlayers == 1
                    && IsMultiBattle() == TRUE)
            #else //EMERALD
                else if (IsMultiBattle() == TRUE)
            #endif
                {
                    gEvIv->cursorPos = AdvanceMultiBattleMonIndex(+1);
                }
                else
                {
                    if (gEvIv->cursorPos == gEvIv->lastIdx)
                        gEvIv->cursorPos = 0;
                    else
                        gEvIv->cursorPos++;
                }

                if (update_mon)
                    UpdateCurrentMon();
            }
            else if (JOY_REPT(DPAD_UP))
            {
                if (gEvIv->isBoxMon)
                {
                    monId =  SeekToNextMonInBox(gEvIv->monList.boxMons, gEvIv->cursorPos, gEvIv->lastIdx, DIR_UP_2);
                    if (monId == -1)//si llega al inicio, revise el último elemento.
                        monId = SeekToNextMonInBox(gEvIv->monList.boxMons, gEvIv->lastIdx -1, gEvIv->lastIdx, DIR_DOWN_2);
                    if (monId == -1)//si el último elemento no tiene boxMon, revise desde el penúltimo hacia atrás
                        monId = SeekToNextMonInBox(gEvIv->monList.boxMons, gEvIv->lastIdx, gEvIv->lastIdx, DIR_UP_2);
                    if (gEvIv->cursorPos == monId)
                        update_mon = FALSE;
                    else
                        gEvIv->cursorPos = monId;
                }
            #ifdef FIRERED
                else if (IsUpdateLinkStateCBActive() == FALSE
                    && gReceivedRemoteLinkPlayers == 1
                    && IsMultiBattle() == TRUE)
            #else //EMERALD
                else if (IsMultiBattle() == TRUE)
            #endif
                {
                    gEvIv->cursorPos = AdvanceMultiBattleMonIndex(-1);
                }
                else
                {
                    if (gEvIv->cursorPos == 0)
                        gEvIv->cursorPos = gEvIv->lastIdx;
                    else
                        gEvIv->cursorPos--;
                }

                if (update_mon)
                    UpdateCurrentMon();
            }
        }

        if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
#ifdef FIRERED
            PlaySE(SE_CARD_FLIP);
#else//EMERALD
            PlaySE(SE_RG_CARD_FLIP);
#endif
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gEvIv->state++;
        }
        break;
    case 2:
        if (!IsCryPlaying())
            Task_EvIvReturn(taskId);
        break;
    }
}


static void ChangeSelectedStat(u8 stat, u8 ev, bool8 increase)
{
    u8 newValue;
    u8 statToEdit;
    u8 increaseBy = (ev) ? 2 : 1;
    u16 total = 0;
    u16 newTotal = 0;
    switch(stat)
    {
        case EDITOR_STAT_SPATK:
            statToEdit = (ev) ? MON_DATA_SPATK_EV : MON_DATA_SPATK_IV;
            break;
        case EDITOR_STAT_SPDEF:
            statToEdit = (ev) ? MON_DATA_SPDEF_EV : MON_DATA_SPDEF_IV;
            break;
        case EDITOR_STAT_SPD:
            statToEdit = (ev) ? MON_DATA_SPEED_EV : MON_DATA_SPEED_IV;
            break;
        default:
            statToEdit = (ev) ? MON_DATA_HP_EV + stat : MON_DATA_HP_IV + stat;
            break;
    }

    struct Pokemon * mon = &gPlayerParty[gEvIv->cursorPos];
    u8 currValue = GetMonData(mon, statToEdit);
    // u8 maxValue = (ev) ? 252 : 31;
    if(!ev)
    {
        if (currValue > 30 && increase)
            newValue = 0;
        else if (currValue == 0 && !increase)
            newValue = 31;
        else if (increase)
            newValue = currValue + increaseBy;
        else
            newValue = currValue - increaseBy;
    }
    if(ev)
    {
        if(currValue > 250 && increase)
            newValue = 0;
        else if (currValue < 2 && !increase)
            newValue = 252;
        else if (increase)
            newValue = currValue + increaseBy;
        else if (!increase)
            newValue = currValue - increaseBy;
        u16 cap = 510;
        total += GetMonData(mon, MON_DATA_HP_EV);
        total += GetMonData(mon, MON_DATA_ATK_EV);
        total += GetMonData(mon, MON_DATA_DEF_EV);
        total += GetMonData(mon, MON_DATA_SPATK_EV);
        total += GetMonData(mon, MON_DATA_SPDEF_EV);
        total += GetMonData(mon, MON_DATA_SPEED_EV);
        newTotal += (statToEdit == MON_DATA_HP_EV) ? newValue : GetMonData(mon, MON_DATA_HP_EV);
        newTotal += (statToEdit == MON_DATA_ATK_EV) ? newValue : GetMonData(mon, MON_DATA_ATK_EV);
        newTotal += (statToEdit == MON_DATA_DEF_EV) ? newValue : GetMonData(mon, MON_DATA_DEF_EV);
        newTotal += (statToEdit == MON_DATA_SPATK_EV) ? newValue : GetMonData(mon, MON_DATA_SPATK_EV);
        newTotal += (statToEdit == MON_DATA_SPDEF_EV) ? newValue : GetMonData(mon, MON_DATA_SPDEF_EV);
        newTotal += (statToEdit == MON_DATA_SPEED_EV) ? newValue : GetMonData(mon, MON_DATA_SPEED_EV);
        if(newTotal > cap)
            newValue = cap - total;
    }
    /*u8 maxValue = (ev) ? 252 : 31;

    increaseBy = (ev) ? 2 : 1;
    if(increase && currValue != 251 && currValue != 252)
         newValue = currValue + increaseBy;
    else if (currValue == 252)
    {
        newValue = 0;
    }
    else if(!increase && currValue != 0 && currValue != 1)
        newValue = currValue - increaseBy;
    else if(currValue == 0)
    {
        newValue = GetCurrentEVCap();
    }
    if(ev)
    {
        u16 cap = GetCurrentEVCap();
        total += mon->hpEv;
        total += mon->atkEv;
        total += mon->defEv;
        total += mon->spAtkEv;
        total += mon->spDefEv;
        total += mon->spdEv;
        newTotal += (statToEdit == MON_DATA_HP_EV) ? newValue : mon->hpEv;
        newTotal += (statToEdit == MON_DATA_ATK_EV) ? newValue : mon->atkEv;
        newTotal += (statToEdit == MON_DATA_DEF_EV) ? newValue : mon->defEv;
        newTotal += (statToEdit == MON_DATA_SPATK_EV) ? newValue : mon->spAtkEv;
        newTotal += (statToEdit == MON_DATA_SPDEF_EV) ? newValue : mon->spDefEv;
        newTotal += (statToEdit == MON_DATA_SPEED_EV) ? newValue : mon->spdEv;
        if(newTotal >= cap)
            newValue = cap - total;
        if(!(newValue % 2 == 0))
            newValue -= 1;
    }*/
    SetMonData(mon, statToEdit, &newValue);
    MiniEvIvPrintText(mon, ev, statToEdit, newValue, stat, FALSE);
}

static void UpdateCurrentMon(void)
{
    BufferMonData(&gEvIv->currentMon);
    HidePokemonPic2(gEvIv->spriteTaskId);
    EvIvPrintText(&gEvIv->currentMon);
    ShowSprite(&gEvIv->currentMon);
}

//08135c34 l 000002ec CB2_SetUpPSS
extern void CB2_SetUpPSS(void);
//08138b8c l 00000060 BufferSelectedMonData
extern void BufferSelectedMonData(struct Pokemon * mon);


extern void CB2_InitSummaryScreen(void);

static void Task_EvIvReturn(u8 taskId)
{
    bool8 return_summary_screen = gEvIv->return_summary_screen;

    if (gPaletteFade.active)
        return;

    HidePokemonPic2(gEvIv->spriteTaskId);
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    
#ifdef FIRERED
    if (return_summary_screen)
    {
        sLastViewedMonIndex = gEvIv->cursorPos;

        sMonSummaryScreen->curPageIndex = PSS_PAGE_INFO;

        sMonSummaryScreen->state3270 = PSS_STATE3270_FADEIN;
        sMonSummaryScreen->summarySetupStep = 0;
        sMonSummaryScreen->loadBgGfxStep = 0;
        sMonSummaryScreen->spriteCreationStep = 0;
        
        sMonSummaryScreen->whichBgLayerToTranslate = 0;
        sMonSummaryScreen->skillsPageBgNum = 2;
        sMonSummaryScreen->infoAndMovesPageBgNum = 1;
        sMonSummaryScreen->flippingPages = FALSE;

        BufferSelectedMonData(&sMonSummaryScreen->currentMon);

        sMonSummaryScreen->isEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_IS_EGG);
        sMonSummaryScreen->isBadEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SANITY_IS_BAD_EGG);

        if (sMonSummaryScreen->isBadEgg == TRUE)
            sMonSummaryScreen->isEgg = TRUE;

        sMonSummaryScreen->lastPageFlipDirection = 0xff;

        SetMainCallback2(CB2_SetUpPSS);
    }
    else
    {
        SetMainCallback2(CB2_ReturnToFieldFromDiploma);
    }
#else//EMERALD
    if (return_summary_screen)
    {
        sMonSummaryScreen->curMonIndex = gEvIv->cursorPos;
        sMonSummaryScreen->minPageIndex = 0;
        sMonSummaryScreen->maxPageIndex = PSS_PAGE_COUNT - 1;
        sMonSummaryScreen->currPageIndex = sMonSummaryScreen->minPageIndex;

        SetMainCallback2(CB2_InitSummaryScreen);
    }
    else
    {
        SetMainCallback2(CB2_ReturnToFieldFadeFromBlack);
    }
#endif
    FREE_AND_SET_NULL_IF_SET(gEvIv);
}

static void BufferMonData(struct Pokemon * mon)
{
    if (!gEvIv->isBoxMon)
    {
        struct Pokemon * partyMons = gEvIv->monList.mons;
        *mon = partyMons[gEvIv->cursorPos];
    }
    else
    {
#if CFRU == FALSE
        struct BoxPokemon * boxMons = gEvIv->monList.boxMons;
        BoxMonToMon(&boxMons[gEvIv->cursorPos], mon);
#else //CFRU
        struct CompressedPokemon * compMon = gEvIv->monList.compMon;
        CompressedMonToMon(&compMon[gEvIv->cursorPos], mon);
#endif
    }
}

//EMERALD: sMultiBattleOrder
static const u8 sMultiBattlePartyOrder[]    = {0, 2, 3, 1, 4, 5};

static bool8 IsValidToViewInMulti(struct Pokemon *mon)
{
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    //if (gEvIv->cursorPos != 0 || !GetMonData(mon, MON_DATA_IS_EGG))
        return TRUE;
    //return FALSE;
}

static s8 AdvanceMultiBattleMonIndex(s8 direction)
{
    struct Pokemon *mons = gEvIv->monList.mons;
    s8 index, foundPartyIdx = 0;
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sMultiBattlePartyOrder[i] == gEvIv->cursorPos)
        {
            foundPartyIdx = i;
            break;
        }
    }

    while (TRUE)
    {
        foundPartyIdx += direction;
        if (foundPartyIdx < 0)
            foundPartyIdx = (PARTY_SIZE - 1);
        if (foundPartyIdx >= PARTY_SIZE)
            foundPartyIdx = 0;

        index = sMultiBattlePartyOrder[foundPartyIdx];
        if (IsValidToViewInMulti(&mons[index]) == TRUE)
            return index;
    }
}

static void ResetBGPos(void)
{
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    ChangeBgX(2, 0, 0);
    ChangeBgY(2, 0, 0);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);
}

static void ResetGpu(void)
{
    void *vram = (void *)VRAM;
    DmaClearLarge16(3, vram, VRAM_SIZE, 0x1000);
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
}

static void EvIvVblankHandler(void)
{
    ResetGpu();
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBg_Templates, 2);
    ResetBGPos();
    InitWindows(sWindows_templates);
    DeactivateAllTextPrinters();
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
    SetBgTilemapBuffer(1, gEvIv->tilemapBuffer);
    ShowBg(0);
    ShowBg(1);
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);
    FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
}

static u8 EvIvLoadGfx(void)
{
    switch (gEvIv->gfxStep)
    {
    case 0:
        ResetTempTileDataBuffers();
        break;
    case 1:
        DecompressAndCopyTileDataToVram(1, gBgEvIvGfx, 0, 0, 0);
        break;
    case 2:
        if (!(FreeTempTileDataBuffersIfPossible() == 1))
        {
            break;
        }
        return 0;
    case 3:
        LoadPalette(gBgEvIvPal, 0, 0x20);
        LoadPalette(GetTextWindowPalette(0), 0xf0, 0x20);
        ListMenuLoadStdPalAt(0xE0, 1);
        break;
    default:
        return 1;
    }
    gEvIv->gfxStep++;
    return 0;
}

static void ShowSprite(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 isEgg    = GetMonData(mon, MON_DATA_IS_EGG);
    u32 otId = GetMonData(mon, MON_DATA_OT_ID);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);

    //imprime el sprite del pokémon, si es un huevo no suena grito.
    //Print the sprite of the pokémon, if it is an egg it does not sound a scream.
    if (!isEgg)
    {
        ShowPokemonPic2(species, otId, personality, PICMON_X, PICMON_Y);
        PlayCry_Normal(species, 0);
    }else
        ShowPokemonPic2(SPECIES_EGG, 0, 0x8000, PICMON_X, PICMON_Y);
}

#define tState               data[0]
#define tSpecie              data[1]
#define tSpriteId            data[2]
#define tTimer               data[3]

void HidePokemonPic2(u8 taskId)
{
    struct Task * task = &gTasks[taskId];
    task->tState = 2;
}

static u8 CreateMonSprite_Field(u16 species, u32 otId, u32 personality, s16 x, s16 y, u8 subpriority)
{
    const struct CompressedSpritePalette * spritePalette = GetMonSpritePalStructFromOtIdPersonality(species, otId, personality);
    u16 spriteId = CreateMonPicSprite_HandleDeoxys(species, otId, personality, 1, x, y, 0, spritePalette->tag);
    PreservePaletteInWeather(IndexOfSpritePaletteTag(spritePalette->tag) + 0x10);
    if (spriteId == 0xFFFF)
        return MAX_SPRITES;
    else
        return spriteId;
}

static void ShowPokemonPic2(u16 species, u32 otId, u32 personality, u8 x, u8 y)
{
    u8 spriteId;

    spriteId = CreateMonSprite_Field(species, otId, personality, 8 * x + 40, 8 * y + 40, FALSE);
    gEvIv->spriteTaskId = CreateTask(Task_ScriptShowMonPic, 80);

    gSprites[spriteId].hFlip = SPRITE_VIEW_DIRECTION;

    //Ajusta el sprite del pokémon 2 píxeles a la izquierda
    //Adjust the pokémon sprite 2 pixels to the left
    gSprites[spriteId].x -= 2;

#if SPRITE_JUMP
    gSprites[spriteId].y -= 32;
    gSprites[spriteId].x += 48 * SPRITE_JUMP_DIRECTION;
#endif

    gTasks[gEvIv->spriteTaskId].tState = 0;
    gTasks[gEvIv->spriteTaskId].tSpecie = species;
    gTasks[gEvIv->spriteTaskId].tSpriteId = spriteId;
    gTasks[gEvIv->spriteTaskId].tTimer = 0;
    gSprites[spriteId].callback = SpriteCallbackDummy;
    gSprites[spriteId].oam.priority = 0;
}

static const s8 delta_jump[] =
{
    [ 0 ...  9] =  0, // < 10
    [10 ... 17] =  1, // < 18
    [18 ... 23] =  2, // < 24
    [24 ... 27] =  3, // < 28
    [28 ... 29] = -3, // < 30
    [30 ... 31] = -2, // < 32
    [32 ... 35] = -1, // < 36
    [36 ... 39] =  0, // < 40
    [40 ... 43] =  1, // < 44
    [44 ... 45] =  2, // < 46
    [46 ... 47] =  3, // < 48

};

static void Task_ScriptShowMonPic(u8 taskId)
{
    struct Task * task = &gTasks[taskId];

    switch (task->tState)
    {
    case 0:
        task->tTimer = 0;
        task->tState++;
        break;
    case 1:

#if SPRITE_JUMP
        if (task->tTimer < 48)
        {
            gSprites[task->tSpriteId].y += delta_jump[task->tTimer];
            gSprites[task->tSpriteId].x -= SPRITE_JUMP_DIRECTION;
        }
        else
            task->tState = 4;

        task->tTimer++;
#endif

        break;
    case 2:
        FreeResourcesAndDestroySprite(&gSprites[task->tSpriteId], task->tSpriteId);
        task->tState++;
        break;
    case 3:
        DestroyTask(taskId);
        break;
    }
}

#undef tState
#undef tSpecie
#undef tSpriteId
#undef tTimer

/*

+- - - - - - - - - - - +
|HP       160  255  16 |  HP_Y
|ATTACK   110   10  30 |  ATK_Y
|DEFENSE   65   20  31 |  DEF_Y
|SP.ATK.   65    1  20 |  SPATK_Y
|SP.DEF.  110   80  31 |  SPDEF_Y
|SPEED     30   11  13 |  SPEED_Y
+- - - - - - - - - - - +
|        |    |    |
 HP_X     BS_X EV_X IV_X

*/

#define HP_X        4
#define BS_X        60
#define EV_X        BS_X + 26
#define IV_X        EV_X + 30

#define HP_Y        3
#define ATK_Y       HP_Y + 14
#define DEF_Y       ATK_Y + 14
#define SPATK_Y     DEF_Y + 14
#define SPDEF_Y     SPATK_Y + 14
#define SPEED_Y     SPDEF_Y + 14


static void PrintWindow0(struct Pokemon *mon);
static void PrintWindow1(u8 nature, u8 isEgg);
static void PrintWindow2(u16 species, u8 isEgg, u8 friendship, struct Pokemon *mon);
static void PrintWindow_HiddenPower(u8 isEgg, u8 friendship);

static void MiniEvIvPrintText(struct Pokemon *mon, bool8 ev, u8 stat, u8 newValue, u8 stat2, bool8 fixev)
{
    u8 nature   = GetNature(mon);
    u8 arrStat;
    if(!fixev)
    {
        switch(stat)
        {
            case MON_DATA_SPATK_EV:
            case MON_DATA_SPATK_IV:
                arrStat = STAT_SPATK;
                break;
            case MON_DATA_SPDEF_EV:
            case MON_DATA_SPDEF_IV:
                arrStat = STAT_SPDEF;
                break;
            case MON_DATA_SPEED_EV:
            case MON_DATA_SPEED_IV:
                arrStat = STAT_SPEED;
                break;
            default:
                arrStat = STAT_HP + stat2;
                break;
        }
    }
    else
    {

    }
    if(ev)
    {
        gTotalStatsEV = 0;
        gStats_ev[arrStat] = newValue;
        for (int i = 0; i < NUM_STATS; i++)
            gTotalStatsEV += gStats_ev[i];
    }
    else
    {
        gTotalStatsIV = 0;
        gStats_iv[arrStat] = newValue;
        for (int i = 0; i < NUM_STATS; i++)
            gTotalStatsIV += gStats_iv[i];
    }
    //FillWindowPixelBuffer(0, 0);
    /*FillWindowPixelBuffer(1, 0);
    FillWindowPixelBuffer(2, 0);

    //PrintWindow0(mon);
    PrintWindow1(nature, isEgg);
    PrintWindow2(species, isEgg, friendship, ability);*/
    ConvertIntToDecimalStringN(gStringVar2, gStats_ev[arrStat], STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, gStats_iv[arrStat], STR_CONV_MODE_RIGHT_ALIGN, 2);
    u8 color = GetColorByNature(nature, arrStat);
    switch(arrStat)
    {
        case STAT_HP:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, HP_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, HP_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
        case STAT_ATK:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, ATK_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, ATK_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
        case STAT_DEF:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, DEF_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, DEF_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
        case STAT_SPATK:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, SPATK_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, SPATK_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
        case STAT_SPDEF:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, SPDEF_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, SPDEF_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
        case STAT_SPEED:
            FillWindowPixelRect(WIN_STATS, PIXEL_FILL(0), (ev) ? EV_X : IV_X, SPEED_Y, 22, 12);
            AddTextPrinterParameterized3(WIN_STATS, 2, (ev) ? EV_X : IV_X, SPEED_Y, sTextColorByNature[color], 0, (ev) ? gStringVar2 : gStringVar3);
            break;
    }

    ConvertIntToDecimalStringN(gStringVar2, gTotalStatsEV, STR_CONV_MODE_RIGHT_ALIGN, 3);
    
    // ConvertIntToDecimalStringN(gStringVar3, gTotalStatsIV, STR_CONV_MODE_RIGHT_ALIGN, 3);
    // FillWindowPixelRect(WIN_BOTTOM_BOX, PIXEL_FILL(0), (ev) ? EV_X  : IV_X + 2, 4, 18, 13);
    FillWindowPixelRect(WIN_BOTTOM_BOX, PIXEL_FILL(0),  EV_X, 4, 18, 13);

    if(ev)
    {
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, EV_X, 4, sBlackTextColor, 0, gStringVar2);
        
    }
    // else
    // {
    //     AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, IV_X + 2, 4, sBlackTextColor, 0, gStringVar3);    
    // }

    //PutWindowTilemap(0);
    PutWindowTilemap(1);
    PutWindowTilemap(2);
}

static void EvIvPrintText(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 nature   = GetNature(mon);
    u8 isEgg    = GetMonData(mon, MON_DATA_IS_EGG, NULL);
    u8 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);

    //reinicia los totales.
    //reset the totals.
    gEvIv->totalStatsEV = 0;
    gEvIv->totalStatsIV = 0;
    gEvIv->totalStatsBS = 0;

    //obtiene las estadísticas del pokémon
    //get pokémon stats

    //STAT_HP
    gEvIv->stats_bs[STAT_HP] = gBaseStatsPtr[species].baseHP;
    gEvIv->stats_ev[STAT_HP] = GetMonData(mon,MON_DATA_HP_EV,NULL);
    gEvIv->stats_iv[STAT_HP] = GetMonData(mon,MON_DATA_HP_IV,NULL);

    //STAT_ATK
    gEvIv->stats_bs[STAT_ATK] = gBaseStatsPtr[species].baseAttack;
    gEvIv->stats_ev[STAT_ATK] = GetMonData(mon,MON_DATA_ATK_EV,NULL);
    gEvIv->stats_iv[STAT_ATK] = GetMonData(mon,MON_DATA_ATK_IV,NULL);

    //STAT_DEF
    gEvIv->stats_bs[STAT_DEF] = gBaseStatsPtr[species].baseDefense;
    gEvIv->stats_ev[STAT_DEF] = GetMonData(mon,MON_DATA_DEF_EV,NULL);
    gEvIv->stats_iv[STAT_DEF] = GetMonData(mon,MON_DATA_DEF_IV,NULL);

    //STAT_SPATK
    gEvIv->stats_bs[STAT_SPATK] = gBaseStatsPtr[species].baseSpAttack;
    gEvIv->stats_ev[STAT_SPATK] = GetMonData(mon,MON_DATA_SPATK_EV,NULL);
    gEvIv->stats_iv[STAT_SPATK] = GetMonData(mon,MON_DATA_SPATK_IV,NULL);
    
    //STAT_SPDEF
    gEvIv->stats_bs[STAT_SPDEF] = gBaseStatsPtr[species].baseSpDefense;
    gEvIv->stats_ev[STAT_SPDEF] = GetMonData(mon,MON_DATA_SPDEF_EV,NULL);
    gEvIv->stats_iv[STAT_SPDEF] = GetMonData(mon,MON_DATA_SPDEF_IV,NULL);
    
    //STAT_SPEED
    gEvIv->stats_bs[STAT_SPEED] = gBaseStatsPtr[species].baseSpeed;
    gEvIv->stats_ev[STAT_SPEED] = GetMonData(mon,MON_DATA_SPEED_EV,NULL);
    gEvIv->stats_iv[STAT_SPEED] = GetMonData(mon,MON_DATA_SPEED_IV,NULL);

    //realiza la suma de los totales
    //performs the sum of the totals
    for (int i = 0; i < NUM_STATS; i++)
    {
        gEvIv->totalStatsEV += gEvIv->stats_ev[i];
        gEvIv->totalStatsIV += gEvIv->stats_iv[i];
        gEvIv->totalStatsBS += gEvIv->stats_bs[i];
    }

    FillWindowPixelBuffer(WIN_POKEMON_NAME, 0);
    FillWindowPixelBuffer(WIN_STATS, 0);
    FillWindowPixelBuffer(WIN_BOTTOM_BOX, 0);
    // FillWindowPixelBuffer(WIN_HIDDEN_POWER, 0);
    // FillWindowPixelBuffer(WIN_TYPE, 0);

    PrintWindow0(mon);
    PrintWindow1(nature, isEgg);
    PrintWindow2(species, isEgg, friendship, mon);
    u8 friendship2 = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);
    PrintWindow_HiddenPower(isEgg, friendship2);

    PutWindowTilemap(WIN_POKEMON_NAME);
    PutWindowTilemap(WIN_STATS);
    PutWindowTilemap(WIN_BOTTOM_BOX);
    // PutWindowTilemap(WIN_HIDDEN_POWER);
    // PutWindowTilemap(WIN_TYPE);
    CopyWindowToVram(WIN_TYPE, COPYWIN_GFX);
}

static void PrintWindow0(struct Pokemon *mon)
{
    u8 max_mon_count;
    u8 current_mon = gEvIv->cursorPos + 1;
    u8 x = 16;

    ConvertIntToDecimalStringN(gStringVar4, current_mon, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(current_mon));
    if (!IsMultiBattle())
    {
        StringAppend(gStringVar4, gText_eviv_Slash);
        max_mon_count = gEvIv->lastIdx + 1;
        ConvertIntToDecimalStringN(gStringVar1, max_mon_count, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(max_mon_count));
        StringAppend(gStringVar4, gStringVar1);

        if(gEvIv->isBoxMon)
            x = 4;
        else
            x = 10;
    }
    AddTextPrinterParameterized3(WIN_POKEMON_NAME, 2, x, 2, sGrayTextColor, 0, gStringVar4);

    AddTextPrinterParameterized3(WIN_POKEMON_NAME, 2, 0x3C, 2, sGrayTextColor, 0, gText_BsEvIv);

    GetMonNickname(mon, gStringVar4);
    AddTextPrinterParameterized3(WIN_POKEMON_NAME, 2, 0x91, 2, sGrayTextColor, 0, gStringVar4);

}

static void PrintWindow1(u8 nature, u8 isEgg)
{
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, HP_Y,    sTextColorByNature[GetColorByNature(nature, STAT_HP)],    0, gText_eviv_Hp);
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, ATK_Y,   sTextColorByNature[GetColorByNature(nature, STAT_ATK)],   0, gText_eviv_Atk);
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, DEF_Y,   sTextColorByNature[GetColorByNature(nature, STAT_DEF)],   0, gText_eviv_Def);
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, SPATK_Y, sTextColorByNature[GetColorByNature(nature, STAT_SPATK)], 0, gText_eviv_SpAtk);
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, SPDEF_Y, sTextColorByNature[GetColorByNature(nature, STAT_SPDEF)], 0, gText_eviv_SpDef);
    // AddTextPrinterParameterized3(WIN_STATS, 2, HP_X, SPEED_Y, sTextColorByNature[GetColorByNature(nature, STAT_SPEED)], 0, gText_eviv_Speed);

    if (!isEgg)
    {
        PrintStat(nature, STAT_HP);
        PrintStat(nature, STAT_ATK);
        PrintStat(nature, STAT_DEF);
        PrintStat(nature, STAT_SPATK);
        PrintStat(nature, STAT_SPDEF);
        PrintStat(nature, STAT_SPEED);
    }else{
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, HP_Y,    sBlackTextColor, 0, gText_CensorEgg);
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, ATK_Y,   sBlackTextColor, 0, gText_CensorEgg);
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, DEF_Y,   sBlackTextColor, 0, gText_CensorEgg);
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPATK_Y, sBlackTextColor, 0, gText_CensorEgg);
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPDEF_Y, sBlackTextColor, 0, gText_CensorEgg);
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPEED_Y, sBlackTextColor, 0, gText_CensorEgg);
    }
}

static void PrintStat(u8 nature, u8 stat)
{
    u8 color_idx = GetColorByNature(nature, stat);

    ConvertIntToDecimalStringN(gStringVar1, gEvIv->stats_bs[stat], STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, gEvIv->stats_ev[stat], STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, gEvIv->stats_iv[stat], STR_CONV_MODE_RIGHT_ALIGN, 2);

    switch (stat)
    {
    case STAT_HP:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, HP_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, HP_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, HP_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    case STAT_ATK:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, ATK_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, ATK_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, ATK_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    case STAT_DEF:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, DEF_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, DEF_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, DEF_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    case STAT_SPATK:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPATK_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, SPATK_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, SPATK_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    case STAT_SPDEF:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPDEF_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, SPDEF_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, SPDEF_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    case STAT_SPEED:
        AddTextPrinterParameterized3(WIN_STATS, 2, BS_X, SPEED_Y, sTextColorByNature[color_idx], 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_STATS, 2, EV_X, SPEED_Y, sTextColorByNature[color_idx], 0, gStringVar2);
        AddTextPrinterParameterized3(WIN_STATS, 2, IV_X, SPEED_Y, sTextColorByNature[color_idx], 0, gStringVar3);
        break;
    default:
        break;
    }
}

static void PrintWindow2(u16 species, u8 isEgg, u8 friendship, struct Pokemon *mon)
{
    u32 friendship_result = 0;
    u16 temp = 0;
    if(!isEgg)
    {
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, 12, 4, sBlackTextColor, 0, gText_eviv_Total);

        ConvertIntToDecimalStringN(gStringVar1, gEvIv->totalStatsBS, STR_CONV_MODE_RIGHT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, gEvIv->totalStatsEV, STR_CONV_MODE_RIGHT_ALIGN, 3);
        // ConvertIntToDecimalStringN(gStringVar3, gEvIv->totalStatsIV, STR_CONV_MODE_RIGHT_ALIGN, 3);
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, BS_X, 4, sBlackTextColor, 0, gStringVar1);
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, EV_X, 4, sBlackTextColor, 0, gStringVar2);
        // AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 2, IV_X -6, 4, sBlackTextColor, 0, gStringVar3);

        StringCopy(gStringVar3, gText_Happy);
        temp = friendship;
        ConvertIntToDecimalStringN(gStringVar2, temp, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(temp));
        StringAppend(gStringVar3, gStringVar2);
        StringAppend(gStringVar3, gText_Lv);
        u8 level = GetMonData(mon, MON_DATA_LEVEL);
        ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(level));
        StringAppend(gStringVar3, gStringVar1);
        StringAppend(gStringVar3, gText_eviv_Slash);
        level = GetCurrentLevelCap();
        ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(level));
        StringAppend(gStringVar3, gStringVar1);
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 0, IV_X -5,4,  sBlackTextColor, 0, gStringVar3);

        
        u16 evoMethod = gEvolutionTable[species][0].method;
        StringCopy(gStringVar4, gText_Tu);
        if (species == SPECIES_EEVEE ) {
                StringAppend(gStringVar4,  gText_EvolvesVariousWays);
        } else if (species == SPECIES_GLOOM || species == SPECIES_PETILIL || species == SPECIES_EXEGGCUTE ){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemLeafStone);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemSunStone);
        }
        else if (species == SPECIES_APPLIN ){
                StringAppend(gStringVar4, gText_EvoApplin); 
        }
        else if (species == SPECIES_SLOWPOKE || species == SPECIES_SLOWPOKE_G){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                StringAppend(gStringVar4, gText_EvoSlowpokeStuff);
        }
        else if (species == SPECIES_SCYTHER){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemMetalCoat);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemKingsRock);
                StringAppend(gStringVar4, gText_Period);
        }
        else if (species == SPECIES_PIKACHU){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_PikachuStuff);
        }
        else if (species == SPECIES_KIRLIA){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                StringAppend(gStringVar4, gText_EvoKirliaStuff);
        }
        else if (species == SPECIES_SNORUNT){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                StringAppend(gStringVar4, gText_EvoSnoruntStuff);
        }
        else if (species == SPECIES_POLIWHIRL){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemWaterStone);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemKingsRock);
        }
        else if (species == SPECIES_CLAMPERL){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemWaterStone);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemSunStone);
        }
        else if (species == SPECIES_KUBFU){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemWaterStone);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemDuskStone);
        }
        else if (species == SPECIES_CHARCADET){
                StringAppend(gStringVar4, gText_EvoItem); 
                StringAppend(gStringVar4, gText_EvoItemSunStone);
                StringAppend(gStringVar4, gText_EvoOr);
                StringAppend(gStringVar4, gText_EvoItemMoonStone);
        }
        else if (species == SPECIES_QUILAVA || species == SPECIES_DARTRIX || species == SPECIES_DEWOTT || species == SPECIES_RUFFLET || species == SPECIES_CUBONE || species == SPECIES_ROCKRUFF || species == SPECIES_KOFFING || species == SPECIES_COSMOEM){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                temp = gEvolutionTable[species][0].param;
                ConvertIntToDecimalStringN(gStringVar2, temp, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(temp));
                StringAppend(gStringVar4, gStringVar2);
                // AddNumInto_gStringVar4(temp);
                StringAppend(gStringVar4, gText_DependingOnTimeOfDay);
        }
        else if (species == SPECIES_BERGMITE){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                StringAppend(gStringVar4, gText_EvoAvaluggStuff);
        }
        else if (species == SPECIES_GOOMY){
                StringAppend(gStringVar4, gText_EvolvesLevel); 
                StringAppend(gStringVar4, gText_EvoGoomyStuff);
        }
        else if (species == SPECIES_MIME_JR){
            StringAppend(gStringVar4, gText_EvoWhenKnowing); 
            StringAppend(gStringVar4, gText_EvoMimic); 
            StringAppend(gStringVar4, gText_EvoMimeJrStuff); 
        }

        else {
            switch (evoMethod) {
                case EVO_LEVEL:
                case EVO_LEVEL_ATK_GT_DEF:
                case EVO_LEVEL_ATK_EQ_DEF:
                case EVO_LEVEL_ATK_LT_DEF:
                case EVO_LEVEL_SILCOON:
                case EVO_LEVEL_CASCOON:
                case EVO_LEVEL_NINJASK:
                case EVO_NATURE_TOXTRICITY:
                case EVO_NATURE_LOWKEY:
                case EVO_LEVEL_SPECIFIC_TIME_RANGE:
                case EVO_MALE_LEVEL:
                case EVO_FEMALE_LEVEL:
                case EVO_TYPE_IN_PARTY:
                case EVO_LEVEL_DAY:
                case EVO_LEVEL_NIGHT:
                    StringAppend(gStringVar4, gText_EvolvesLevel);
                    temp = gEvolutionTable[species][0].param;
                    ConvertIntToDecimalStringN(gStringVar2, temp, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(temp));
                    StringAppend(gStringVar4, gStringVar2);
                    switch (evoMethod){
                        case (EVO_MALE_LEVEL):
                            if (species == SPECIES_BASCULIN_RED || species == SPECIES_BASCULIN_BLUE || species == SPECIES_ESPURR ) {
                                StringAppend(gStringVar4, gText_EvoGenderDepend);
                            }
                            else 
                                StringAppend(gStringVar4, gText_EvoIfMale);
                            break;
                        case (EVO_FEMALE_LEVEL):
                            if (species == SPECIES_BURMY_TRASH || species == SPECIES_BURMY || species == SPECIES_BURMY_SANDY || species == SPECIES_LECHONK ) {
                                StringAppend(gStringVar4, gText_EvoGenderDepend);
                            }
                            else 
                                StringAppend(gStringVar4, gText_EvoIfFemale);
                            break;
                        case (EVO_TYPE_IN_PARTY ):
                            //only pancham rn
                            StringAppend(gStringVar4, gText_EvoMethodIf);
                            StringAppend(gStringVar4, gText_EvoMethodDark);
                            StringAppend(gStringVar4, gText_EvoTypeInParty);
                            break;
                        case (EVO_LEVEL_DAY):
                            if (species != SPECIES_COSMOEM) {
                                StringAppend(gStringVar4, gText_EvoIfDay);
                            } 
                            break;
                        case (EVO_LEVEL_NIGHT):
                            StringAppend(gStringVar4, gText_EvoIfNight);
                            break;
                    }
                    StringAppend(gStringVar4, gText_Period);
                    break;
    
                case EVO_MOVE:
                    StringAppend(gStringVar4, gText_EvoWhenKnowing);
                    temp = gEvolutionTable[species][0].param;
                    switch (temp) {
                        case MOVE_ANCIENTPOWER:
                            StringAppend(gStringVar4, gText_EvoAncientPower); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_DOUBLEHIT:
                            StringAppend(gStringVar4, gText_EvoDoubleHit); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_MIMIC:
                            StringAppend(gStringVar4, gText_EvoMimic); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_STOMP:
                            StringAppend(gStringVar4, gText_EvoStomp); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_DRAGONPULSE:
                            StringAppend(gStringVar4, gText_EvoDragonPulse); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_TAUNT:
                            StringAppend(gStringVar4, gText_EvoTaunt); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_SHADOWPUNCH:
                            StringAppend(gStringVar4, gText_EvoShadowPunch); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_HYPERDRILL:
                            StringAppend(gStringVar4, gText_EvoHyperDrill); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_TWINBEAM:
                            StringAppend(gStringVar4, gText_EvoTwinBeam); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case MOVE_ROLLOUT:
                            StringAppend(gStringVar4, gText_EvoRollout); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                    }
                    break;

                case EVO_ITEM:
                    StringAppend(gStringVar4, gText_EvoItem); 
                    temp = gEvolutionTable[species][0].param;
                    switch(temp) {
                        case ITEM_LINK_CABLE:
                            StringAppend(gStringVar4, gText_EvoItemLinkCable); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_SUN_STONE:
                            StringAppend(gStringVar4, gText_EvoItemSunStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_MOON_STONE:
                            StringAppend(gStringVar4, gText_EvoItemMoonStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_FIRE_STONE:
                            StringAppend(gStringVar4, gText_EvoItemFireStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_THUNDER_STONE:
                            StringAppend(gStringVar4, gText_EvoItemThunderStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_WATER_STONE:
                            StringAppend(gStringVar4, gText_EvoItemWaterStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_LEAF_STONE:
                            StringAppend(gStringVar4, gText_EvoItemLeafStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_SHINY_STONE:
                            StringAppend(gStringVar4, gText_EvoItemShinyStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_DUSK_STONE:
                            StringAppend(gStringVar4, gText_EvoItemDuskStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_DAWN_STONE:
                            StringAppend(gStringVar4, gText_EvoItemDawnStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_ICE_STONE:
                            StringAppend(gStringVar4, gText_EvoItemIceStone); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_UP_GRADE:
                            StringAppend(gStringVar4, gText_EvoItemUpgrade); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_KINGS_ROCK:
                            StringAppend(gStringVar4, gText_EvoItemKingsRock); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_METAL_COAT:
                            StringAppend(gStringVar4, gText_EvoItemMetalCoat); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                        case ITEM_PRISM_SCALE:
                            StringAppend(gStringVar4, gText_EvoItemPrismScale); 
                            StringAppend(gStringVar4, gText_Period);
                            break;
                    }
                    break;

                case EVO_FRIENDSHIP:
                    StringAppend(gStringVar4, gText_EvosFriendship); 
                    break;
                case EVO_FRIENDSHIP_DAY:
                    StringAppend(gStringVar4, gText_EvolvesFriendshipDay);
                    break;
                case EVO_FRIENDSHIP_NIGHT:
                    StringAppend(gStringVar4, gText_EvolvesFriendshipNight);
                    break;
                case EVO_RAINY_FOGGY_OW:
                    StringAppend(gStringVar4, gText_EvolvesLevel);
                    temp = gEvolutionTable[species][0].param;
                    ConvertIntToDecimalStringN(gStringVar2, temp, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(temp));
                    StringAppend(gStringVar4, gStringVar2);
                    StringAppend(gStringVar4, gText_inRain);
                    break;
                default:
                    StringAppend(gStringVar4, gText_NoEvolution);
                    break;
            }
        }
        // AddTextPrinterParameterized3(2, 0x2, 0x7, 0x11, sBlackTextColor, 0, gStringVar4);
        // StringCopy(gStringVar4, gText_Happy);
        
        // friendship_result = (friendship * 100) / 0xFF;
        // ConvertIntToDecimalStringN(gStringVar2, friendship_result, STR_CONV_MODE_LEFT_ALIGN, 3);
        // StringAppend(gStringVar4, gStringVar2);
        
        // StringAppend(gStringVar4, gText_Percent);
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 0, 7, 18, sBlackTextColor, 0, gStringVar4);
        
    }else
    {
        friendship_result = ((friendship + 1) * 0x100) - (gSaveBlock1Ptr->daycare.stepCounter + 1);
        if (gSaveBlock1Ptr->daycare.stepCounter == 0xFF)
            friendship_result += 0x100;
        StringCopy(gStringVar4, gText_Steps_to_hatching);
        ConvertIntToDecimalStringN(gStringVar2, friendship_result, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(friendship_result));
        StringAppend(gStringVar4, gStringVar2);
        AddTextPrinterParameterized3(WIN_BOTTOM_BOX, 0, 7, 4, sBlackTextColor, 0, gStringVar4);
    }
}

// static u8 GetPower_HiddenPower(void)
// {
// #if HIDDEN_POWER_STATIC
// 	return HIDDEN_POWER_BASE_POWER;
// #else
//     s32 powerBits;

//     powerBits = ((gEvIv->stats_iv[STAT_HP] & 2) >> 1)
//               | ((gEvIv->stats_iv[STAT_ATK] & 2) << 0)
//               | ((gEvIv->stats_iv[STAT_DEF] & 2) << 1)
//               | ((gEvIv->stats_iv[STAT_SPEED] & 2) << 2)
//               | ((gEvIv->stats_iv[STAT_SPATK] & 2) << 3)
//               | ((gEvIv->stats_iv[STAT_SPDEF] & 2) << 4);

//     return (40 * powerBits) / 63 + 30;
// #endif
// }

// static u8 GetType_HiddenPower(void)
// {
//     u8 type;
//     s32 typeBits;

//     typeBits  = ((gEvIv->stats_iv[STAT_HP] & 1) << 0)
//               | ((gEvIv->stats_iv[STAT_ATK] & 1) << 1)
//               | ((gEvIv->stats_iv[STAT_DEF] & 1) << 2)
//               | ((gEvIv->stats_iv[STAT_SPEED] & 1) << 3)
//               | ((gEvIv->stats_iv[STAT_SPATK] & 1) << 4)
//               | ((gEvIv->stats_iv[STAT_SPDEF] & 1) << 5);

//     type = (15 * typeBits) / 63 + 1;

//     if (type >= TYPE_MYSTERY)
//         type++;

//     return type;
// }

static void PrintWindow_HiddenPower(u8 isEgg, u8 friendship)
{
    // u8 power = GetPower_HiddenPower();
    // u8 type = GetType_HiddenPower();

    // const u8 gText_q[2] = _("?");

    StringCopy(gStringVar4, gText_HiddenPower);
    StringCopy(gStringVar4, gText_Happy);
    u32 temp = friendship;
    ConvertIntToDecimalStringN(gStringVar2, temp, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(temp));
    StringAppend(gStringVar4, gStringVar2);
    AddTextPrinterParameterized3(WIN_HIDDEN_POWER, 0, 4, 3, sBlackTextColor, 0, gStringVar4);

    // if (!isEgg)
    // {
    //     ConvertIntToDecimalStringN(gStringVar1, power, STR_CONV_MODE_LEFT_ALIGN, GetDigitsDec(power));
    //     StringCopy(gStringVar4, gText_Power);
    //     StringAppend(gStringVar4, gStringVar1);
    //     AddTextPrinterParameterized3(WIN_HIDDEN_POWER, 0, 16, 16, sWhiteTextColor, 0, gStringVar4);
    //     BlitMoveInfoIcon(WIN_TYPE, type + 1, 2, 4);
    // }
    // else
    //     AddTextPrinterParameterized3(WIN_HIDDEN_POWER, 0, 32, 16, sWhiteTextColor, 0, gText_q);
}

/**
 * Devuelve el número de dígitos de un número
 * 
 * Returns the number of digits in a number
*/
static u8 GetDigitsDec(u32 num)
{
    u8 digits = 1;
    while (num >= 10)
    {
        digits++;
        num = num / 10;
    }
    return digits;
    /*
    if (num < 10)
        return 1;
    else
        return 1 + GetDigitsDec(num/10);
    */
}

static u8 GetDigitsHex(u32 num)
{
    u8 digits = 1;
    while (num >= 0x10)
    {
        digits++;
        num = num / 0x10;
    }
    return digits;
    /*
    if (num < 0x10)
        return 1;
    else
        return 1 + GetDigitsHex(num/0x10);
    */
}


/**
 * Devuelve el número de dígitos de un entero con signo
 * 
 * Returns the number of digits in a signed int
*/
u8 GetDigitsDec_Signed(s32 num)
{
    if (num < 0)
        num = num * (-1);
    
    return GetDigitsDec(num);
}

u8 GetDigitsHex_Signed(s32 num)
{
    if (num < 0)
        num = num * (-1);
    
    return GetDigitsHex(num);
}

const s8 gNatureStatTable_copy[NUM_NATURES][NUM_NATURE_STATS] =
{
                    //  Atk             Def             Spd             Sp.Atk          Sp.Def
    [NATURE_HARDY]   = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_LONELY]  = {STAT_INCREASE,  STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_BRAVE]   = {STAT_INCREASE,  STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_ADAMANT] = {STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL},
    [NATURE_NAUGHTY] = {STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE},
    [NATURE_BOLD]    = {STAT_DECREASE,  STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_DOCILE]  = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_RELAXED] = {STAT_NEUTRAL,   STAT_INCREASE,  STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_IMPISH]  = {STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL},
    [NATURE_LAX]     = {STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE},
    [NATURE_TIMID]   = {STAT_DECREASE,  STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_HASTY]   = {STAT_NEUTRAL,   STAT_DECREASE,  STAT_INCREASE,  STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_SERIOUS] = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_JOLLY]   = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE,  STAT_DECREASE,  STAT_NEUTRAL},
    [NATURE_NAIVE]   = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL,   STAT_DECREASE},
    [NATURE_MODEST]  = {STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL},
    [NATURE_MILD]    = {STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL,   STAT_INCREASE,  STAT_NEUTRAL},
    [NATURE_QUIET]   = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE,  STAT_INCREASE,  STAT_NEUTRAL},
    [NATURE_BASHFUL] = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
    [NATURE_RASH]    = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE,  STAT_DECREASE},
    [NATURE_CALM]    = {STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE},
    [NATURE_GENTLE]  = {STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_INCREASE},
    [NATURE_SASSY]   = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE,  STAT_NEUTRAL,   STAT_INCREASE},
    [NATURE_CAREFUL] = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_DECREASE,  STAT_INCREASE},
    [NATURE_QUIRKY]  = {STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL,   STAT_NEUTRAL},
};

static u8 GetColorByNature(u8 nature, u8 statIndex)
{
    if (statIndex < STAT_ATK || statIndex > STAT_SPDEF)
        return STAT_NEUTRAL + 1;

    return gNatureStatTable_copy[nature][statIndex - 1] + 1;
}

#ifdef FIRERED
//08134840 l 0000036c Task_InputHandler_Info

//static 
extern void PokeSum_TryPlayMonCry(void);
extern void Task_PokeSum_SwitchDisplayedPokemon(u8 taskId);
extern bool32 IsPageFlipInput(u8 direction);
extern void Task_PokeSum_FlipPages(u8 taskId);
extern void PokeSum_SeekToNextMon(u8 taskId, s8 direction);
extern void Task_FlipPages_FromInfo(u8 taskId);
extern void Task_DestroyResourcesOnExit(u8 taskId);
extern void PokeSum_DestroySprites(void);

//static 
void New_Task_InputHandler_Info(u8 taskId)
{
    switch (sMonSummaryScreen->state3270) {
    case PSS_STATE3270_FADEIN:
        BeginNormalPaletteFade(0xffffffff, 0, 16, 0, 0);
        sMonSummaryScreen->state3270 = PSS_STATE3270_PLAYCRY;
        break;
    case PSS_STATE3270_PLAYCRY:
        if (!gPaletteFade.active)
        {
            PokeSum_TryPlayMonCry();
            sMonSummaryScreen->state3270 = PSS_STATE3270_HANDLEINPUT;
            return;
        }

        sMonSummaryScreen->state3270 = PSS_STATE3270_PLAYCRY;
        break;
    case PSS_STATE3270_HANDLEINPUT:
        if (MenuHelpers_CallLinkSomething() == TRUE)
            return;
        else if (LinkRecvQueueLengthMoreThan2() == TRUE)
            return;
        else if (FuncIsActiveTask(Task_PokeSum_SwitchDisplayedPokemon))
            return;

        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        {
            if (IsPageFlipInput(1) == TRUE)
            {
                if (FuncIsActiveTask(Task_PokeSum_FlipPages))
                {
                    sMonSummaryScreen->lastPageFlipDirection = 1;
                    return;
                }
                else if (sMonSummaryScreen->curPageIndex < PSS_PAGE_MOVES)
                {
                    PlaySE(SE_SELECT);
                    HideBg(0);
                    sMonSummaryScreen->pageFlipDirection = 1;
                    PokeSum_RemoveWindows(sMonSummaryScreen->curPageIndex);
                    sMonSummaryScreen->curPageIndex++;
                    sMonSummaryScreen->state3270 = PSS_STATE3270_FLIPPAGES;
                }
                return;
            }
            else if (IsPageFlipInput(0) == TRUE)
            {
                if (FuncIsActiveTask(Task_PokeSum_FlipPages))
                {
                    sMonSummaryScreen->lastPageFlipDirection = 0;
                    return;
                }
                else if (sMonSummaryScreen->curPageIndex > PSS_PAGE_INFO)
                {
                    PlaySE(SE_SELECT);
                    HideBg(0);
                    sMonSummaryScreen->pageFlipDirection = 0;
                    PokeSum_RemoveWindows(sMonSummaryScreen->curPageIndex);
                    sMonSummaryScreen->curPageIndex--;
                    sMonSummaryScreen->state3270 = PSS_STATE3270_FLIPPAGES;
                }
                return;
            }
        }

        if ((!FuncIsActiveTask(Task_PokeSum_FlipPages)) || FuncIsActiveTask(Task_PokeSum_SwitchDisplayedPokemon))
        {
            if (JOY_NEW(DPAD_UP))
            {
                PokeSum_SeekToNextMon(taskId, -1);
                return;
            }
            else if (JOY_NEW(DPAD_DOWN))
            {
                PokeSum_SeekToNextMon(taskId, 1);
                return;
            }
            else if (JOY_NEW(A_BUTTON))
            {
                if (sMonSummaryScreen->curPageIndex == PSS_PAGE_INFO)
                {
                    PlaySE(SE_SELECT);
                    sMonSummaryScreen->state3270 = PSS_STATE3270_ATEXIT_FADEOUT;
                }
                else if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS && FlagGet(FLAG_EV_IV) && !FlagGet(0x908) && !gMain.inBattle)
                {
                    sMonSummaryScreen->state3270 = PSS_STATE3270_EV_IV;
                    BeginNormalPaletteFade(0xffffffff, 0, 0, 16, RGB_BLACK);
                    PlaySE(SE_CARD_OPEN);   
                }
                else if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES)
                {
                    PlaySE(SE_SELECT);
                    sMonSummaryScreen->pageFlipDirection = 1;
                    PokeSum_RemoveWindows(sMonSummaryScreen->curPageIndex);
                    sMonSummaryScreen->curPageIndex++;
                    sMonSummaryScreen->state3270 = PSS_STATE3270_FLIPPAGES;
                }
                return;
            }
            else if (JOY_NEW(B_BUTTON))
            {
                sMonSummaryScreen->state3270 = PSS_STATE3270_ATEXIT_FADEOUT;
            }
        }
        break;
    case PSS_STATE3270_FLIPPAGES:
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        {
            CreateTask(Task_PokeSum_FlipPages, 0);
            sMonSummaryScreen->state3270 = PSS_STATE3270_HANDLEINPUT;
        }
        else
        {
            gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_FlipPages_FromInfo;
            sMonSummaryScreen->state3270 = PSS_STATE3270_HANDLEINPUT;
        }
        break;
    case PSS_STATE3270_ATEXIT_FADEOUT:
        BeginNormalPaletteFade(0xffffffff, 0, 0, 16, 0);
        sMonSummaryScreen->state3270 = PSS_STATE3270_ATEXIT_WAITLINKDELAY;
        break;
    case PSS_STATE3270_ATEXIT_WAITLINKDELAY:
        if (Overworld_LinkRecvQueueLengthMoreThan2() == TRUE)
            return;
        else if (LinkRecvQueueLengthMoreThan2() == TRUE)
            return;

        sMonSummaryScreen->state3270 = PSS_STATE3270_ATEXIT_WAITFADE;
        break;
    case PSS_STATE3270_EV_IV:
        if (!gPaletteFade.active)
        {
            SetMainCallback2(CB2_ShowEvIv_SummaryScreen);
            PokeSum_DestroySprites();
            FreeAllWindowBuffers();
            DestroyTask(taskId);
        }
        break;
    default:
        if (!gPaletteFade.active)
            Task_DestroyResourcesOnExit(taskId);

        break;
    }
}

//static
extern void PokeSum_PrintPageName(const u8 * str);
extern void PokeSum_PrintControlsString(const u8 * str);
extern void PrintMonLevelNickOnWindow2(const u8 * str);

//case PSS_PAGE_SKILLS:
void PokeSum_PrintPageHeaderText_new(void)
{
    PokeSum_PrintPageName(gText_PokeSum_PageName_PokemonSkills);

    if (FlagGet(FLAG_EV_IV) && !FlagGet(0x908) && !gMain.inBattle)
        PokeSum_PrintControlsString(gText_PokeSum_Controls_Page_EvIv);
    else
        PokeSum_PrintControlsString(gText_PokeSum_Controls_Page);
    PrintMonLevelNickOnWindow2(gText_PokeSum_NoData);
}
#else//EMERALD

#define PALETTES_BG      0x0000FFFF
#define PALETTES_OBJECTS 0xFFFF0000
#define PALETTES_ALL     (PALETTES_BG | PALETTES_OBJECTS)

extern bool8 MenuHelpers_ShouldWaitForLinkRecv(void);

static void CloseSummaryScreen_AndCallEvIv(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE && !gPaletteFade.active)
    {
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_ShowEvIv_SummaryScreen);
        DestroyTask(taskId);
    }
}

extern u8 GetLRKeysPressed(void);
extern void ChangeSummaryPokemon(u8 taskId, s8 delta);
extern void ChangePage(u8 taskId, s8 delta);
extern void StopPokemonAnimations(void);
extern void BeginCloseSummaryScreen(u8 taskId);
extern void SwitchToMoveSelection(u8 taskId);

//static 
void Task_HandleInput(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE && !gPaletteFade.active)
    {
        if (JOY_NEW(DPAD_UP))
        {
            ChangeSummaryPokemon(taskId, -1);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            ChangeSummaryPokemon(taskId, 1);
        }
        else if ((JOY_NEW(DPAD_LEFT)) || GetLRKeysPressed() == MENU_L_PRESSED)
        {
            ChangePage(taskId, -1);
        }
        else if ((JOY_NEW(DPAD_RIGHT)) || GetLRKeysPressed() == MENU_R_PRESSED)
        {
            ChangePage(taskId, 1);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            if (sMonSummaryScreen->currPageIndex != PSS_PAGE_SKILLS)
            {
                if (sMonSummaryScreen->currPageIndex == PSS_PAGE_INFO)
                {
                    StopPokemonAnimations();
                    PlaySE(SE_SELECT);
                    BeginCloseSummaryScreen(taskId);
                }
                else // Contest or Battle Moves
                {
                    PlaySE(SE_SELECT);
                    SwitchToMoveSelection(taskId);
                }
            }
            else if (FlagGet(FLAG_EV_IV)&& !FlagGet(0x908) && !gMain.inBattle)
            {
                //StopPokemonAnimations();
                PlaySE(SE_RG_CARD_OPEN);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
                gTasks[taskId].func = CloseSummaryScreen_AndCallEvIv;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            StopPokemonAnimations();
            PlaySE(SE_SELECT);
            BeginCloseSummaryScreen(taskId);
        }
    }
}


enum {
    FONT_SMALL,
    FONT_NORMAL,
    FONT_SHORT,
    FONT_SHORT_COPY_1,
    FONT_SHORT_COPY_2,
    FONT_SHORT_COPY_3,
    FONT_BRAILLE,
    FONT_NARROW,
    FONT_SMALL_NARROW, // Very similar to FONT_SMALL, some glyphs are narrower
    FONT_BOLD, // JP glyph set only
};

extern int GetStringRightAlignXOffset(int fontId, const u8 *str, int totalWidth);
extern void PrintAOrBButtonIcon(u8 windowId, bool8 bButton, u32 x);
extern void PrintTextOnWindow(u8 windowId, const u8 *string, u8 x, u8 y, u8 lineSpacing, u8 colorId);
extern bool8 InBattleFactory(void);
extern bool8 InSlateportBattleTent(void);
extern void RemoveWindowByIndex(u8 windowIndex);

void PutPageWindowTilemaps(u8 page)
{
    u8 i;
    int stringXPos;
    int iconXPos;

    ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_TITLE);
    ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_TITLE);
    ClearWindowTilemap(PSS_LABEL_WINDOW_BATTLE_MOVES_TITLE);
    ClearWindowTilemap(PSS_LABEL_WINDOW_CONTEST_MOVES_TITLE);

    switch (page)
    {
    case PSS_PAGE_INFO:
        if (FlagGet(FLAG_EV_IV) && !FlagGet(0x908) && !gMain.inBattle)
        {
            FillWindowPixelBuffer(PSS_LABEL_WINDOW_PROMPT_CANCEL, PIXEL_FILL(0));
            stringXPos = GetStringRightAlignXOffset(FONT_NORMAL, gText_Cancel2, 62);
            iconXPos = stringXPos - 16;
            if (iconXPos < 0)
                iconXPos = 0;
            PrintAOrBButtonIcon(PSS_LABEL_WINDOW_PROMPT_CANCEL, FALSE, iconXPos);
            PrintTextOnWindow(PSS_LABEL_WINDOW_PROMPT_CANCEL, gText_Cancel2, stringXPos, 1, 0, 0);
        }
        
        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_TITLE);
        PutWindowTilemap(PSS_LABEL_WINDOW_PROMPT_CANCEL);
        if (InBattleFactory() == TRUE || InSlateportBattleTent() == TRUE)
            PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_RENTAL);
        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_TYPE);
        break;
    case PSS_PAGE_SKILLS:
        if (FlagGet(FLAG_EV_IV) && !FlagGet(0x908) && !gMain.inBattle )
        {
            FillWindowPixelBuffer(PSS_LABEL_WINDOW_PROMPT_CANCEL, PIXEL_FILL(0));
            stringXPos = GetStringRightAlignXOffset(FONT_NORMAL, gText_PokeSum_EvIv, 62);
            iconXPos = stringXPos - 16;
            if (iconXPos < 0)
                iconXPos = 0;
            PrintAOrBButtonIcon(PSS_LABEL_WINDOW_PROMPT_CANCEL, FALSE, iconXPos);
            PrintTextOnWindow(PSS_LABEL_WINDOW_PROMPT_CANCEL, gText_PokeSum_EvIv, stringXPos, 1, 0, 0);
            PutWindowTilemap(PSS_LABEL_WINDOW_PROMPT_CANCEL);
        }

        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_TITLE);
        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_STATS_LEFT);
        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_STATS_RIGHT);
        PutWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_EXP);
        break;
    case PSS_PAGE_BATTLE_MOVES:
        PutWindowTilemap(PSS_LABEL_WINDOW_BATTLE_MOVES_TITLE);
        if (sMonSummaryScreen->mode == SUMMARY_MODE_SELECT_MOVE)
        {
            if (sMonSummaryScreen->newMove != MOVE_NONE || sMonSummaryScreen->firstMoveIndex != MAX_MON_MOVES)
                PutWindowTilemap(PSS_LABEL_WINDOW_MOVES_POWER_ACC);
        }
        else
        {
            PutWindowTilemap(PSS_LABEL_WINDOW_PROMPT_INFO);
        }
        break;
    case PSS_PAGE_CONTEST_MOVES:
        PutWindowTilemap(PSS_LABEL_WINDOW_CONTEST_MOVES_TITLE);
        if (sMonSummaryScreen->mode == SUMMARY_MODE_SELECT_MOVE)
        {
            if (sMonSummaryScreen->newMove != MOVE_NONE || sMonSummaryScreen->firstMoveIndex != MAX_MON_MOVES)
                PutWindowTilemap(PSS_LABEL_WINDOW_MOVES_APPEAL_JAM);
        }
        else
        {
            PutWindowTilemap(PSS_LABEL_WINDOW_PROMPT_INFO);
        }
        break;
    }

    for (i = 0; i < ARRAY_COUNT(sMonSummaryScreen->windowIds); i++)
        PutWindowTilemap(sMonSummaryScreen->windowIds[i]);

    ScheduleBgCopyTilemapToVram(0);
}

void ClearPageWindowTilemaps(u8 page)
{
    u8 i;

    switch (page)
    {
    case PSS_PAGE_INFO:
        ClearWindowTilemap(PSS_LABEL_WINDOW_PROMPT_CANCEL);
        if (InBattleFactory() == TRUE || InSlateportBattleTent() == TRUE)
            ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_RENTAL);
        ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_INFO_TYPE);
        break;
    case PSS_PAGE_SKILLS:
        if (FlagGet(FLAG_EV_IV) && !FlagGet(0x908) && !gMain.inBattle)
            ClearWindowTilemap(PSS_LABEL_WINDOW_PROMPT_CANCEL);

        ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_STATS_LEFT);
        ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_STATS_RIGHT);
        ClearWindowTilemap(PSS_LABEL_WINDOW_POKEMON_SKILLS_EXP);
        break;
    case PSS_PAGE_BATTLE_MOVES:
        if (sMonSummaryScreen->mode == SUMMARY_MODE_SELECT_MOVE)
        {
            if (sMonSummaryScreen->newMove != MOVE_NONE || sMonSummaryScreen->firstMoveIndex != MAX_MON_MOVES)
                ClearWindowTilemap(PSS_LABEL_WINDOW_MOVES_POWER_ACC);
        }
        else
        {
            ClearWindowTilemap(PSS_LABEL_WINDOW_PROMPT_INFO);
        }
        break;
    case PSS_PAGE_CONTEST_MOVES:
        if (sMonSummaryScreen->mode == SUMMARY_MODE_SELECT_MOVE)
        {
            if (sMonSummaryScreen->newMove != MOVE_NONE || sMonSummaryScreen->firstMoveIndex != MAX_MON_MOVES)
                ClearWindowTilemap(PSS_LABEL_WINDOW_MOVES_APPEAL_JAM);
        }
        else
        {
            ClearWindowTilemap(PSS_LABEL_WINDOW_PROMPT_INFO);
        }
        break;
    }

    for (i = 0; i < ARRAY_COUNT(sMonSummaryScreen->windowIds); i++)
        RemoveWindowByIndex(i);

    ScheduleBgCopyTilemapToVram(0);
}

#endif


static void UpdateCursorSpritePos(u16 spriteId, u8 stat, bool8 goingUp, bool8 resetY)
{
    struct Sprite * sprite = &gSprites[spriteId];
    u8 newPosX = sprite->x; 
    u8 newPosY = sprite->y;
    // u8 ability = GetMonAbility(&gEvIv->gcurrentMon);
    // u16 species = GetMonData(&gEvIv->currentMon, MON_DATA_SPECIES);

    if(stat == 0xFF)
    {
        switch(gSelectedColumn)
        {
            case EVS:
                newPosX = 112;
                break;
            case IVS:
                newPosX = 138;
                break;
            // case ABILITY_EDIT:
            //     CopyAbilityName(gStringVar1, ability, species);
            //     newPosX = (157 + (GetStringWidth(2, gStringVar1, 0) / 2)) - 5;
            //     newPosY = 121;
            //     break;
            case GENDER:
                newPosX = 164;
                newPosY = 24;
                break;
        }
    }
    else
    {
        if(!goingUp)
        {
            if(stat == EDITOR_STAT_HP)
                newPosY = 28;
            else
                newPosY += 14;
        }
        else
        {
            if(stat == EDITOR_STAT_SPD)
                newPosY = 98;
            else
                newPosY -= 14;
        }
    }
    if(resetY)
    {
        if(goingUp)
        {
            newPosX = 112;
            newPosY = 28;
        }
        else
        {
            newPosX = 138;
            newPosY = 28;
        }
    }

    sprite->x = newPosX;
    sprite->y = newPosY;
}

static void FixOddEVs(void)
{
    struct Pokemon * mon = &gEvIv->currentMon;
    for(int i = MON_DATA_HP_EV; i < MON_DATA_SPDEF_EV + 1; i++)
    {
        u16 ev = GetMonData(mon, i);
        u16 newEv;
        if(ev % 2 && ev != 0)
        {
            newEv = ev - 1;
            SetMonData(mon, i, &newEv);
            MiniEvIvPrintText(mon, TRUE, i, newEv, i - MON_DATA_HP_EV, FALSE);
        }
            

    }
}

