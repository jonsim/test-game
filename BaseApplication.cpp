//---------- INCLUDES ----------//

#include "BaseApplication.h"



//---------- FUNCTIONS ----------//

/// @brief  Constructor
BaseApplication::BaseApplication (void) : mRoot(0),
                                          mCamera(0),
                                          mSceneMgr(0),
                                          mWindow(0),
                                          mResourcesCfg(Ogre::StringUtil::BLANK),
                                          mPluginsCfg(Ogre::StringUtil::BLANK),
                                          mTrayMgr(0),
                                          mDetailsPanel(0),
                                          mTargetCursorVisible(false),
                                          mCursorWasVisible(false),
                                          mShutDown(false),
                                          mInputManager(0),
                                          mMouse(0),
                                          mKeyboard(0)
{
}



/// @brief  Deconstructor
BaseApplication::~BaseApplication (void)
{
    if (mTrayMgr) delete mTrayMgr;

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}



void BaseApplication::go (void)
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



bool BaseApplication::setup (void)
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



void BaseApplication::setupResources (void)
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



bool BaseApplication::configure (void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
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



void BaseApplication::chooseSceneManager (void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}



void BaseApplication::createCamera (void)
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



void BaseApplication::createViewports (void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    // Set the background colour and match the aspect ratio to the window's.
    vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}



void BaseApplication::createResourceListener (void)
{

}



void BaseApplication::loadResources (void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}



void BaseApplication::createFrameListener (void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();

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

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

    mRoot->addFrameListener(this);
}



void BaseApplication::destroyScene (void)
{
}



bool BaseApplication::keyPressed (const OIS::KeyEvent &arg)
{
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectKeyDown(arg.key);
    sys.injectChar(arg.text);

    if (mTrayMgr->isDialogVisible())
        return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    return true;
}



bool BaseApplication::keyReleased (const OIS::KeyEvent &arg)
{
    CEGUI::System::getSingleton().injectKeyUp(arg.key);

    return true;
}



static unsigned int lineCount = 0;
Ogre::ManualObject* BaseApplication::drawLine (Ogre::SceneNode* sn, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& col)
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



void BaseApplication::setupTargetPath (void)
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



void BaseApplication::drawTargetPath (float distance, float height)
{
    // The curve's equation is of the form  y = a - ((x - b) / c)^2  (an upside down parabola).
    // a gives the curve's maximum height (apex), b gives the curves centre on the x-axis and
    // c can be shown to be equal to b / sqrt(a). Imposing the arbitrary constraint that
    // a*b = 500 the curve's area becomes fixed, giving the illusion of different trajectories.
    float x, y, z;
    float eval_step = distance / ((float) ARC_RESOLUTION);
    Ogre::Vector3 localPosition, worldPosition;

    // Debug output
    char bob[128];
    sprintf(bob, "distance = %.2f\n", distance);
    OutputDebugString(bob);
    

    
    Ogre::Vector3 playerPos = mPlayerNode->getPosition();
    Ogre::Vector3 targetPos = mTargetNode->getPosition();
    float x1 = 0;                   float y1 = 0;
    float x2 = distance / 2.0f;     float y2 = height;
    float x3 = distance;            float y3 = targetPos.y - playerPos.y;
    //float x1 = mPlayerNode->getPosition().x;    float y1 = mPlayerNode->getPosition().y;
    //float x3 = x1 + distance;                   float y3 = mTargetNode->getPosition().y;
    //float x2 = (x3 - x1) / 2.0f;                float y2 = y1 + height;
    Ogre::Matrix3 xMat( x1*x1, x1, 1,
                        x2*x2, x2, 1,
                        x3*x3, x3, 1  );
    Ogre::Vector3 yMat( y1,
                        y2,
                        y3  );
    Ogre::Matrix3 xMatI = xMat.Inverse();
    Ogre::Vector3 rMat = xMatI * yMat;
    sprintf(bob, "a=%.3f, b=%.3f, c=%.3f\n", rMat.x, rMat.y, rMat.z);
    OutputDebugString(bob);
    float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
    float A     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
    float B     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
    float C     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
    sprintf(bob, "A=%.3f, B=%.3f, C=%.3f\n", A, B, C);
    OutputDebugString(bob);
    
    // Clear the chains in preparation to redraw.
    mPathChain->clearAllChains();
    mPathChain->addChainElement(0, Ogre::BillboardChain::Element(Ogre::Vector3::ZERO, 1, 0, Ogre::ColourValue(1.0f, 1.0f, 1.0f)));
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


void BaseApplication::setupGUI (void)
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



bool BaseApplication::mouseMoved (const OIS::MouseEvent &arg)
{
    if (mTrayMgr->injectMouseMove(arg))
        return true;

    // Inject CEGUI
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
        sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);



    /*if (arg.state.buttonDown(OIS::MB_Left))
    {
        mPathNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(-0.3 * arg.state.X.rel)));

        int cursorOffset = arg.state.Y.abs - mCursorStartY;
        //if (cursorOffset > 0)
        //{
            if (cursorOffset < 10)
                cursorOffset = 10;
            else if (cursorOffset > 200)
                cursorOffset = 200;
        //}
        //else
        //{
            //if (cursorOffset > -10)
            //    cursorOffset = -10;
            //else if (cursorOffset < 200)
            //    cursorOffset = -200;
        //}
        redrawArc(20.0f * cursorOffset);
    }
    else*/ if (arg.state.buttonDown(OIS::MB_Right))
    {
        mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(-0.3 * arg.state.X.rel)));
    }

    return true;
}



bool BaseApplication::mousePressed (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject CEGUI
    CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));

    if (mTrayMgr->injectMouseDown(arg, id))
        return true;

    if (id == OIS::MB_Left)
    {
        /*mPathNode->setVisible(true);
        mCursorStartY = arg.state.Y.abs;*/
        mTargetCursorVisible = true;
    }

    return true;
}



bool BaseApplication::mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject CEGUI
    CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));

    if (mTrayMgr->injectMouseUp(arg, id))
        return true;
    
    if (id == OIS::MB_Left)
    {
        //mPathNode->setVisible(false);
        mTargetCursorVisible = false;
    }

    return true;
}



CEGUI::MouseButton BaseApplication::convertButton (OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
        case OIS::MB_Left:    return CEGUI::LeftButton;
        case OIS::MB_Right:   return CEGUI::RightButton;
        case OIS::MB_Middle:  return CEGUI::MiddleButton;
        default:              return CEGUI::LeftButton;
    }
}



//Adjust mouse clipping area
void BaseApplication::windowResized (Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}



//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed (Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}






bool BaseApplication::frameRenderingQueued (const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;
    
    //Need to inject timestamps to CEGUI System.
    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
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
    }

    return true;
}