#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include "BaseApplication.h"
 
class TutorialApplication : public BaseApplication
{
private:
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    Ogre::TerrainGroup* mTerrainGroup;
    OgreBites::Label* mInfoLabel;
    bool mTerrainsImported;
 
    void defineTerrain(long x, long y);                     //
    void initBlendMaps(Ogre::Terrain* terrain);             //
    void configureTerrainDefaults(Ogre::Light* light);      //
public:
    TutorialApplication(void);                              //
    virtual ~TutorialApplication(void);                     //
 
protected:
    virtual void createScene(void);                         //
    virtual void createFrameListener(void);                 //
    virtual void destroyScene(void);                        //
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
};