#include <sstream>
#include "DxGraphic.h"
#include "TextureCollection.h"
#include "BoundaryBox.h"
#include "SpacePartitioning.h"
#include "ImmortalObj.h"
#include "TileMap.h"
#include "Scoreboard.h"
#include "GameWorld.h"

GameWorld* GameWorld::instance = NULL;

bool GameWorld::loadResource()
{
	if (!loadAllTextures()) return false;

	if (!scorebar->loadResource()) return false;
	
	return true;
}

GameWorld * GameWorld::getInstance()
{
	if (instance == NULL)
		instance = new GameWorld();
	return instance;
}

GameWorld::~GameWorld()
{
	TextureCollection* textures = TextureCollection::getInstance();
	delete tileMap;
	delete scorebar;
	delete textures;
}

bool GameWorld::initialize()
{
	scorebar = Scoreboard::getInstance();
	tileMap = new TileMap();

	if (!loadResource()) return false;
	
	if (!newGame()) return false;

	return true;
}

void GameWorld::update(float dtTime)
{
	updateInProcObjsList();
	scorebar->update(dtTime);

	camera.x += 1;
}

void GameWorld::render()
{
	DxGraphic* dxGraphic = DxGraphic::getInstance();
	if (dxGraphic->direct3dDevice->BeginScene())
	{
		//Clear backbuffer with a color
		dxGraphic->direct3dDevice->ColorFill(
			dxGraphic->backBuffer, NULL, Color(0,0,0));

		dxGraphic->spriteHandler->Begin(D3DXSPRITE_ALPHABLEND);
		// Call render function of gameObjects in Game World
		tileMap->render(camera);
		scorebar->render();

		//
		dxGraphic->spriteHandler->End();
		dxGraphic->direct3dDevice->EndScene();
	}
	dxGraphic->direct3dDevice->Present(NULL, NULL, NULL, NULL);
}

bool GameWorld::newGame()
{
	if (!tileMap->loadStage(Stage::_3_1))
		return false;
	
	spacePart.loadFromFile(STAGE_3_1_GRID_FILE, MAP_POS);
	if(!loadGameObjs(Stage::_3_1))
		return false;

	scorebar->mainHealth = MAX_HEALTH;
	scorebar->enemyHealth = MAX_HEALTH;
	scorebar->timer = 0.f;
	scorebar->score = 0;
	scorebar->items = 0;
	scorebar->players = MAX_PLAYERS;
	scorebar->stage = Stage::_3_1;

	return true;
}

bool GameWorld::loadGameObjs(Stage stage)
{
	for (auto iterator : gameObjects)
		delete iterator.second;
	gameObjects.clear();
	inProcObjs.clear();

	STRINGS objsInfoLines;
	switch (stage)
	{
	case _3_1:
		objsInfoLines = readFileText(STAGE_3_1_OBJS_FILE);
		break;
	case _3_2:
		objsInfoLines = readFileText(STAGE_3_2_OBJS_FILE);
		break;
	case _3_3:
		objsInfoLines = readFileText(STAGE_3_3_OBJS_FILE);
		break;
	}
	if (objsInfoLines.size() <= 1)
		return false;
	for (unsigned int i = 1; i < objsInfoLines.size(); i++)
	{
		int id, left_bmp, bottom_bmp, width, height;
		stringstream iss(objsInfoLines[i]);
		iss >> id; iss >> left_bmp; iss >> bottom_bmp; iss >> width; iss >> height;

		Vector2 LeftBot_wP = transformViewToWorld(Vector2(left_bmp, bottom_bmp), MAP_POS);

		LPGAMEOBJ obj = NULL;
		switch (id / OBJ_KIND_WEIGHT)
		{
		case ObjKind::Ground:
		case ObjKind::Stair:
		case ObjKind::Wall:
		case ObjKind::Gate:
		case ObjKind::Water:
			obj = new ImmortalObj(LeftBot_wP, Vector2(width, height));
			this->gameObjects[id] = obj;
			break;
		default:
			break;
		}
	}
	return true;
}