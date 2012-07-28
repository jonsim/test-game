//---------- INCLUDES ----------//

#include "SharedIncludes.h"
#include <vector>



//---------- INCLUDES ----------//

/// @brief  Constructor
GraphicsCore::GraphicsCore (void) : mRoot(0),
                                mCamera(0),
                                mSceneMgr(0),
                                mWindow(0),
                                mResourcesCfg(Ogre::StringUtil::BLANK),
                                mPluginsCfg(Ogre::StringUtil::BLANK),
                                mTrayMgr(0),
                                mDetailsPanel(0),
                                mPlayerAimHeight(100),
                                mCursorWasVisible(false),
                                mShutDown(false)
{
}



/// @brief  Deconstructor
GraphicsCore::~GraphicsCore (void)
{
    if (mTrayMgr)
        delete mTrayMgr;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}



void GraphicsCore::go (void)
{
    // Select and load the relevant resources
    mResourcesCfg = "../../media/resources.cfg";
#ifdef _DEBUG
    mPluginsCfg = "plugins_d.cfg";
#else
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}




bool GraphicsCore::setup (void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
}






void GraphicsCore::setupGUI (void)
{
    static bool guiSetup = false;
    if (!guiSetup)
    {
        // Attach and start the CEGUI renderer.
        mGUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem(*mWindow);

        // Initialise the CEGUI renderer
	    // Load the fonts and set their sizes.
        CEGUI::Font* pFont;
	    CEGUI::Font::setDefaultResourceGroup("Fonts");
	    pFont = &CEGUI::FontManager::getSingleton().create("DejaVuSans-10.font");
	    pFont->setProperty( "PointSize", "10" );
	    //pFont = &CEGUI::FontManager::getSingleton().create("Verdana-outline-10.font");
	    //pFont->setProperty( "PointSize", "10" );
	    // Register font as default
	    CEGUI::System::getSingleton().setDefaultFont("DejaVuSans-10");
    
	    // Create skin scheme outlining widget (window) parameters.
	    CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	    CEGUI::SchemeManager::getSingleton().create("VanillaSkin.scheme");
	    CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
	    CEGUI::SchemeManager::getSingleton().create("GWEN.scheme");
	    CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
    
	    // Register skin's default image set and cursor icon
	    CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	    CEGUI::System::getSingleton().setDefaultMouseCursor("Vanilla-Images", "MouseArrow");
        // Move CEGUI mouse to (0,0)                                        
        CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();  
        CEGUI::System::getSingleton().injectMouseMove(-mousePos.d_x,-mousePos.d_y);

	    // Tell CEGUI where to look for layouts
	    CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	    // Create an empty default window layer
	    mGUIWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "root_wnd");
	    CEGUI::System::getSingleton().setGUISheet(mGUIWindow);

        // Prevent the GUI from being initialised again
        guiSetup = true;
    }
}


void GraphicsCore::createFrameListener (void)
{
    // Start the input
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mUserInput = new Input(pl, this);



    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    //mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    //mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    //mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    //mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");

//    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    //mDetailsPanel->setParamValue(9, "Bilinear");
    //mDetailsPanel->setParamValue(10, "Solid");
    //mDetailsPanel->hide();

    mRoot->addFrameListener(this);



//    mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);
}



void GraphicsCore::setupResources (void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}



bool GraphicsCore::configure (void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if (mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

        return true;
    }
    else
    {
        return false;
    }
}



void GraphicsCore::chooseSceneManager (void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}



void GraphicsCore::createCamera (void)
{
    // Create the camera and set it up.
    mCamera = mSceneMgr->createCamera("PlayerCam");
    mCamera->setPosition(Ogre::Vector3(0, 0, 0));
    mCamera->lookAt(0, -100, -300);
    mCamera->setNearClipDistance(5);
    // Enable infinite clip distance if possible
    if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
        mCamera->setFarClipDistance(0);
    else
        mCamera->setFarClipDistance(50000);

    // Setup the nodes, this isn't really the right place to setup the player node but it will do for now.
    mPlayerNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerNode");
    mCameraNode    = mPlayerNode->createChildSceneNode("CameraNode");
    mCharacterNode = mPlayerNode->createChildSceneNode("CharacterNode");
    mPathNode      = mPlayerNode->createChildSceneNode("PathNode");
    mCameraNode->attachObject(mCamera);
    mCameraNode->setPosition(0, 100, 200);
    mPathNode->setVisible(false);
    setupTargetPath();
}



void GraphicsCore::createViewports (void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    // Set the background colour and match the aspect ratio to the window's.
    vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}



void GraphicsCore::createResourceListener (void)
{

}



void GraphicsCore::loadResources (void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}



void GraphicsCore::createScene (void)
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



void GraphicsCore::configureTerrainDefaults (Ogre::Light* light)
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



void GraphicsCore::defineTerrain (long x, long y)
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



void GraphicsCore::initBlendMaps (Ogre::Terrain* terrain)
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



void GraphicsCore::destroyScene (void)
{
    OGRE_DELETE mTerrainGroup;
    OGRE_DELETE mTerrainGlobals;
}







static unsigned int lineCount = 0;
Ogre::ManualObject* GraphicsCore::drawLine (Ogre::SceneNode* sn, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& col)
{
    char lineName[32];
    char nodeName[32];
    sprintf(lineName, "line_%d",     lineCount);
    sprintf(nodeName, "linenode_%d", lineCount++);

    Ogre::ManualObject* mo = mSceneMgr->createManualObject(lineName);

    mo->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    mo->colour(col);
    mo->position(start);
    mo->position(end);
    mo->end();
    sn->createChildSceneNode(nodeName)->attachObject(mo);

    return mo;
}



void GraphicsCore::setupTargetPath (void)
{
    // Add billboard chain
    mPathChain = mSceneMgr->createBillboardChain("arc");
    mPathChain->setNumberOfChains(1);
    mPathChain->setMaxChainElements(ARC_RESOLUTION);
    mPathChain->setTextureCoordDirection(Ogre::BillboardChain::TCD_V);
    mPathChain->setMaterialName("Examples/LightRibbonTrail");
    mPathNode->attachObject(mPathChain);
    mPathChain->setVisible(true);
}



void GraphicsCore::drawTargetPath (float height)
{
    char bob[128];
    sprintf(bob, "height = %.2f\n", height);
    OutputDebugString(bob);

    // The curve's equation is of the form  y = a - ((x - b) / c)^2  (an upside down parabola).
    // a gives the curve's maximum height (apex), b gives the curves centre on the x-axis and
    // c can be shown to be equal to b / sqrt(a). Imposing the arbitrary constraint that
    // a*b = 500 the curve's area becomes fixed, giving the illusion of different trajectories.
    float x, y, z;
    Ogre::Vector3 localPosition, worldPosition;
    
    Ogre::Vector3 playerPos = mPlayerNode->getPosition();
    Ogre::Vector3 targetPos = mTargetNode->getPosition();
    Ogre::Vector3 targetPos_pc = mPlayerNode->convertWorldToLocalPosition(targetPos);
    targetPos_pc.y = 0;
    float distance = targetPos_pc.length();

    // Deriving a curve's parameters requires 3 known points. We only have 2 available (the start and end),
    // however we also know the maximum height we want the curve to reach. Using this and making some assumptions
    // about the shape of the curve that we want we can infer a third point, the 'apex' of the curve. This will
    // not actually be the curve's true apex, but the curve will pass through it and the closeness of the model
    // used to approximate the third point to the true curve will affect how close this apex is.
    //
    // Start with three points, p1=(x1, y1), p2=(x2, y2) p3=(x3, y3). p1 and p3 are the start and end co-ordinates
    // in 2D co-ordinates relative to the player node (so p1 is always (0, 0))(they could be in world co-ordinates
    // but it adds unnecessary complexity to the calculations later).
    // The naive approach to getting a third point would be to assume that the apex of the curve would be central
    // along the x-axis (i.e. distance / 2), true when there is no difference in height between the player and the
    // target, however when a significant drop in height occurs between the two it means that the curve will be
    // significantly too high (as the true apex would fall closer to the player along the curve, but the curve 
    // must still pass through the calculated apex midway along the curve, causing the true apex to be much 
    // higher). The solution is to model the curve (with no knowledge of it) and guess where the true apex will
    // occur, thus minimising the difference between calculated and true apex and the over/under shooting that 
    // occurs.
    // To do this we use the simple, arbitrarily selected curve y = -0.2 x^2, where x = -sqrt( abs(5y) ). Using
    // this we can calculate where the apex of the curve falls on the x-axis in relation to the starting height
    // of the curve. Selecting three points from this curve (in this case (0, 0.5), (0.5, 0.415), (0.875, 0.261)
    // using the x-axis to represent the height the player node starts at relative to the TOTAL vertical distance
    // (i.e. not in player co-ordinates) and the y-axis to represent the horizontal position of the apex relative
    // to the total horizontal distance travelled) allow us to build a further curve showing the rate of change
    // of the apex position against the changing starting height. This curve has the parameters (a=-0.275,
    // b=-0.032, c=0.5) and, given x = (drop distance) / (drop distance + apex height to player) produces the
    // horizontal position of the apex along the curve (between 0 and 1). This apex is for the previously noted
    // curve y = -0.2 x^2, however since our desired curve is of polynomial order and the sam rough shape, it is
    // not a wild assumption that the apex of OUR curve will be in a similar location.
    // The output we just generated can be multiplied by the total distance travelled by our curve to give an
    // approximation of the x co-ordinate of the apex. This completes the third point to derive our curves
    // parameters, allowing it to be evaluated and plotted.
    // If greater accuracy is required, the curve generated at this point can be fed back into the algorithm in
    // an iterative manner in place of the function y=-0.2x^2 as a much more accurate method of approximating the
    // apex's location. This would be relatively straight forward though computationally expensive as this 
    // calculation would have to be done online and, since the curve would be more complicated, differentiation
    // would have to be used to calculate the apex rather than inverting the function. The advantage of this is
    // that the output would be far more accurate and would converge rapidly - I would estimate that no more than
    // one such iteration would be required.

    float x1 = 0;                   float y1 = 0;
    float x3 = distance;            float y3 = targetPos.y - playerPos.y;
    float x2 = distance * 0.5f;     float y2 = (height + MAX(y1, y3));

    float heightDropRatio, apexDistanceRatio;
    if (y3 > 0)
        heightDropRatio   = ( y3) / ( y3 + height);
    else
        heightDropRatio   = (-y3) / (-y3 + height);
    apexDistanceRatio = (-0.275 * heightDropRatio * heightDropRatio) + (-0.032 * heightDropRatio) + 0.5;
    if (y3 > 0)
        apexDistanceRatio = 1 - apexDistanceRatio;
    x2 = apexDistanceRatio * distance;

    //char bob[128];
    sprintf(bob, "heightDropRatio=%.2f, apexDistanceRatio=%.2f\n", heightDropRatio, apexDistanceRatio);
    OutputDebugString(bob);

                                    //float y2 = height - (y3 / 2.0f);
    /*Ogre::Matrix3 xMat( x1*x1, x1, 1,
                        x2*x2, x2, 1,
                        x3*x3, x3, 1  );
    Ogre::Vector3 yMat( y1,
                        y2,
                        y3  );
    Ogre::Matrix3 xMatI = xMat.Inverse();
    Ogre::Vector3 rMat = xMatI * yMat;*/
    float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
    float A     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
    float B     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
    float C     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
    
    // Clear the chains in preparation to redraw.
    mPathChain->clearAllChains();
    mPathChain->addChainElement(0, Ogre::BillboardChain::Element(Ogre::Vector3::ZERO, 1, 0, Ogre::ColourValue(1.0f, 1.0f, 1.0f)));
    float eval_step = distance / ((float) ARC_RESOLUTION);
    for (float i = eval_step; i < distance; i += eval_step)
    {
        // Evaluate the curve at the current point.
        x = 0;
        y = A * i * i + B * i + C;
        z = -i;

        // Check the curve to see if it has clipped through the terrain
        localPosition = Ogre::Vector3(x, y, z);

        // Redraw the curve with the new values.
        mPathChain->addChainElement(0, Ogre::BillboardChain::Element(localPosition, 1, 0, Ogre::ColourValue(1.0f, 1.0f, 1.0f)));
    }
}



Ogre::Vector3 GraphicsCore::getTerrainNormalAtWorldPosition (Ogre::Vector3 pos)
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



void GraphicsCore::showAimReticule (OIS::MouseState ms)
{
    
        // Place the target node ting
        // Ray cast from the current mouse point through the camera
        Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(ms.X.abs / ((float) ms.width), ms.Y.abs / ((float) ms.height));
        Ogre::TerrainGroup::RayResult terrainHit = mTerrainGroup->rayIntersects(mouseRay);

        // Check if the user clicked on terrain
        if (terrainHit.hit)
        {
            // Get the terrain normal
            Ogre::Vector3 n = getTerrainNormalAtWorldPosition(terrainHit.position);

            // Place and angle the aim nodule
            mTargetNode->setPosition(terrainHit.position);
            mTargetNode->setDirection(n, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Y);
            //mTargetBillboardSet->setCommonUpVector(Ogre::Vector3(0, 1, 0));
            //mTargetBillboardSet->setCommonUpVector(n);
            //mTargetBillboardSet->getBillboard(0)->setPosition(terrainHit.position + Ogre::Vector3(0, 5, 0));
            mTargetNode->setVisible(true);

            // Get the direction of the target relative to the player, projecting it down into 2D to get the planar
            // distance (XZ plane) of the target (elevation is accounted for when solving the equations to produce 
            // the curve by further projection to the XY plane and subsequent rotation). Failing to project to XZ
            // here will cause erroneous distances to be calculated later on.
            Ogre::Vector3 targetVector_pc = mPlayerNode->convertWorldToLocalPosition(mTargetNode->getPosition());
            targetVector_pc.y = 0;
            //float        targetDistance = targetVector_pc.length();
            Ogre::Radian targetAngle    = targetVector_pc.angleBetween(Ogre::Vector3::NEGATIVE_UNIT_Z);
            if (targetVector_pc.x > 0)
                targetAngle *= -1;

            // Angle the path node appropriately.
            mPathNode->setOrientation(Ogre::Quaternion(targetAngle, Ogre::Vector3::UNIT_Y));

            // Draw the path.
            drawTargetPath(mPlayerAimHeight);

            // HIde the normal cursor.
            CEGUI::MouseCursor::getSingleton().hide();
            
            // Display the aiming cursor.
            mPathNode->setVisible(true);
            mTargetNode->setVisible(true);
        }
}


void GraphicsCore::hideAimReticule (void)
{
        mPathNode->setVisible(false);
        mTargetNode->setVisible(false);
}



//Adjust mouse clipping area
void GraphicsCore::windowResized (Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mUserInput->mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}



//Unattach OIS before window shutdown (very important under Linux)
void GraphicsCore::windowClosed (Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if (mUserInput != NULL)
            delete mUserInput;
    }
}








bool GraphicsCore::frameRenderingQueued (const Ogre::FrameEvent& evt)
{
    if (mWindow->isClosed())
        return false;

    if (mShutDown)
        return false;
    
    //Need to inject timestamps to CEGUI System.
    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    //Need to capture/update each device
    mUserInput->capture();
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_ESCAPE) == true)
    {
        mShutDown = true;
    }

    //mTrayMgr->frameRenderingQueued(evt);

    /*if (!mTrayMgr->isDialogVisible())
    {
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }*/



    Ogre::Vector3 pp = mPlayerNode->getPosition();
    pp.y = mTerrainGroup->getHeightAtWorldPosition(pp.x, pp.y, pp.z) + mPlayerHeight;
    
    float forward = 0;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_W))
        forward -= 200 * evt.timeSinceLastFrame;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_S))
        forward += 200 * evt.timeSinceLastFrame;

    float right = 0;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_Q))
        right += 200 * evt.timeSinceLastFrame;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_E))
        right -= 200 * evt.timeSinceLastFrame;

    float clockwise = 0;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_A))
        clockwise += 100 * evt.timeSinceLastFrame;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_D))
        clockwise -= 100 * evt.timeSinceLastFrame;

    mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(clockwise)));

    Ogre::Radian rot = mPlayerNode->getOrientation().getYaw(true);
    float zComponent = Ogre::Math::Cos(rot);
    float xComponent = Ogre::Math::Sin(rot);
    
    mPlayerNode->setPosition(pp.x + (xComponent * forward) + (xComponent * right), pp.y, pp.z + (zComponent * forward) + (zComponent * right));

    
    if (mTerrainGroup->isDerivedDataUpdateInProgress())
    {
        //mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
        //mInfoLabel->show();

        //if (mTerrainsImported)
            //mInfoLabel->setCaption("Building terrain, please wait...");
        //else
            //mInfoLabel->setCaption("Updating textures, patience...");
    }
    else
    {
        //mTrayMgr->removeWidgetFromTray(mInfoLabel);
        //mInfoLabel->hide();
        if (mTerrainsImported)
        {
            mTerrainGroup->saveAllTerrains(true);
            mTerrainsImported = false;
        }
    }


    return true;
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
        GraphicsCore app;
 
        try
        {
            app.go();
        }
        catch (Ogre::Exception& e)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            OutputDebugString("OHNOES\n");
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