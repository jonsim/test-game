#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H


//---------- INCLUDES ----------//

#include "SceneSetup.h"
#include "Input.h"


//---------- DEFINITIONS ----------//

class GraphicsCore : public SceneSetup, public Ogre::FrameListener, public Ogre::WindowEventListener, OgreBites::SdkTrayListener
{
private:
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    OgreBites::Label* mInfoLabel;
    bool mTerrainsImported;
    float mPlayerHeight;
 
    void defineTerrain (long x, long y);
    void initBlendMaps (Ogre::Terrain* terrain);
    void configureTerrainDefaults (Ogre::Light* light);


public:
    GraphicsCore (void);
    virtual ~GraphicsCore (void);

    virtual void go(void);
    void showAimReticule (OIS::MouseState ms);
    void hideAimReticule (void);


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
    void setupTargetPath (void);
    void drawTargetPath (float height);
    Ogre::ManualObject* drawLine (Ogre::SceneNode* sn, const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& col);
    Ogre::Vector3 getTerrainNormalAtWorldPosition (Ogre::Vector3 pos);



    

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
    float mPlayerAimHeight;
    int  mCursorStartY;
    bool mShutDown;

    // Input
    Input* mUserInput;
};

#endif // #ifndef GRAPHICSCORE_H