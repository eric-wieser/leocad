// Typedefs.
//

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

class Group;
class Piece;
class PieceInfo;
class Camera;

#include "str.h"
#include "lc_math.h"

enum LC_NOTIFY
{
	LC_COLOR_CHANGED,
	LC_CAPTURE_LOST,
	LC_ACTIVATE,
	LC_PIECE_MODIFIED,
	LC_CAMERA_MODIFIED,
	LC_LIGHT_MODIFIED
};

enum LC_COMMANDS
{
	LC_FILE_NEW,
	LC_FILE_OPEN,
	LC_FILE_MERGE,
	LC_FILE_SAVE,
	LC_FILE_SAVEAS,
	LC_FILE_PICTURE,
	LC_FILE_3DS,
	LC_FILE_HTML,
	LC_FILE_BRICKLINK,
	LC_FILE_POVRAY,
	LC_FILE_WAVEFRONT,
	LC_FILE_PROPERTIES,
	LC_FILE_TERRAIN,
	LC_FILE_LIBRARY,
	LC_FILE_RECENT,
	LC_EDIT_UNDO,
	LC_EDIT_REDO,
	LC_EDIT_CUT,
	LC_EDIT_COPY,
	LC_EDIT_PASTE,
	LC_EDIT_SELECT_ALL,
	LC_EDIT_SELECT_NONE,
	LC_EDIT_SELECT_INVERT,
	LC_EDIT_SELECT_BYNAME,
	LC_PIECE_INSERT,
	LC_PIECE_DELETE,
	LC_PIECE_MINIFIG,
	LC_PIECE_ARRAY,
	LC_PIECE_COPYKEYS,
	LC_PIECE_GROUP,
	LC_PIECE_UNGROUP,
	LC_PIECE_GROUP_ADD,
	LC_PIECE_GROUP_REMOVE,
	LC_PIECE_GROUP_EDIT,
	LC_PIECE_HIDE_SELECTED,
	LC_PIECE_HIDE_UNSELECTED,
	LC_PIECE_UNHIDE_ALL,
	LC_PIECE_PREVIOUS,
	LC_PIECE_NEXT,
	LC_VIEW_PREFERENCES,
	LC_VIEW_ZOOM,
	LC_VIEW_ZOOMIN,
	LC_VIEW_ZOOMOUT,
	LC_VIEW_ZOOMEXTENTS,
	LC_VIEW_STEP_NEXT,
	LC_VIEW_STEP_PREVIOUS,
	LC_VIEW_STEP_FIRST,
	LC_VIEW_STEP_LAST,
	LC_VIEW_STEP_CHOOSE,
	LC_VIEW_STEP_SET,
	LC_VIEW_STEP_INSERT,
	LC_VIEW_STEP_DELETE,
	LC_VIEW_STOP,
	LC_VIEW_PLAY,
	LC_VIEW_VIEWPOINT_FRONT,
	LC_VIEW_VIEWPOINT_BACK,
	LC_VIEW_VIEWPOINT_TOP,
	LC_VIEW_VIEWPOINT_BOTTOM,
	LC_VIEW_VIEWPOINT_LEFT,
	LC_VIEW_VIEWPOINT_RIGHT,
	LC_VIEW_VIEWPOINT_HOME,
	LC_VIEW_CAMERA_MENU,
	LC_VIEW_CAMERA_RESET,
	LC_HELP_ABOUT,
	LC_TOOLBAR_ANIMATION,
	LC_TOOLBAR_ADDKEYS,
	LC_TOOLBAR_SNAPMENU,
	LC_TOOLBAR_LOCKMENU,
	LC_TOOLBAR_FASTRENDER,
	LC_EDIT_MOVEXY_SNAP_0,
	LC_EDIT_MOVEXY_SNAP_1,
	LC_EDIT_MOVEXY_SNAP_2,
	LC_EDIT_MOVEXY_SNAP_3,
	LC_EDIT_MOVEXY_SNAP_4,
	LC_EDIT_MOVEXY_SNAP_5,
	LC_EDIT_MOVEXY_SNAP_6,
	LC_EDIT_MOVEXY_SNAP_7,
	LC_EDIT_MOVEXY_SNAP_8,
	LC_EDIT_MOVEXY_SNAP_9,
	LC_EDIT_MOVEZ_SNAP_0,
	LC_EDIT_MOVEZ_SNAP_1,
	LC_EDIT_MOVEZ_SNAP_2,
	LC_EDIT_MOVEZ_SNAP_3,
	LC_EDIT_MOVEZ_SNAP_4,
	LC_EDIT_MOVEZ_SNAP_5,
	LC_EDIT_MOVEZ_SNAP_6,
	LC_EDIT_MOVEZ_SNAP_7,
	LC_EDIT_MOVEZ_SNAP_8,
	LC_EDIT_MOVEZ_SNAP_9,
	LC_EDIT_ANGLE_SNAP_0,
	LC_EDIT_ANGLE_SNAP_1,
	LC_EDIT_ANGLE_SNAP_2,
	LC_EDIT_ANGLE_SNAP_3,
	LC_EDIT_ANGLE_SNAP_4,
	LC_EDIT_ANGLE_SNAP_5,
	LC_EDIT_ANGLE_SNAP_6,
	LC_EDIT_ANGLE_SNAP_7,
	LC_EDIT_ANGLE_SNAP_8,
	LC_EDIT_ANGLE_SNAP_9,
	LC_EDIT_ACTION_SELECT,
	LC_EDIT_ACTION_INSERT,
	LC_EDIT_ACTION_LIGHT,
	LC_EDIT_ACTION_SPOTLIGHT,
	LC_EDIT_ACTION_CAMERA,
	LC_EDIT_ACTION_MOVE,
	LC_EDIT_ACTION_ROTATE,
	LC_EDIT_ACTION_ERASER,
	LC_EDIT_ACTION_PAINT,
	LC_EDIT_ACTION_ZOOM,
	LC_EDIT_ACTION_ZOOM_REGION,
	LC_EDIT_ACTION_PAN,
	LC_EDIT_ACTION_ROTATE_VIEW,
	LC_EDIT_ACTION_ROLL,
};

enum LC_ACTIONS
{
	LC_ACTION_SELECT, 
	LC_ACTION_INSERT,
	LC_ACTION_LIGHT,
	LC_ACTION_SPOTLIGHT,
	LC_ACTION_CAMERA,
	LC_ACTION_MOVE,
	LC_ACTION_ROTATE,
	LC_ACTION_ERASER,
	LC_ACTION_PAINT,
	LC_ACTION_ZOOM,
	LC_ACTION_ZOOM_REGION,
	LC_ACTION_PAN,
	LC_ACTION_ROTATE_VIEW,
	LC_ACTION_ROLL,
	LC_ACTION_CURVE
};

// Select by Name dialog data
enum LC_SEL_DATA_TYPE
{
	LC_SELDLG_PIECE,
	LC_SELDLG_CAMERA,
	LC_SELDLG_LIGHT,
	LC_SELDLG_GROUP
};

struct LC_SEL_DATA
{
	const char* name;
	unsigned char type;
	bool selected;
	void* pointer;
};

struct LC_PIECE_MODIFY
{
	Piece* piece;
	lcVector3 Position;
	lcVector3 Rotation;
	char name[81];
	int from;
	int to;
	bool hidden;
	int color;
};

struct LC_CAMERA_MODIFY
{
	Camera* camera;
	lcVector3 Eye;
	lcVector3 Target;
	lcVector3 Up;
	char name[81];
	float fovy;
	float znear;
	float zfar;
	bool hidden;
};

// Image

enum LC_IMAGE_FORMATS
{
	LC_IMAGE_BMP,
	LC_IMAGE_GIF,
	LC_IMAGE_JPG,
	LC_IMAGE_PNG,
	LC_IMAGE_AVI
};

struct LC_IMAGE_OPTS
{
	unsigned char quality;
	bool interlaced;
	bool transparent;
	bool truecolor;
	unsigned char background[3];
	float pause;
	unsigned int format;
};

struct LC_IMAGEDLG_OPTS
{
	char filename[LC_MAXPATH];
	unsigned short from;
	unsigned short to;
	bool multiple;
	unsigned short width;
	unsigned short height;
	LC_IMAGE_OPTS imopts;
};

enum LC_DIALOGS
{
	LC_DLG_FILE_OPEN_PROJECT,
	LC_DLG_FILE_SAVE_PROJECT,
	LC_DLG_FILE_MERGE_PROJECT,
	LC_DLG_FILE_OPEN,
	LC_DLG_FILE_SAVE,
	LC_DLG_DIRECTORY_BROWSE,
	LC_DLG_PICTURE_SAVE,
	LC_DLG_BRICKLINK,
	LC_DLG_HTML,
	LC_DLG_POVRAY,
	LC_DLG_WAVEFRONT,
	LC_DLG_MINIFIG,
	LC_DLG_ARRAY,
	LC_DLG_PREFERENCES,
	LC_DLG_PROPERTIES,
	LC_DLG_TERRAIN,
	LC_DLG_LIBRARY,
	LC_DLG_SELECTBYNAME,
	LC_DLG_STEPCHOOSE,
	LC_DLG_EDITGROUPS,
	LC_DLG_GROUP,
	LC_DLG_EDITCATEGORY,
	LC_DLG_ABOUT
};

enum LC_FILEOPENDLG_TYPES
{
	LC_FILEOPENDLG_LCF
};

struct LC_FILEOPENDLG_OPTS
{
	int type;
	char path[LC_MAXPATH];
	int numfiles;
	char** filenames;
};

enum LC_FILESAVEDLG_TYPES
{
	LC_FILESAVEDLG_LCF,
};

struct LC_FILESAVEDLG_OPTS
{
	int type;
	char path[LC_MAXPATH];
};

struct LC_DLG_DIRECTORY_BROWSE_OPTS
{
	const char* Title;
	char Path[LC_MAXPATH];
};

struct LC_POVRAYDLG_OPTS
{
	bool render;
	char povpath[LC_MAXPATH];
	char outpath[LC_MAXPATH];
	char libpath[LC_MAXPATH];
};

struct LC_HTMLDLG_OPTS
{
	char path[LC_MAXPATH];
	bool singlepage;
	bool index;
	bool images;
	bool id;
	int color;
	bool listend;
	bool liststep;
	bool highlight;
	LC_IMAGEDLG_OPTS imdlg;
};

struct LC_ARRAYDLG_OPTS
{
	unsigned short n1DCount;
	unsigned short n2DCount;
	unsigned short n3DCount;
	unsigned char nArrayDimension;
	float f2D[3];
	float f3D[3];
	float fMove[3];
	float fRotate[3];
};

struct LC_PROPERTIESDLG_OPTS
{
	char strAuthor[101];
	char strDescription[101];
	char strComments[256];
	char* strTitle;
	char* strFilename;
	const char** PieceNames;
	int NumPieces;
	int* PieceColorCount;
	int NumColors;
};

struct LC_GROUPEDITDLG_OPTS
{
	int piececount;
	Piece** pieces;
	Group** piecesgroups;
	int groupcount;
	Group** groups;
	Group** groupsgroups;
};

struct LC_PREFERENCESDLG_OPTS
{
	int nMouse;
	int nSaveInterval;
	char strUser[101];
	char strPath[LC_MAXPATH];
	unsigned long nDetail;
	float fLineWidth;
	int AASamples;
	unsigned long nSnap;
	unsigned short nAngleSnap;
	unsigned short nGridSize;
	unsigned long nScene;
	float fDensity;
	char strBackground[LC_MAXPATH];
	float fBackground[4];
	float fFog[4];
	float fAmbient[4];
	float fGrad1[3];
	float fGrad2[3];
	char strFooter[256];
	char strHeader[256];
};

struct LC_CATEGORYDLG_OPTS
{
	String Name;
	String Keywords;
};

enum LC_TRANSFORM_TYPE
{
	LC_TRANSFORM_ABSOLUTE_TRANSLATION,
	LC_TRANSFORM_RELATIVE_TRANSLATION,
	LC_TRANSFORM_ABSOLUTE_ROTATION,
	LC_TRANSFORM_RELATIVE_ROTATION,
};

#endif
