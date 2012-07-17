#ifndef TUTORIALAPPLICATION_H
#define TUTORIALAPPLICATION_H


//---------- INCLUDES ----------//

#include "BaseApplication.h"



//---------- DEFINITIONS ----------//

class TutorialApplication : public BaseApplication
{
private:
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    OgreBites::Label* mInfoLabel;
    bool mTerrainsImported;
    float mPlayerHeight;
 
    void defineTerrain (long x, long y);
    void initBlendMaps (Ogre::Terrain* terrain);
    void configureTerrainDefaults (Ogre::Light* light);

    Ogre::Vector3 getTerrainNormalAtWorldPosition (Ogre::Vector3 pos);

public:
    TutorialApplication (void);
    virtual ~TutorialApplication (void);
 
protected:
    virtual void createScene (void);
    virtual void createFrameListener (void);
    virtual void destroyScene (void);
    virtual bool frameRenderingQueued (const Ogre::FrameEvent& evt);
};

#endif // #ifndef TUTORIALAPPLICATION_H