//---------- INCLUDES ----------//

#include "SharedIncludes.h"



//---------- FUNCTIONS ----------//

/// @brief  Constructor
Input::Input (OIS::ParamList paramList, GraphicsCore* gc) : mGraphicsCore(gc)
{
    mInputManager = OIS::InputManager::createInputSystem(paramList);

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mMouse    = static_cast<OIS::Mouse*>(   mInputManager->createInputObject(OIS::OISMouse,    true));
    
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
}



/// @brief  Deconstructor
Input::~Input (void)
{
    mInputManager->destroyInputObject(mMouse);
    mInputManager->destroyInputObject(mKeyboard);

    OIS::InputManager::destroyInputSystem(mInputManager);
    mInputManager = 0;
}



void Input::capture (void)
{
    mKeyboard->capture();
    mMouse->capture();
}



bool Input::keyPressed (const OIS::KeyEvent &arg)
{
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectKeyDown(arg.key);
    sys.injectChar(arg.text);

    /*if (mTrayMgr->isDialogVisible())
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
    else*/ 

    return true;
}



bool Input::keyReleased (const OIS::KeyEvent &arg)
{
    CEGUI::System::getSingleton().injectKeyUp(arg.key);

    return true;
}



bool Input::mouseMoved (const OIS::MouseEvent &arg)
{
//    if (mTrayMgr->injectMouseMove(arg))
//        return true;

    // Inject CEGUI
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
        sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);

    
    if (arg.state.buttonDown(OIS::MB_Left))
    {

        //mPlayerAimHeight += arg.state.Y.rel;
        //drawTargetPath(mPlayerAimHeight);
    }
    else if (arg.state.buttonDown(OIS::MB_Right))
    {
        //mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(-0.3 * arg.state.X.rel)));
    }

    return true;
}



bool Input::mousePressed (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject CEGUI
    CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));

//    if (mTrayMgr->injectMouseDown(arg, id))
//        return true;

    if (id == OIS::MB_Left)
    {
        OIS::MouseState ms = mMouse->getMouseState();
        mGraphicsCore->showAimReticule(ms);
    }

    return true;
}



bool Input::mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject CEGUI
    CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));

//    if (mTrayMgr->injectMouseUp(arg, id))
//        return true;
    
    if (id == OIS::MB_Left)
    {
        // Hide the pathing nodule.
        mGraphicsCore->hideAimReticule();

        // Show the normal cursor again.
        CEGUI::MouseCursor::getSingleton().show();
    }

    return true;
}



CEGUI::MouseButton Input::convertButton (OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
        case OIS::MB_Left:    return CEGUI::LeftButton;
        case OIS::MB_Right:   return CEGUI::RightButton;
        case OIS::MB_Middle:  return CEGUI::MiddleButton;
        default:              return CEGUI::LeftButton;
    }
}