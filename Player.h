#ifndef PLAYER_H
#define PLAYER_H


//---------- INCLUDES ----------//
#include "SharedIncludes.h"
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <CEGUI.h>



//---------- DEFINITIONS ----------//
class Player
{
public:
    Player (Ogre::SceneManager* sceneManager);
    ~Player (void);
    
    void frameUpdate (const float tslf);

    void showTargettingSystem (OIS::MouseState ms);
    void hideTargettingSystem (void);

    void attachCamera (Ogre::Camera* camera);

private:
    void setupNodes (Ogre::SceneManager* sceneManager);
    void setupTargettingSystem (Ogre::SceneManager* sceneManager);

    void drawTargettingPath (const Ogre::Vector3& endPosition, const float height);
    void placeTargettingEnd (const Ogre::Vector3& endPosition);
    

    // Nodes
    Ogre::SceneNode* mPlayerNode;
    Ogre::SceneNode* mCharacterNode;

    Ogre::SceneNode* mPlayerCameraNode;

    Ogre::SceneNode* mTargettingPathNode;
    Ogre::SceneNode* mTargettingEndNode;

    Ogre::BillboardChain* mTargettingPathChain;
    Ogre::BillboardSet* mTargettingEndBBS;
    #define ARC_RESOLUTION 100
    
    float mPlayerAimHeight;
    float mPlayerHeight;
};

#endif // #ifndef PLAYER_H
