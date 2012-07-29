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

    mUserInput = new Input(pl);



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
    Core::mWorld = new World(mSceneMgr, light);

    // Setup the player yo.
    Core::mPlayer = new Player(mSceneMgr);
    Core::mPlayer->attachCamera(mCamera);
}



void GraphicsCore::destroyScene (void)
{
    delete Core::mWorld;
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
    // Capture input.
    mUserInput->capture();
    
    // Check for exit conditions.
    if (mWindow->isClosed())
        return false;
    if (mShutDown)
        return false;
    if (mUserInput->mKeyboard->isKeyDown(OIS::KC_ESCAPE) == true)
        mShutDown = true;
    
    // Inject CEGUI timestamps.
    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    // Update various parts of the graphics.
    Core::mPlayer->frameUpdate(evt.timeSinceLastFrame);

    // Update the terrain, saving it if it has been updated and not already been.
    Core::mWorld->saveTerrainState();

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
        // Create application object, and save a reference to it.
        GraphicsCore app;
        Core::setup(&app);
        
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