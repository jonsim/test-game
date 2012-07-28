#ifndef SCENESETUP_H
#define SCENESETUP_H


//---------- INCLUDES ----------//

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <CEGUI.h>



//---------- DEFINITIONS ----------//
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

class SceneSetup
{
public:
    SceneSetup (void);
    virtual ~SceneSetup (void);

protected:
//    virtual void createFrameListener(void);
//    virtual void createScene(void) = 0; // Override me!
//    virtual void destroyScene(void);


    // Ogre::FrameListener
//    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);


};

#endif // #ifndef SCENESETUP_H
