#ifndef BASEAPPLICATION_H
#define BASEAPPLICATION_H


//---------- INCLUDES ----------//

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <CEGUI.h>
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>



//---------- DEFINITIONS ----------//
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    BaseApplication(void);
    virtual ~BaseApplication(void);

    virtual void go(void);

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0; // Override me!
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);
    virtual void setupGUI (void);

    void setupTargetPath (void);
    void drawTargetPath (float distance, float height);
    Ogre::ManualObject* drawLine (Ogre::SceneNode* sn, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& col);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    // CEGUI
	CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);

    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    
    Ogre::BillboardSet* mTargetBillboardSet;
    Ogre::TerrainGroup* mTerrainGroup;
    #define ARC_RESOLUTION 100
    Ogre::SceneNode* mPlayerNode;
        Ogre::SceneNode* mCameraNode;
        Ogre::SceneNode* mCharacterNode;
        Ogre::SceneNode* mPathNode;
    Ogre::SceneNode* mTargetNode;
    Ogre::BillboardChain* mPathChain;

    // OgreBites
    CEGUI::OgreRenderer* mGUIRenderer;
	CEGUI::Window*       mGUIWindow;
    OgreBites::SdkTrayManager* mTrayMgr;
    OgreBites::ParamsPanel*    mDetailsPanel;  // sample details panel
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    bool mTargetCursorVisible;
    int  mCursorStartY;
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*        mMouse;
    OIS::Keyboard*     mKeyboard;
};

#endif // #ifndef BASEAPPLICATION_H
