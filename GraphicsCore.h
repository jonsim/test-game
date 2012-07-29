#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H


//---------- INCLUDES ----------//

#include "SceneSetup.h"
#include "Input.h"
#include "Player.h"
#include "World.h"


//---------- DEFINITIONS ----------//

class GraphicsCore : public SceneSetup, public Ogre::FrameListener, public Ogre::WindowEventListener, OgreBites::SdkTrayListener
{
public:
    GraphicsCore (void);
    virtual ~GraphicsCore (void);

    virtual void go(void);

    // Input
    Input* mUserInput;
    Ogre::Camera* mCamera;


protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);
    virtual void setupGUI (void);


    // stuff
    Ogre::ManualObject* drawLine (Ogre::SceneNode* sn, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& col);



    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);



    virtual void createScene (void);
    virtual void createFrameListener (void);
    virtual void destroyScene (void);
    virtual bool frameRenderingQueued (const Ogre::FrameEvent& evt);


    // jazz
    Ogre::Root *mRoot;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    

    // OgreBites
    CEGUI::OgreRenderer* mGUIRenderer;
	CEGUI::Window*       mGUIWindow;
    OgreBites::SdkTrayManager* mTrayMgr;
    OgreBites::ParamsPanel*    mDetailsPanel;  // sample details panel
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    int  mCursorStartY;
    bool mShutDown;
};

#endif // #ifndef GRAPHICSCORE_H