//---------- INCLUDES ----------//

#include "World.h"



//---------- FUNCTIONS ----------//

/// @brief  Constructor, setting up the terrain
World::World (Ogre::SceneManager* sceneManager, Ogre::Light* sun)
{
    // Create the terrain.
    mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();
    mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(sceneManager, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);

    // Configure the terrain.
    mTerrainGroup->setFilenameConvention(Ogre::String("BasicTutorial3Terrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
    configureTerrainDefaults(sceneManager, sun);
    
    // Define the terrain
    for (long x = 0; x <= 0; ++x)
        for (long y = 0; y <= 0; ++y)
            defineTerrain(x, y);
    
    // Load the terrain, synchronising it so that everything is in place at the end.
    mTerrainGroup->loadAllTerrains(true);
    
    // Load the textures.
    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
        while (ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }
    
    // Finish up the initialisation.
    mTerrainGroup->freeTemporaryResources();

    // Load the skybox.
    sceneManager->setSkyDome(true, "Examples/CloudySky", 5, 8, 500);
}



/// @brief  Deconstructor
World::~World (void)
{
    OGRE_DELETE mTerrainGroup;
    OGRE_DELETE mTerrainGlobals;
}



void World::configureTerrainDefaults (Ogre::SceneManager* sceneManager, Ogre::Light* sun)
{
    // Configure global
    mTerrainGlobals->setMaxPixelError(8);

    // testing composite map
    mTerrainGlobals->setCompositeMapDistance(3000);
 
    // Important to set these so that the terrain knows what to use for derived (non-realtime) data
    mTerrainGlobals->setLightMapDirection(sun->getDerivedDirection());
    mTerrainGlobals->setCompositeMapAmbient(sceneManager->getAmbientLight());
    mTerrainGlobals->setCompositeMapDiffuse(sun->getDiffuseColour());
 
    // Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = 513;
    defaultimp.worldSize = 12000.0f;
    defaultimp.inputScale = 600;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;

    // Initialise the texture stack.
    defaultimp.layerList.resize(3);
    defaultimp.layerList[0].worldSize = 100;
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
    defaultimp.layerList[1].worldSize = 30;
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
    defaultimp.layerList[2].worldSize = 200;
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
}



void getTerrainImage (bool flipX, bool flipY, Ogre::Image& img)
{
    img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (flipX)
        img.flipAroundY();
    if (flipY)
        img.flipAroundX();
}



void World::defineTerrain (long x, long y)
{
    // Get this page's filename.
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);

    // Define the terrain, flipping the terrain image so that it is seamless.
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
    {
        mTerrainGroup->defineTerrain(x, y);
    }
    else
    {
        Ogre::Image img;
        getTerrainImage(x % 2 != 0, y % 2 != 0, img);
        mTerrainGroup->defineTerrain(x, y, &img);
        mTerrainsImported = true;
    }
}



void World::initBlendMaps (Ogre::Terrain* terrain)
{
    Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::Real minHeight1 = 70;
    Ogre::Real fadeDist1 = 15;
    float* pBlend0 = blendMap0->getBlendPointer();
    float* pBlend1 = blendMap1->getBlendPointer();
    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;
 
            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val;
 
            val = (height - minHeight1) / fadeDist1;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend1++ = val;
        }
    }
    blendMap0->dirty();
    blendMap1->dirty();
    blendMap0->update();
    blendMap1->update();
}



Ogre::Vector3 World::getTerrainNormal (Ogre::Vector3 worldPosition)
{
    long tx = 0, ty = 0;
    Ogre::Terrain* t = NULL;
    
    // Get the terrain slot from world coords.
    mTerrainGroup->convertWorldPositionToTerrainSlot(worldPosition, &tx, &ty);
    t = mTerrainGroup->getTerrain(tx, ty);
    if (t == NULL)
        return Ogre::Vector3::ZERO;

    // Get the terrain coords.
    Ogre::Vector3 terrainCoords;
    t->getTerrainPosition(worldPosition, &terrainCoords);

    // Get the left/bottom points.
    float factor  = t->getSize() - 1.0f;
    float ifactor = 1.0f / factor;
    long  startX  = static_cast<long>(terrainCoords.x * factor);
    long  startY  = static_cast<long>(terrainCoords.y * factor);
    long  endX    = startX + 1;
    long  endY    = startY + 1;

    // Get the points in terrain space (rounding (not clamping) to boundaries).
    float t_startX = startX * ifactor;
    float t_startY = startY * ifactor;
    float t_endX   = endX   * ifactor;
    float t_endY   = endY   * ifactor;

    // Clamp.
    endX = MIN(endX, (long) t->getSize() - 1);
    endY = MIN(endY, (long) t->getSize() - 1);

    // Get parametric coords.
    float paramX = (terrainCoords.x - t_startX) / ifactor;
    float paramY = (terrainCoords.y - t_startY) / ifactor;

    /* For even / odd tri strip rows, triangles are this shape:
       even     odd
       3---2   3---2
       | / |   | \ |
       0---1   0---1
    */
    // Build all 4 positions in terrain space, using point-sampled height
    // - note:  I've modified this code from the original to return verticies in world space, which is required
    //   to get a proper terrain normal below
    Ogre::Vector3 v0 (t_startX * t->getWorldSize(), t->getWorldSize() - t_startY * t->getWorldSize(), t->getHeightAtPoint(startX, startY));
    Ogre::Vector3 v1 (t_endX   * t->getWorldSize(), t->getWorldSize() - t_startY * t->getWorldSize(), t->getHeightAtPoint(endX,   startY));
    Ogre::Vector3 v2 (t_endX   * t->getWorldSize(), t->getWorldSize() - t_endY   * t->getWorldSize(), t->getHeightAtPoint(endX,   endY));
    Ogre::Vector3 v3 (t_startX * t->getWorldSize(), t->getWorldSize() - t_endY   * t->getWorldSize(), t->getHeightAtPoint(startX, endY));

    // Define the plane in world space, paying attention to the differing tessilation of even/odd rows
    Ogre::Plane plane;
    if (startY % 2)
    {
       // odd row
       bool secondTri = ((1.0 - paramY) > paramX);
       if (secondTri)
          plane.redefine(v0, v1, v3);
       else
          plane.redefine(v1, v2, v3);
    }
    else
    {
       // even row
       bool secondTri = (paramY > paramX);
       if (secondTri)
          plane.redefine(v0, v2, v3);
       else
          plane.redefine(v0, v1, v2);
    }

    // All the above calculations are based on the terrain being in the x-y plane.
    // convert normal to terrain being in the x-z plane.
    Ogre::Vector3 r(-plane.normal.x, -plane.normal.z, -plane.normal.y);
    r.normalise();
    return r;
}



float World::getTerrainHeight (Ogre::Vector3 worldPosition)
{
    return mTerrainGroup->getHeightAtWorldPosition(worldPosition);
}


bool World::terrainRaycast (Ogre::Ray& ray, Ogre::Vector3* resultPosition)
{
    // Ray cast to the terrain
    Ogre::TerrainGroup::RayResult terrainHit = mTerrainGroup->rayIntersects(ray);

    if (terrainHit.hit)
    {
        *resultPosition = terrainHit.position;
        return true;
    }
    return false;
}


void World::saveTerrainState (void)
{
    // Check terrain is not currently updating (textures/lighting) and the terrains have 
    // been fully imported and not saved.
    if (!mTerrainGroup->isDerivedDataUpdateInProgress() && mTerrainsImported)
    {
        mTerrainGroup->saveAllTerrains(true);
        mTerrainsImported = false;
    }
}