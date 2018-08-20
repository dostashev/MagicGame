#include "World.h"
#include <iostream>
World::World()
{
	init();
}

inline bool World::init() {
	textureScale = 4;
	dynamicGrid.textureScale = 4;
	staticGrid.textureScale = 4;
	projectiles.textureScale = 4;
	bool result = initSpriteMap();
	initPlayer();
	generate(3000, 3000);
	return result;
}

inline bool World::initSpriteMap()
{
	std::ifstream ifs("textures.txt");
	if (!ifs.is_open())
		return 0;
	std::string textureName, textureFile;
	int X1, Y1, X2, Y2;
	char firstSymbol;
	while (ifs >> firstSymbol) {
		if (firstSymbol == '#') {
			std::string temp;
			getline(ifs, temp);
			continue;
		}
		ifs >> textureName >> textureFile >> X1 >> Y1 >> X2 >> Y2;
		textureName = firstSymbol + textureName;
		//printf("==\n%s\n%s\n%d %d %d %d\n", textureName.c_str(), textureFile.c_str(), X1, Y1, X2, Y2);
		sf::Texture * texture = new sf::Texture;
		if (!texture->loadFromFile(textureFile, sf::IntRect(X1, Y1, X2, Y2)))
			return 0;
		spriteMap[textureName] = texture;
	}
	ifs.close();
}

void World::draw(sf::RenderTarget & target, float drawTime)
{
	std::vector<DynamicObject*> dynamicObjects = dynamicGrid.getObjects();
	std::vector<Object*> visibleObjects;
	for (auto obj : dynamicObjects)visibleObjects.push_back((Object*)obj);
	std::vector<DynamicSpell*> spells = projectiles.getObjects();
	for (auto obj : spells)visibleObjects.push_back((Object*)obj);
	std::vector<Object*> staticObjects = staticGrid.getObjects();

	visibleObjects.insert(visibleObjects.end(), staticObjects.begin(), staticObjects.end());
	std::sort(visibleObjects.begin(), visibleObjects.end(), cmpImgDraw);
	for (auto obj : visibleObjects) {
		sf::Sprite sprite;
		sprite.setScale(sf::Vector2f(textureScale, textureScale));
		sprite.setPosition(obj->position);
		sprite.setTexture(*(spriteMap[obj->getSpriteName(drawTime)]));
		target.draw(sprite);
	}
}

void World::updateObjects(float interactTime)
{
	std::vector<DynamicObject*> allObjects = dynamicGrid.getObjects();
	for (auto obj : allObjects) {
		obj->updateState(interactTime);
	}
	std::vector<DynamicSpell*> spells = projectiles.getObjects();
	for (auto obj : spells) {
		obj->updateState(interactTime);
		if (!obj->getRectangle().intersects(worldRect))projectiles.removeObject(obj);
	}
	interact(interactTime);
}

void World::interact(float interactTime)
{
	for (auto spell : projectiles.getObjects()) {
		if (spell->getCollisionBox().intersects(player->getCollisionBox())) {
			spell->interactWithPlayer(player, interactTime);
			if (!spell->isPiercing)projectiles.removeObject(spell);
		}
	}
	std::cout << player->getHp() << std::endl;
}

inline void World::generate(int xSize, int ySize)
{
	sf::Vector2f grassSize(spriteMap["grass"]->getSize());
	for (float i = 0; i < xSize; i += grassSize.x*textureScale)
	{
		for (float j = 0; j < ySize; j += grassSize.y*textureScale) {
			initGrass(sf::Vector2f(i, j));
		}
	}
	//initHouse(sf::Vector2f(500, 300));
}

inline void World::initPlayer()
{
	player = new Player();
	dynamicGrid.addObject(player);
	player->AssignTexture((sf::Vector2f)(spriteMap[player->getSpriteName(0)]->getSize()));
	player->world = this;
}

inline void World::initGrass(sf::Vector2f position)
{
	Grass *grass = new Grass();
	staticGrid.addObject(grass);
	grass->position = position;
	grass->AssignTexture((sf::Vector2f)(spriteMap[grass->getSpriteName(0)]->getSize()));
}

void World::initArcaneBolt(sf::Vector2f start, sf::Vector2f finish, sf::Vector2f direction, bool isHorizontal)
{
	ArcaneBolt *bolt = new ArcaneBolt(finish - start);
	projectiles.addObject(bolt);
	bolt->AssignTexture((sf::Vector2f)(spriteMap[bolt->getSpriteName(0)]->getSize()));
	//bolt->position = start - bolt->getTextureSize()*0.5f;
	bolt->position = start - sf::Vector2f(bolt->collisionBox.left, bolt->collisionBox.top) + sf::Vector2f(bolt->collisionBox.width*direction.x, bolt->collisionBox.height*direction.y);
	if (isHorizontal)bolt->position -= sf::Vector2f(0, bolt->collisionBox.height*0.5f);
	else bolt->position -= sf::Vector2f(bolt->collisionBox.width*0.5f, 0);
	//std::cout << (bolt->getSpeed()) << std::endl;
}