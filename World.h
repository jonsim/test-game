#ifndef WORLD_H
#define WORLD_H


//---------- INCLUDES ----------//
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>



//---------- DEFINITIONS ----------//
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


class World
{
public:
    World (Ogre::SceneManager* sceneManager, Ogre::Light* sun);
    virtual ~World (void);

    Ogre::Vector3 getTerrainNormal (Ogre::Vector3 worldPosition);
    float         getTerrainHeight (Ogre::Vector3 worldPosition);
    bool terrainRaycast (Ogre::Ray& ray, Ogre::Vector3* resultPosition);
    void saveTerrainState (void);

private:
    void defineTerrain (long x, long y);
    void initBlendMaps (Ogre::Terrain* terrain);
    void configureTerrainDefaults (Ogre::SceneManager* sceneManager, Ogre::Light* sun);
    void setupWorld (void);
    
    Ogre::TerrainGroup* mTerrainGroup;
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
//    OgreBites::Label* mInfoLabel;
    bool mTerrainsImported;
};

#endif // #ifndef WORLD_H
