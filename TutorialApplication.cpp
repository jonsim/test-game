//---------- INCLUDES ----------//

#include "TutorialApplication.h"
#include <vector>



//---------- INCLUDES ----------//

/// @brief  Constructor
TutorialApplication::TutorialApplication (void)
{
}



/// @brief  Deconstructor
TutorialApplication::~TutorialApplication (void)
{
}



void TutorialApplication::createFrameListener (void)
{
    BaseApplication::createFrameListener();
 
    mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);
}



void TutorialApplication::createScene (void)
{
    // Setup CEGUI
    setupGUI();


    // Setup texturing
    Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);
    

    // Setup lighting
    Ogre::Vector3 lightdir(0.55f, -0.3f, 0.75f);
    lightdir.normalise();
    
    Ogre::Light* light = mSceneMgr->createLight("tstLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(lightdir);
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
    
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.2f));
    

    // Setup terrain
    mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();
    
    mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);
    mTerrainGroup->setFilenameConvention(Ogre::String("BasicTutorial3Terrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
    
    configureTerrainDefaults(light);
    
    for (long x = 0; x <= 0; ++x)
        for (long y = 0; y <= 0; ++y)
            defineTerrain(x, y);
 
    // sync load since we want everything in place when we start
    mTerrainGroup->loadAllTerrains(true);
 
    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
        while(ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }
 
    mTerrainGroup->freeTemporaryResources();

    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8, 500);


    // Add entities
    // Add player
    Ogre::Entity* playerEntity = mSceneMgr->createEntity("PlayerCharacter", "potato.mesh");
    Ogre::AxisAlignedBox bb = playerEntity->getBoundingBox();
    mPlayerHeight = (bb.getMaximum().y - bb.getMinimum().y) * 0.1f * 0.5f;
    mCharacterNode->attachObject(playerEntity);
    mCharacterNode->setScale(0.1f, 0.1f, 0.1f);
    mCharacterNode->setPosition(0, 0, 0);

    // Add target marker
    mTargetNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("TargetNode");
    Ogre::Entity* targetEntity = mSceneMgr->createEntity("TargetEntity", "sphere.mesh");
    mTargetNode->attachObject(targetEntity);
    targetEntity = mSceneMgr->createEntity("TargetEntity2", "column.mesh");
    mTargetNode->attachObject(targetEntity);
    mTargetNode->setScale(0.1f, 0.1f, 0.1f);
    mTargetNode->setPosition(0, 0, 0);


    // Billboard set
    mTargetBillboardSet = mSceneMgr->createBillboardSet("TargetBillboardSet", 2);
    mSceneMgr->getRootSceneNode()->attachObject(mTargetBillboardSet);
    mTargetBillboardSet->setMaterialName("Examples/Flare");
    mTargetBillboardSet->setVisible(true);
    mTargetBillboardSet->createBillboard(0, 300, 0, Ogre::ColourValue(0.5f, 0.6f, 1.0f));
    mTargetBillboardSet->setDefaultDimensions(100, 100);
    mTargetBillboardSet->setBillboardType(Ogre::BBT_ORIENTED_COMMON);
    mTargetBillboardSet->setCommonUpVector(Ogre::Vector3(0, 1, 0));
}



void TutorialApplication::configureTerrainDefaults (Ogre::Light* light)
{
    // Configure global
    mTerrainGlobals->setMaxPixelError(8);
    // testing composite map
    mTerrainGlobals->setCompositeMapDistance(3000);
 
    // Important to set these so that the terrain knows what to use for derived (non-realtime) data
    mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
    mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
 
    // Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = 513;
    defaultimp.worldSize = 12000.0f;
    defaultimp.inputScale = 600;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;
    // textures
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



void TutorialApplication::defineTerrain (long x, long y)
{
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);
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



void TutorialApplication::initBlendMaps (Ogre::Terrain* terrain)
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



void TutorialApplication::destroyScene (void)
{
    OGRE_DELETE mTerrainGroup;
    OGRE_DELETE mTerrainGlobals;
}



Ogre::Vector3 TutorialApplication::getTerrainNormalAtWorldPosition (Ogre::Vector3 pos)
{
    long tx = 0, ty = 0;
    Ogre::Terrain* t = NULL;
    
    // Get the terrain slot from world coords.
    mTerrainGroup->convertWorldPositionToTerrainSlot(pos, &tx, &ty);
    t = mTerrainGroup->getTerrain(tx, ty);
    if (t == NULL)
        return Ogre::Vector3::ZERO;

    // Get the terrain coords.
    Ogre::Vector3 terrainCoords;
    t->getTerrainPosition(pos, &terrainCoords);

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



bool TutorialApplication::frameRenderingQueued (const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);

    Ogre::Vector3 pp = mPlayerNode->getPosition();
    pp.y = mTerrainGroup->getHeightAtWorldPosition(pp.x, pp.y, pp.z) + mPlayerHeight;

    if (mTargetCursorVisible)
    {
        OIS::MouseState ms = mMouse->getMouseState();
        Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(ms.X.abs / ((float) ms.width), ms.Y.abs / ((float) ms.height));
        
        Ogre::TerrainGroup::RayResult terrainHit = mTerrainGroup->rayIntersects(mouseRay);
        if (terrainHit.hit)
        {
            Ogre::Vector3 n = getTerrainNormalAtWorldPosition(terrainHit.position);

            mTargetNode->setPosition(terrainHit.position);
            //mTargetNode->setDirection(Ogre::Vector3(1, 1, 0), Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Y);
            mTargetNode->setDirection(n, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Y);
            //mTargetBillboardSet->setCommonUpVector(Ogre::Vector3(0, 1, 0));
            //mTargetBillboardSet->setCommonUpVector(n);
            //mTargetBillboardSet->getBillboard(0)->setPosition(terrainHit.position + Ogre::Vector3(0, 5, 0));
            mTargetNode->setVisible(true);

            // Add the arc
            // Project the vectors down to 2 dimensions.
            //Ogre::Vector3 playerVector_2D, targetVector_2D;
            //playerVector_2D = mPlayerNode->getPosition();
            //playerVector_2D.y = 0;
            //targetVector_2D = mTargetNode->getPosition();
            //targetVector_2D.y = 0;

            
            // Get the direction of the target relative to the player, projecting it down into 2D to get the planar
            // distance (XZ plane) of the target (elevation is accounted for when solving the equations to produce 
            // the curve by further projection to the XY plane and subsequent rotation). Failing to project to XZ
            // here will cause erroneous distances to be calculated later on.
            Ogre::Vector3 targetVector_pc = mPlayerNode->convertWorldToLocalPosition(mTargetNode->getPosition());
            targetVector_pc.y = 0;
            float        targetDistance = targetVector_pc.length();
            Ogre::Radian targetAngle    = targetVector_pc.angleBetween(Ogre::Vector3::NEGATIVE_UNIT_Z);
            if (targetVector_pc.x > 0)
                targetAngle *= -1;

            mPathNode->setOrientation(Ogre::Quaternion(targetAngle, Ogre::Vector3::UNIT_Y));

            drawTargetPath(targetDistance, 100);
            
            //char stri[128];
            //sprintf(stri, "targetDistance = %.1f\n", targetDistance);
            //OutputDebugString(stri);
        }
        else
        {
            mTargetNode->setVisible(false);
        }
    }
    else
    {
        //mTargetNode->setVisible(false);
    }

    float forward = 0;
    if (mKeyboard->isKeyDown(OIS::KC_W))
        forward -= 200 * evt.timeSinceLastFrame;
    if (mKeyboard->isKeyDown(OIS::KC_S))
        forward += 200 * evt.timeSinceLastFrame;

    float right = 0;
    if (mKeyboard->isKeyDown(OIS::KC_Q))
        right += 200 * evt.timeSinceLastFrame;
    if (mKeyboard->isKeyDown(OIS::KC_E))
        right -= 200 * evt.timeSinceLastFrame;

    float clockwise = 0;
    if (mKeyboard->isKeyDown(OIS::KC_A))
        clockwise += 100 * evt.timeSinceLastFrame;
    if (mKeyboard->isKeyDown(OIS::KC_D))
        clockwise -= 100 * evt.timeSinceLastFrame;

    mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(clockwise)));

    Ogre::Radian rot = mPlayerNode->getOrientation().getYaw(true);
    float zComponent = Ogre::Math::Cos(rot);
    float xComponent = Ogre::Math::Sin(rot);
    
    mPlayerNode->setPosition(pp.x + (xComponent * forward) + (xComponent * right), pp.y, pp.z + (zComponent * forward) + (zComponent * right));

    
    if (mTerrainGroup->isDerivedDataUpdateInProgress())
    {
        mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
        mInfoLabel->show();
        if (mTerrainsImported)
        {
            mInfoLabel->setCaption("Building terrain, please wait...");
        }
        else
        {
            mInfoLabel->setCaption("Updating textures, patience...");
        }
    }
    else
    {
        mTrayMgr->removeWidgetFromTray(mInfoLabel);
        mInfoLabel->hide();
        if (mTerrainsImported)
        {
            mTerrainGroup->saveAllTerrains(true);
            mTerrainsImported = false;
        }
    }
 
    return ret;
}



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain (HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
    int main (int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;
 
        try
        {
            app.go();
        } catch( Ogre::Exception& e )
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif